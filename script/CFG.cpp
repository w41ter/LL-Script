#include "CFG.h"

#include <map>
#include <queue>
#include <list>
#include <functional>
#include <algorithm>
#include <cassert>
#include <sstream>

#include "Value.h"
#include "Instruction.h"
#include "IRContext.h"

namespace script
{
    BasicBlock::~BasicBlock()
    {
		while (!instrs_.empty())
		{
			instrs_.front()->erase_from_parent();
		}
    }

    BasicBlock * CFG::createBasicBlock(const std::string &name)
    {
        numBlockIDs_++;
        BasicBlock *temp = new BasicBlock(name);
        blocks_.push_back(temp);
        return temp;
    }

    void CFG::setEntry(BasicBlock * entry)
    {
        assert(entry != nullptr);
        start_ = entry;
    }

    void CFG::setEnd(BasicBlock * end)
    {
        assert(end != nullptr);
        end_ = end;
    }

    BasicBlock * CFG::getEntryBlock()
    {
        return start_;
    }

    void CFG::sealOthersBlock()
    {
        for (auto *block : blocks_)
            sealBlock(block);
    }

    void CFG::sealBlock(BasicBlock * block)
    {   
        assert(block != nullptr);
        if (sealedBlock_.count(block) != 0)
            return;
        
        auto &block2Phi = incompletePhis_[block];
        for (auto &b2p : block2Phi)
        {
            addPhiOperands(b2p.first, (Phi*)b2p.second);
        }
        sealedBlock_.insert(block);
    }

    void CFG::saveVariableDef(std::string name, 
        BasicBlock * block, Value * value)
    {
        assert(block != nullptr && value != nullptr);
        currentDef_[name][block] = value;
    }

    Value * CFG::readVariableDef(std::string name, BasicBlock * block)
    {
        assert(block != nullptr);
        auto &def = currentDef_[name];
        if (def.find(block) != def.end())
            return def[block];
        return readVariableRecurisive(name, block);
    }

    std::string CFG::phiName(const std::string & name)
    {
        std::stringstream stream;
        stream << name << '.' << phiCounts_[name]++;
        std::string tmp;
        stream >> tmp;
        return tmp;
    }

    Value * CFG::readVariableRecurisive(std::string name, BasicBlock * block)
    {
        assert(block != nullptr);
        Value *val = nullptr;
        if (sealedBlock_.find(block) == sealedBlock_.end())
        {
            // incomplete CFGs.
            val = IRContext::createAtBegin<Phi>(block, phiName(name));
            incompletePhis_[block][name] = val;
        }
        else if (block->numOfPrecursors() == 1)
        {
            // Optimize the common case of one predecessor, no Phi needed.
            val = readVariableDef(name, block->precursor(0));
        }
        else
        {
            // Break potential cycles with operandless Phi
            val = IRContext::createAtBegin<Phi>(block, phiName(name));
            saveVariableDef(name, block, val);
            val = addPhiOperands(name, (Phi*)val);
        }
        saveVariableDef(name, block, val);
        return val;
    }

    Value * CFG::addPhiOperands(std::string name, Phi * phi)
    {
        // Determine operands from predecessors
        BasicBlock *phiParent = phi->get_parent();
        for (auto i = phiParent->precursor_begin(),
            e = phiParent->precursor_end();
            i != e; ++i)
        {
            phi->appendOperand(readVariableDef(name, *i));
        }
        return tryRemoveTrivialPhi(phi);
    }

    Value * CFG::tryRemoveTrivialPhi(Phi * phi)
    {
        Value *same = nullptr;
        for (auto beg = phi->op_begin(); beg != phi->op_end(); beg++)
        {
            Value *op = beg->get_value();
            if (op == same || op == phi)
                // Unique value or self−reference
                continue;
            if (same != nullptr)
                // The phi merges at least two values : not trivial
                return phi;
            same = op;
        }
        if (same == nullptr)
            same = IRContext::create<Undef>();
        // try all users except the phi itself.
        // Try to recursively remove all phi users, 
        // which might have become trivial
        for (auto iter = phi->use_begin(); iter != phi->use_end(); ++iter)
        {   
            Instruction *instr = static_cast<Instruction*>(
                (*iter)->get_user());
            if (instr != phi && instr->is_phi_node())
                tryRemoveTrivialPhi((Phi*)instr);
        }
        // Reroute all uses of phi to same and remove phi
		phi->replace_all_uses_with(same);
		phi->erase_from_parent();
        return same;
    }

	void CFG::computeBlockOrder()
	{
		std::list<BasicBlock*> finalBlockOrder;
		std::priority_queue<BasicBlock*, 
			std::vector<BasicBlock*>, BlockOrderCmp> queue;

		loopDetection();

		queue.push(this->getEntryBlock());
		while (!queue.empty()) {
			BasicBlock *block = queue.top(); 
			queue.pop();

			finalBlockOrder.push_back(block);
			for (auto *succ : block->successors_) {
				succ->incomingForwardBranches_ -= 1;
				if (succ->incomingForwardBranches_ == 0) {
					queue.push(succ);
				}
			}
		}
		blocks_.swap(finalBlockOrder);
		numberOperations();
	}

	void CFG::loopDetection()
	{
		B2B loopEndToHead;
		std::set<BasicBlock*> visited;
		BasicBlock *entry = this->getEntryBlock();
		this->numLoopIndex_ = 0;
		this->tryToDetect(loopEndToHead, entry);
		for (auto pair : loopEndToHead) {
			visited.clear();
			tryToAssignIndex(
				pair.second->loopIndex_,
				pair.first, pair.second, visited);
		}
	}

	void CFG::numberOperations()
	{
		unsigned nextID = 0;
		for (auto *block : this->blocks_) {
			block->start_ = nextID;
			for (auto *instr : block->instrs_) {
				instr->setID(nextID);
				nextID += 2;
			}
			block->end_ = nextID;
		}
	}

	void CFG::tryToDetect(B2B & set, BasicBlock * block)
	{
		assert(block);

		if (block->state_ & BasicBlock::Visited)
			return;

		// init
		block->loopDepth_ = 0;
		block->loopIndex_ = -1;
		block->incomingForwardBranches_ = block->precursors_.size();

		block->state_ |= BasicBlock::Visited;
		block->state_ |= BasicBlock::Active;
		for (auto *succ : block->successors_) {
			if (succ->state_ & BasicBlock::Active) {
				set.insert({ block, succ });
				succ->loopIndex_ = this->numLoopIndex_++;
				succ->loopDepth_ = 1;
				succ->incomingForwardBranches_ -= 1;
				continue;
			}
			tryToDetect(set, succ);
		}
		block->state_ &= ~BasicBlock::Active;
	}

	void CFG::tryToAssignIndex(
		int index, 
		BasicBlock * current, 
		BasicBlock * target,
		std::set<BasicBlock*> &visited)
	{
		if (visited.count(current) || current == target)
			return;
		
		++current->loopDepth_;
		if (current->loopIndex_ < 0)
			current->loopIndex_ = index;
		else
			current->loopIndex_ = std::min(current->loopIndex_, index);
	}

    void CFG::erase(BasicBlock *block) 
    {
        for (auto P = block->phi_begin(); P != block->phi_end(); ++P) 
        {
            Phi *phi = *P;
            phi->drop_all_references();
        }
        
        blocks_.remove(block);
        delete block;
    }

    CFG::CFG()
        : start_(nullptr), end_(nullptr)
    {
    }

    CFG::~CFG()
    {
        for (auto &i : blocks_)
        {
            delete i;
            i = nullptr;
        }
    }

    void BasicBlock::addPrecursor(BasicBlock * block)
    {
        precursors_.push_back(block);
    }

    void BasicBlock::addSuccessor(BasicBlock * block)
    {
        successors_.push_back(block);
    }

    BasicBlock * BasicBlock::precursor(int idx)
    {
        if (idx < numOfPrecursors())
            return precursors_[idx];
        return nullptr;
    }

    BasicBlock * BasicBlock::successor(int idx)
    {
        if (idx < numOfSuccessors())
            return successors_[idx];
        return nullptr;
    }

	void BasicBlock::insert(instr_iterator iter, Instruction * instr)
	{
		assert(instr != nullptr);
		instrs_.insert(iter, instr);
        tryInsertPhiNode(instr);
	}

	void BasicBlock::push_back(Instruction * instr)
    {
        assert(instr != nullptr);
        instrs_.push_back(instr);
        tryInsertPhiNode(instr);
    }

    void BasicBlock::push_front(Instruction * instr)
    {
        assert(instr != nullptr);
        instrs_.push_front(instr);
        tryInsertPhiNode(instr);
    }

    void BasicBlock::pop_back()
    {
        tryRemovePhiNode(instrs_.back());
        instrs_.pop_back();
    }

    void BasicBlock::pop_front()
    {
        tryRemovePhiNode(instrs_.front());
        instrs_.pop_front();
    }

    bool BasicBlock::contains(Instruction *instr)
    {
        for (auto *I : instrs_)
            if (I == instr)
                return true;
        return false;
    }

    void BasicBlock::erase(Instruction * instr)
    {
        tryRemovePhiNode(instr);
        instrs_.remove(instr);
        delete instr;
    }

    void BasicBlock::remove(Instruction * instr)
    {
        tryRemovePhiNode(instr);
        instrs_.remove(instr);
    }

    void BasicBlock::replaceInstrWith(Instruction *from, Instruction *to)
    {
        for (auto *& elem : instrs_)
        {
            if (elem == from) {
                elem = to;
                tryRemovePhiNode(elem);
                tryInsertPhiNode(to);
            }
        }
    }

    void BasicBlock::tryRemovePhiNode(Instruction * instr)
    {
        if (instr->is_phi_node()) {
            removePhiNodeRecord(static_cast<Phi*>(instr));
        }
    }

    void BasicBlock::tryInsertPhiNode(Instruction * instr)
    {
        if (instr->is_phi_node()) {
            recordPhiNode(static_cast<Phi*>(instr));
        }
    }

    void BasicBlock::removePhiNodeRecord(Phi * phi)
	{
		phiNodes_.remove(phi);
	}
	
	void BasicBlock::recordPhiNode(Phi * phi)
	{
		for (auto *p : phiNodes_)
			if (p == phi)
				return;
		phiNodes_.push_back(phi);
	}
}


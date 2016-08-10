#include "CFG.h"

#include "Instruction.h"
#include "IRContext.h"

#include <map>
#include <list>
#include <functional>
#include <algorithm>
#include <cassert>

using namespace script::ir;
namespace script
{
    BasicBlock * CFG::createBasicBlock(std::string name)
    {
        BasicBlock *temp = new BasicBlock(numBlockIDs_++, name);
        blocks_.push_back(temp);
        return temp;
    }

    void CFG::setEntry(BasicBlock * entry)
    {
        start_ = entry;
    }

    void CFG::setEnd(BasicBlock * end)
    {
        end_ = end;
    }

    BasicBlock * CFG::getEntryBlock()
    {
        return start_;
    }

    std::list<BasicBlock*>& CFG::blocks()
    {
        return blocks_;
    }

    IRContext * CFG::getContext()
    {
        return context_;
    }

    void CFG::sealOthersBlock()
    {
        for (auto *block : blocks_)
            sealBlock(block);
    }

    void CFG::sealBlock(BasicBlock * block)
    {   
        //if (sealedBlock_.count(block) != 0)
        //    return;
        //auto &block2Phi = incompletePhis_[block];
        //for (auto &b2p : block2Phi)
        //{
        //    addPhiOperands(b2p.first, (Phi*)b2p.second);
        //}
        //sealedBlock_.insert(block);
    }

    void CFG::saveVariableDef(std::string name, BasicBlock * block, ir::Value * value)
    {
        currentDef_[name][block] = value;
    }

    ir::Value * CFG::readVariableDef(std::string name, BasicBlock * block)
    {
        auto &def = currentDef_[name];
        if (def.find(block) != def.end())
            return def[block];
        return readVariableRecurisive(name, block);
    }

    ir::Value * CFG::readVariableRecurisive(std::string name, BasicBlock * block)
    {
        Value *val = nullptr;
        if (sealedBlock_.find(block) != sealedBlock_.end())
        {
            // incomplete CFGs.
            assert(block->begin() != nullptr);
            val = context_->create<Phi>(name, block->begin());
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
            //assert(block->begin() != nullptr);
            val = block->begin() == nullptr
                ? context_->create<Phi>(name, block)
                : context_->create<Phi>(name, block->begin());
            saveVariableDef(name, block, val);
            val = addPhiOperands(name, (Phi*)val);
        }
        saveVariableDef(name, block, val);
        return val;
    }

    ir::Value * CFG::addPhiOperands(std::string name, ir::Phi * phi)
    {
        // Determine operands from predecessors
        for (auto *pred : phi->getParent()->precursors())
        {
            phi->appendOperand(readVariableDef(name, pred));
        }
        return tryRemoveTrivialPhi(phi);
    }

    ir::Value * CFG::tryRemoveTrivialPhi(ir::Phi * phi)
    {
        Value *same = nullptr;
        for (auto beg = phi->op_begin(); beg != phi->op_end(); beg++)
        {
            Value *op = beg->getValue();
            if (op == same || op == phi)
                // Unique value or self−reference
                continue;
            if (same != nullptr)
                // The phi merges at least two values : not trivial
                return phi;
            same = op;
        }
        if (same == nullptr)
            same = context_->create<Undef>();
        // try all users except the phi itself.
        // Try to recursively remove all phi users, 
        // which might have become trivial
        for (auto iter = phi->use_begin(); iter != phi->use_end(); ++iter)
        {
            if (iter->getUser() != phi)
            {
                Value *user = iter->getUser();
                if (user->instance() == Instructions::IR_Phi)
                    tryRemoveTrivialPhi((Phi*)user);
            }
        }
        // Reroute all uses of phi to same and remove phi
        phi->replaceBy(same);
        return same;
    }

    CFG::CFG()
        : context_(new IRContext()), numBlockIDs_(0), start_(nullptr), end_(nullptr)
    {
    }

    CFG::~CFG()
    {
        if (context_ != nullptr)
            delete context_;

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

    void BasicBlock::push(Instruction * instr)
    {
        if (head_ == nullptr)
        {
            end_ = head_ = instr;
        }
        else
        {
            end_->next_ = instr;
            instr->prev_ = end_;
            instr->next_ = nullptr;
            end_ = instr;
        }
    }

    void BasicBlock::unique()
    {
        //precursors_.sort();
        //auto end_unique = std::unique(precursors_.begin(), precursors_.end());
        //precursors_.erase(end_unique, precursors_.end());
        //
        //successors_.sort();
        //end_unique = std::unique(successors_.begin(), successors_.end());
        //precursors_.erase(end_unique, successors_.end());
    }
}


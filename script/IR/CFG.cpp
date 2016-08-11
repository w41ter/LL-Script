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
    BasicBlock * CFG::createBasicBlock(const std::string &name)
    {
        BasicBlock *temp = new BasicBlock(numBlockIDs_++, name);
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

    void CFG::saveVariableDef(std::string name, BasicBlock * block, Value * value)
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

    Value * CFG::readVariableRecurisive(std::string name, BasicBlock * block)
    {
        assert(block != nullptr);
        Value *val = nullptr;
        if (sealedBlock_.find(block) != sealedBlock_.end())
        {
            // incomplete CFGs.
            val = context_->createAtBegin<Phi>(block, name);
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
            val = context_->createAtBegin<Phi>(block, name);
            saveVariableDef(name, block, val);
            val = addPhiOperands(name, (Phi*)val);
        }
        saveVariableDef(name, block, val);
        return val;
    }

    ir::Value * CFG::addPhiOperands(std::string name, ir::Phi * phi)
    {
        // Determine operands from predecessors
        BasicBlock *phiParent = phi->getParent();
        for (auto i = phiParent->precursor_begin(),
            e = phiParent->precursor_end();
            i != e; ++i)
        {
            phi->appendOperand(readVariableDef(name, *i));
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
        : context_(new IRContext()), numBlockIDs_(0)
        , start_(nullptr), end_(nullptr)
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

    void BasicBlock::push_back(Instruction * instr)
    {
        assert(instr != nullptr);
        instrs_.push_back(instr);
    }

    void BasicBlock::push_front(Instruction * instr)
    {
        assert(instr != nullptr);
        instrs_.push_front(instr);
    }

    void BasicBlock::pop_back()
    {
        instrs_.pop_back();
    }

    void BasicBlock::pop_front()
    {
        instrs_.pop_front();
    }
}


#include "CFG.h"

#include "Instruction.h"

#include <map>
#include <list>
#include <functional>
#include <algorithm>

using script::ir::Instruction;

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

    CFG::CFG()
        : numBlockIDs_(0), start_(nullptr), end_(nullptr)
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
        precursors_.sort();
        auto end_unique = std::unique(precursors_.begin(), precursors_.end());
        precursors_.erase(end_unique, precursors_.end());
        
        successors_.sort();
        end_unique = std::unique(successors_.begin(), successors_.end());
        precursors_.erase(end_unique, successors_.end());
    }
}


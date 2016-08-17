#pragma once

#include "Instruction.h"
#include "CFG.h"

#include <list>
#include <cassert>

namespace script
{

    class IRContext
    {
        using Value = ir::Value;
        using Branch = ir::Branch;
        using Goto = ir::Goto;
    public:
        ~IRContext();

        template<typename T, typename ...Args>
        Value *create(Args ...args)
        {
            auto *buffer = new T(args...);
            codes_.push_back(buffer);
            return buffer;
        }

        template<typename Type, typename ...Args>
        Value *createAtBegin(BasicBlock *block, Args ...args)
        {
            assert(block != nullptr);
            auto *tmp = new Type(args...);
            codes_.push_back(tmp);
            block->push_front(tmp);
            Instruction *instr = (Instruction*)tmp;
            instr->setParent(block);
            return tmp;
        }

        template<typename Type, typename ...Args>
        Value *createAtEnd(BasicBlock *block, Args ...args)
        {
            assert(block != nullptr);
            auto *tmp = new Type(args...);
            codes_.push_back(tmp);
            block->push_back(tmp);
            Instruction *instr = (Instruction*)tmp;
            instr->setParent(block);
            return tmp;
        }

        Value *createBranchAtEnd(BasicBlock *parent, Value *cond, BasicBlock *then, BasicBlock *_else)
        {
            assert(parent != nullptr);
            auto *tmp = new Branch(parent, cond, then, _else);
            codes_.push_back(tmp);
            parent->push_back(tmp);
            return tmp;
        }

        Value *createBranchAtBegin(BasicBlock *parent, Value *cond, BasicBlock *then, BasicBlock *_else)
        {
            assert(parent != nullptr);
            auto *tmp = new Branch(parent, cond, then, _else);
            codes_.push_back(tmp);
            parent->push_front(tmp);
            return tmp;
        }

        Value *createGotoAtBegin(BasicBlock *parent, BasicBlock *then)
        {
            assert(parent != nullptr);
            auto *tmp = new Goto(parent, then);
            codes_.push_back(tmp);
            parent->push_front(tmp);
            return tmp;
        }

        Value *createGotoAtEnd(BasicBlock *parent, BasicBlock *then)
        {
            assert(parent != nullptr);
            auto *tmp = new Goto(parent, then);
            codes_.push_back(tmp);
            parent->push_back(tmp);
            return tmp;
        }

    protected:
        std::list<Value*> codes_;
    };
}
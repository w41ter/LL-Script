#pragma once

#include "Instruction.h"
#include "CFG.h"

#include <list>
#include <cassert>

namespace script
{
    class IRContext
    {
    public:
        template<typename T, typename ...Args>
        static T *create(Args ...args)
        {
            auto *buffer = new T(args...);
            return buffer;
        }

        template<typename Type, typename ...Args>
		static Type *createAtBegin(BasicBlock *block, Args ...args)
        {
            assert(block != nullptr);
            auto *tmp = new Type(args...);
            assert(!tmp->is_value());
            block->push_front(tmp);
            Instruction *instr = (Instruction*)tmp;
            instr->set_parent(block);
            return tmp;
        }

        template<typename Type, typename ...Args>
		static Type *createAtEnd(BasicBlock *block, Args ...args)
        {
            assert(block != nullptr);
            auto *tmp = new Type(args...);
            assert(!tmp->is_value());
            block->push_back(tmp);
            Instruction *instr = (Instruction*)tmp;
            instr->set_parent(block);
            return tmp;
        }

		static Branch *createBranchAtEnd(
			BasicBlock *parent, Value *cond, 
			BasicBlock *then, BasicBlock *_else)
        {
            assert(parent != nullptr);
            auto *tmp = new Branch(cond, then, _else);
            parent->push_back(tmp);
            parent->addSuccessor(then);
            parent->addSuccessor(_else);
            then->addPrecursor(parent);
            _else->addPrecursor(parent);
			tmp->set_parent(parent);
            return tmp;
        }

		static Goto *createGotoAtEnd(
			BasicBlock *parent, BasicBlock *then)
        {
            assert(parent != nullptr);
            auto *tmp = new Goto(then);
            parent->push_back(tmp);
            parent->addSuccessor(then);
            then->addPrecursor(parent);
			tmp->set_parent(parent);
            return tmp;
        }

		template <typename Type, typename ...Args>
		static Type *insertAfter(
			BasicBlock::instr_iterator iter, Args ...args) {
			auto *tmp = new Type(args...);
			BasicBlock *parent = (*iter)->get_parent();
			parent->insert(iter, tmp);
			tmp->set_parent(parent);
			++iter;
			return tmp;
		}
    };
}
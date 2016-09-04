#pragma once 

#include <vector>
#include <cassert>

#include "Use.h"
#include "Value.h"

#include <functional>

namespace script
{
	class User : public Value
    {
		using map = std::function<void(Value *)>;
    public:
		User(unsigned scid) : Value(scid) { }

        virtual ~User() 
        {
			std::vector<Value*> values;
            for (size_t i = 0; i < get_num_operands(); ++i)
            {
                Value *val = get_operand(i);
                if (val && val->is_value())    // can't be erase by BasicBlock.
					values.push_back(val);
            }
			drop_all_references();

			for (auto *val : values)
				delete val;
        }

        typedef std::vector<Use>::iterator op_iterator;
        op_iterator op_begin() { return operands.begin(); }
        op_iterator op_end() { return operands.end(); }
        unsigned get_num_operands() const { return operands.size(); }
        void op_reserve(size_t size) { operands.reserve(size); }

		void op_map(map map)
		{
			for (auto &oper : operands)
				map(oper.get_value());
		}
        
        Value *get_operand(size_t idx)
        {
            assert(idx < get_num_operands());
            return operands[idx].get_value();
        }

        void drop_all_references()
        {
            for (auto &oper : operands)
                oper.set(0);
			operands.clear();
        }

        void replace_all_uses_of_with(Value *from, Value *to)
        {
            if (from == to)
                return;
            for (auto &oper : operands)
            {
                if (oper.get_value() == from)
                    oper.set(to);
            }
        }
        
    protected:
        std::vector<Use> operands;
    };

}
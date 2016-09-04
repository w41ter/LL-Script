#include "Instruction.h"

#include <cassert>

#include "CFG.h"

namespace script
{
	Instruction::Instruction(unsigned value_id)
		: User(Value::InstructionVal)
		, parent(0)
		, opcode(value_id)
		, output_reg({ 0 })
		, machine_state(false)
    {}

    Instruction::Instruction(unsigned value_id, const char *value_name)
        : Instruction(value_id)
    {
        this->set_value_name(value_name);
    }

    Instruction::Instruction(unsigned value_id, 
        const std::string &value_name)
        : Instruction(value_id, value_name.c_str())
    {
    }

    Instruction::~Instruction()
    {
        assert(parent == 0 && "Instruction still linked in the program!");
    }

    void Instruction::set_parent(BasicBlock *parent)
    {
        if (this->parent)
        {
            this->remove_from_parent();
        }
        this->parent = parent;
		assert(parent->contains(this));
    }

    void Instruction::remove_from_parent()
    {
		assert(parent != 0);
		auto *parent = this->parent;
		this->parent = nullptr;
		parent->remove(this);
    }

    void Instruction::erase_from_parent()
    {
		assert(parent != 0);
		auto *parent = this->parent;
		this->parent = nullptr;
		parent->erase(this);
    }

    void Instruction::replace_with(Instruction *to)
    {
        assert(parent != 0 && to != this);
		replace_all_uses_with(to); 
        parent->replaceInstrWith(this, to);
		erase_from_parent();
    }

    Invoke::Invoke(Value *func, 
        const std::vector<Value*> &args, const char *name)
        : Instruction(Instruction::InvokeVal, name)
    {
		tail_call = false;
        init(func, args);
		assert(operands.size() >= 1);
    }

    Invoke::Invoke(Value *func,
        const std::vector<Value*> &args, const std::string &name)
        : Invoke(func, args, name.c_str())
    {
    }

    Value *Invoke::get_func() 
    {
        return this->get_operand(0);
    }

    void Invoke::init(Value * function, const std::vector<Value*>& args)
    {
        operands.reserve(args.size() + 1);
        operands.push_back(Use(function, this));
        for (Value *value : args)
        {
            operands.push_back(Use(value, this));
        }
    }

    Branch::Branch(Value *cond, BasicBlock *then, BasicBlock *_else)
        : Instruction(Instruction::BranchVal)
    {
        init(cond);
        this->then_ = then;
        this->else_ = _else;
    }

    void Branch::init(Value * cond)
    {
        operands.reserve(1);
        operands.push_back(Use(cond, this));
    }

    Value *Branch::get_cond()
    {
        return get_operand(0);
    }

    Goto::Goto(BasicBlock *block)
        : Instruction(Instruction::GotoVal)
    {
        this->block_ = block;
    }

	Assign::Assign(MachineRegister L, MachineRegister R)
		: Instruction(AssignVal), left(L), right(R)
	{
		machine_state = true;
	}

	Assign::Assign(Value *value, const char *name)
        : Instruction(Instruction::AssignVal, name)
		, left({ 0 })
		, right({ 0 })
    {
        init(value);
    }

    Assign::Assign(Value *value, const std::string &name)
        : Instruction(Instruction::AssignVal, name.c_str())
		, left({ 0 })
		, right({ 0 })
    {
        init(value);
    }

    void Assign::init(Value * value)
    {
        operands.reserve(1);
        operands.push_back(Use(value, this));
    }

    Value *Assign::get_value()
    {
        return get_operand(0);
    }

    NotOp::NotOp(Value *value, const char *name)
        : Instruction(Instruction::NotOpVal, name)
    {
        init(value);
    }

    NotOp::NotOp(Value *value, const std::string &name)
        : Instruction(Instruction::NotOpVal, name.c_str())
    {
        init(value);
    }

    void NotOp::init(Value * value)
    {
        operands.reserve(1);
        operands.push_back(Use(value, this));
    }

    Value *NotOp::get_value()
    {
        return get_operand(0);
    }

    Return::Return(Value *value)
        : Instruction(Instruction::ReturnVal)
    {
        init(value);
		tail_call = false;
		if (value->is_instr()) {
			Instruction *instr = static_cast<Instruction*>(value);
			if (instr->is_invoke()) {
				Invoke *invoke = static_cast<Invoke*>(instr);
				invoke->enable_tail_call();
				tail_call = true;
			}
		}
    }

    void Return::init(Value * value)
    {
        operands.reserve(1);
        operands.push_back(Use(value, this));
    }

    Value *Return::get_value()
    {
        return get_operand(0);
    }

    ReturnVoid::ReturnVoid()
        : Instruction(Instruction::ReturnVoidVal)
    {
    }

    BinaryOperator::BinaryOperator(unsigned ops, Value *lhs, 
        Value *rhs, const char *name)
        : Instruction(Instruction::BinaryOpsVal, name), op_(ops)
    {
        init(lhs, rhs);
    }

    BinaryOperator::BinaryOperator(unsigned ops, Value *lhs, 
        Value *rhs, const std::string &name)
        : Instruction(Instruction::BinaryOpsVal, name.c_str()), op_(ops)
    {
        init(lhs, rhs);
    }
        
    void BinaryOperator::init(Value * lhs, Value * rhs)
    {
        operands.reserve(2);
        operands.push_back(Use(lhs, this));
        operands.push_back(Use(rhs, this));
    }

    Value *BinaryOperator::get_lhs() 
    {
        return get_operand(0);
    }

    Value *BinaryOperator::get_rhs()
    {
        return get_operand(1);
    }

    Index::Index(Value *table, Value *index, const char *name)
       : Instruction(Instruction::IndexVal, name)
    {
        init(table, index);
    } 

    Index::Index(Value *table, Value *index, const std::string &name)
        : Instruction(Instruction::IndexVal, name.c_str())
    {
        init(table, index);
    }    

    void Index::init(Value * table, Value * index)
    {
        operands.reserve(2);
        operands.push_back(Use(table, this));
        operands.push_back(Use(index, this));
    }

    Value *Index::table()
    {
        return get_operand(0);
    }

    Value *Index::index()
    {
        return get_operand(1);
    }

    SetIndex::SetIndex(Value *table, Value *index, Value *to)
        : Instruction(Instruction::SetIndexVal)
    {
        init(table, index, to);
    }

    void SetIndex::init(Value * table, Value * index, Value * to)
    {
        operands.reserve(3);
        operands.push_back(Use(table, this));
        operands.push_back(Use(index, this));
        operands.push_back(Use(to, this));
    }

    Value *SetIndex::table()
    {
        return get_operand(0);
    }

    Value *SetIndex::index()
    {
        return get_operand(1);
    }

    Value *SetIndex::to()
    {
        return get_operand(2);
    }

    Phi::Phi(const char *name)
        : Instruction(Instruction::PhiVal, name)
    {
    }

    Phi::Phi(const std::string &name)
        : Instruction(Instruction::PhiVal, name)
    {}

    Phi::Phi(const char *name, std::initializer_list<Value*> &params)
        : Instruction(Instruction::PhiVal, name)
    {
        init(params);
    }

    Phi::Phi(const std::string &name, std::initializer_list<Value*> &params)
        : Instruction(Instruction::PhiVal, name)
    {
        init(params);
    }

    void Phi::appendOperand(Value * value)
    {
        operands.push_back(Use(value, this));
    }

	void Phi::init(std::initializer_list<Value*> &params)
    {
        op_reserve(params.size());
        for (auto * value : params)
            operands.push_back(Use(value, this));
    }
}
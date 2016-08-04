#include "Instruction.h"
#include "CFG.h"

using std::string;

namespace script
{
namespace ir
{
    Use::Use(Value *value, User *user)
        : value_(value), user_(user)
    {
        if (value)
            value->addUse(*this);
    }

    Use::~Use()
    {
        if (value_)
            value_->killUse(*this);
    }

    Constant::Constant()
        : type_(Null)
    {
    }

    Constant::Constant(int num)
        : type_(Integer), num_(num)
    {}

    Constant::Constant(bool state)
        : type_(Boolean), bool_(state)
    {
    }

    Constant::Constant(char c)
        : type_(Character), c_(c)
    {}

    Constant::Constant(float fnum)
        : type_(Float), fnum_(fnum)
    {}

    Constant::Constant(string str)
        : type_(String), str_(str)
    {}

    void Load::init(Value * from)
    {
        operands_.reserve(1);
        operands_.push_back(Use(from, this));
    }

    void Store::init(Value * value, Value * addr)
    {
        // TODO: check param 2 is addr.
        operands_.reserve(2);
        operands_.push_back(Use(value, this));
        operands_.push_back(Use(addr, this));
    }

    void Invoke::init(const std::string functionName, const std::vector<Value*>& args)
    {
        operands_.reserve(args.size());
        for (Value *value : args)
        {
            operands_.push_back(Use(value, this));
        }
    }

    void Invoke::init(Value * function, const std::vector<Value*>& args)
    {
        operands_.reserve(args.size() + 1);
        operands_.push_back(Use(function, this));
        for (Value *value : args)
        {
            operands_.push_back(Use(value, this));
        }
    }

    void Branch::init(Value * cond)
    {
        operands_.reserve(1);
        operands_.push_back(Use(cond, this));
    }

    void Branch::init(BasicBlock *then, BasicBlock *_else)
    {
        this->parent_->addSuccessor(then);
        then->addPrecursor(this->parent_);

        if (_else == nullptr)
            return;

        this->parent_->addSuccessor(_else);
        _else->addPrecursor(this->parent_);
    }

    void NotOp::init(Value * value)
    {
        operands_.reserve(1);
        operands_.push_back(Use(value, this));
    }

    void Return::init(Value * value)
    {
        operands_.reserve(1);
        operands_.push_back(Use(value, this));
    }

    void BinaryOperator::init(BinaryOps bop, Value * lhs, Value * rhs)
    {
        operands_.reserve(2);
        operands_.push_back(Use(lhs, this));
        operands_.push_back(Use(rhs, this));
    }

    void Index::init(Value * table, Value * index)
    {
        operands_.reserve(2);
        operands_.push_back(Use(table, this));
        operands_.push_back(Use(index, this));
    }

    void SetIndex::init(Value * table, Value * index, Value * to)
    {
        operands_.reserve(3);
        operands_.push_back(Use(table, this));
        operands_.push_back(Use(index, this));
        operands_.push_back(Use(to, this));
    }

    Instruction::Instruction(const std::string & name, Instruction * before)
    {
        if (before->prev_ != nullptr)
        {
            before->prev_->next_ = this;
            this->prev_ = before->prev_;
        }
        before->prev_ = this;
        this->next_ = before;

        name_ = name;
    }

    Instruction::Instruction(const std::string & name, BasicBlock * end)
    {
        name_ = name;
        end->push(this);
    }

    void Assign::init(Value * value)
    {
        operands_.reserve(1);
        operands_.push_back(Use(value, this));
    }

}
}
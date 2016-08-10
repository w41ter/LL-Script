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

    void Use::replaceValue(Value * value)
    {
        if (value)
            value->addUse(*this);
        if (value_)
            value_->killUse(*this);
        value_ = value;
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
        op_ = bop;
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
        : prev_ (nullptr), next_(nullptr)
    {
        if (before == nullptr)
            return;

        if (before->prev_ != nullptr)
        {
            before->prev_->next_ = this;
            this->prev_ = before->prev_;
        }
        before->prev_ = this;
        this->next_ = before;

        name_ = name;
        this->parent_ = before->parent_;
    }

    Instruction::Instruction(const std::string & name, BasicBlock * end)
        : prev_(nullptr), next_(nullptr)
    {
        if (end == nullptr)
            return;
        name_ = name;
        this->parent_ = end;
        end->push(this);
    }

    void Instruction::eraseFromParent()
    {
        if (this->prev_ == nullptr)
        {
            if (this->next_ == nullptr)
                this->parent_->head_ = this->parent_->end_ = nullptr;
            else
                this->parent_->head_ = this->next_;
        }
        else if (this->next_ == nullptr)
        {
            this->prev_->next_ = nullptr;
            this->parent_->end_ = this->prev_;
        }
        else
        {
            this->prev_->next_ = this->next_;
            this->next_->prev_ = this->prev_;
        }
        this->parent_ = nullptr;
    }

    void Assign::init(Value * value)
    {
        operands_.reserve(1);
        operands_.push_back(Use(value, this));
    }

    void Goto::init(BasicBlock *block)
    {
        block->addPrecursor(this->parent_);
        this->parent_->addSuccessor(block);
    }

    void Phi::appendOperand(Value * value)
    {
        operands_.push_back(Use(value, this));
    }

    void Phi::replaceBy(Value * val)
    {
        for (auto i : this->uses_)
        {
            for (auto iter = i.getUser()->op_begin();
                iter != i.getUser()->op_end();
                ++iter)
            {
                auto &use = *iter;
                if (use.getValue() == val)
                    use.replaceValue(val);
            }
        }
    }

}
}
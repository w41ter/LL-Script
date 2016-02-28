#include <algorithm>

#include "RegisterAllocator.h"

namespace script
{
    RegisterAllocator::RegisterAllocator(
        OpcodeContext &context, BasicBlock * block)
        : context_(context)
    {
        Quad *begin = block->begin(), *end = block->end();
        index_ = 0;
        while (begin != nullptr)
        {
            index_++;
            begin->accept(this);
            // fill use and define
            if (begin == end) break;
            begin = begin->next_;
        }
        for (auto &i : register_)
            i = nullptr;
    }

    RegisterAllocator::~RegisterAllocator()
    {
    }

    Register RegisterAllocator::allocate(Value * temp)
    {
        removeDeadRegister(define_[temp]);

        // for each register if no use insert push 
        Value **reg = register_, **_max = reg;
        int index = 0;
        while (reg != register_ + RG_Total - RG_Begin)
        {
            if (*reg == nullptr)
                break;
            if (use_[*reg] >= index)
            {
                _max = reg;
                index = use_[*reg];
            }
            ++reg;
        }

        if (*reg != nullptr)
        {
            context_.insertPushR(Register(reg - register_));
        }

        *reg = temp;
        reg_[temp] = Register(reg - register_);
        return Register(reg - register_);
    }

    Register RegisterAllocator::getReg(Value * temp)
    {
        // if current temp not in any register, allocate new register and pop it.
        auto reg = reg_[temp];
        if (*(register_ + reg) != temp) // 当前变量被push到栈上去了
        {
            // FIXME: it just garanteed that there always has enough reg
            // for allocate.
            Register r = allocate(temp);
            context_.insertPopR(r);
            return r;
        }
        return reg;
    }

    bool RegisterAllocator::visit(Constant * v)
    {
        return false;
    }

    bool RegisterAllocator::visit(Temp * v)
    {
        return false;
    }

    bool RegisterAllocator::visit(Identifier * v)
    {
        return false;
    }

    bool RegisterAllocator::visit(Array * v)
    {
        return false;
    }

    bool RegisterAllocator::visit(ArrayIndex * v)
    {
        updateUse(v->value_);
        updateUse(v->index_);
        return false;
    }

    bool RegisterAllocator::visit(If * v)
    {
        updateUse(v->condition_);
        return false;
    }

    bool RegisterAllocator::visit(Call * v)
    {
        define_[v->result_] = index_;
        return false;
    }

    bool RegisterAllocator::visit(Goto * v)
    {
        return false;
    }

    bool RegisterAllocator::visit(Copy * v)
    {
        if (v->sour_->kind() == Value::V_Temp)
            updateUse(v->sour_);
        define_[v->dest_] = index_;
        return false;
    }

    bool RegisterAllocator::visit(Load * v)
    {
        v->id_->accept(this);
        define_[v->result_] = index_;
        return false;
    }

    bool RegisterAllocator::visit(Store * v)
    {
        updateUse(v->result_);
        v->id_->accept(this);
        return false;
    }

    bool RegisterAllocator::visit(Label * v)
    {
        return false;
    }

    bool RegisterAllocator::visit(Param * v)
    {
        updateUse(v->Value_);
        return false;
    }

    bool RegisterAllocator::visit(Invoke * v)
    {
        updateUse(v->name_);
        define_[v->result_] = index_;
        return false;
    }

    bool RegisterAllocator::visit(Return * v)
    {
        updateUse(v->arg_);
        return false;
    }

    bool RegisterAllocator::visit(IfFalse * v)
    {
        updateUse(v->condition_);
        return false;
    }

    bool RegisterAllocator::visit(Operation * v)
    {
        updateUse(v->left_);
        updateUse(v->right_);
        define_[v->result_] = index_;
        return false;
    }

    bool RegisterAllocator::visit(AssignArray * v)
    {
        return false;
    }

    bool RegisterAllocator::visit(ArrayAssign * v)
    {
        return false;
    }

    void RegisterAllocator::updateUse(Value * val)
    {
        use_[val] = index_;
    }

    void RegisterAllocator::removeDeadRegister(int index)
    {
        for (auto &i : register_)
            if (i != nullptr && use_[i] < index)
            {
                i = nullptr;
            }
    }
}


#include "QuadGenerator.h"

namespace script
{
    void QuadContext::insertIfFalse(Value * condition, Label * label)
    {
        push(Create<IfFalse>(condition, label));
    }

    void QuadContext::insertStore(Value * id, Value * result)
    {
        push(Create<Store>(id, result));
    }

    void QuadContext::insertLoad(Value * id, Value * result)
    {
        push(Create<Load>(id, result));
    }

    void QuadContext::insertReturn(Value * value)
    {
        push(Create<Return>(value));
    }

    void QuadContext::insertLabel(Label * label)
    {
        push(label);
    }

    void QuadContext::insertGoto(Label * label)
    {
        push(Create<Goto>(label));
    }

    void QuadContext::insertCopy(Value * source, Value * dest)
    {
        push(Create<Copy>(source, dest));
    }

    void QuadContext::insertCall(std::string name, Value * result, int num)
    {
        push(Create<Call>(std::move(name), result, num));
    }

    void QuadContext::insertInvoke(Value * function, Value * result, int num)
    {
        push(Create<Invoke>(function, result, num));
    }

    void QuadContext::insertParam(Value * value)
    {
        push(Create<Param>(value));
    }

    void QuadContext::insertSingle(unsigned op, Value * source, Value * result)
    {
        push(Create<Operation>(op, nullptr, source, result));
    }

    void QuadContext::insertOperation(unsigned op, Value * left, Value * right, Value * result)
    {
        push(Create<Operation>(op, left, right, result));
    }

    void QuadContext::push(Quad * quad)
    {
        end_->next_ = quad;
        end_->next_->prev_ = end_;
        end_ = end_->next_;
        end_->next_ = nullptr;
    }

    void QuadContext::insertIf(Value * condition, Label * label)
    {
        push(Create<If>(condition, label));
    }

}

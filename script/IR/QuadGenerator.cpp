#include "QuadGenerator.h"

namespace script
{
    void QuadContext::insertIfFalse(Value * condition, Label * label)
    {
        end_->next_ = Create<IfFalse>(condition, label);
        end_ = end_->next_;
        end_->next_ = nullptr;
    }

    void QuadContext::insertStore(Value * id, Value * result)
    {
        end_->next_ = Create<Store>(id, result);
        end_ = end_->next_;
        end_->next_ = nullptr;
    }

    void QuadContext::insertLoad(Value * id, Value * result)
    {
        end_->next_ = Create<Load>(id, result);
        end_ = end_->next_;
        end_->next_ = nullptr;
    }

    void QuadContext::insertReturn(Value * value)
    {
        end_->next_ = Create<Return>(value);
        end_ = end_->next_;
        end_->next_ = nullptr;
    }

    void QuadContext::insertLabel(Label * label)
    {
        end_->next_ = label;
        end_ = end_->next_;
        end_->next_ = nullptr;
    }

    void QuadContext::insertGoto(Label * label)
    {
        end_->next_ = Create<Goto>(label);
        end_ = end_->next_;
        end_->next_ = nullptr;
    }

    void QuadContext::insertCopy(Value * source, Value * dest)
    {
        end_->next_ = Create<Copy>(source, dest);
        end_ = end_->next_;
        end_->next_ = nullptr;
    }

    void QuadContext::insertCall(std::string name, Value * result, int num)
    {
        end_->next_ = Create<Call>(std::move(name), result, num);
        end_ = end_->next_;
        end_->next_ = nullptr;
    }

    void QuadContext::insertInvoke(Value * function, Value * result, int num)
    {
        end_->next_ = Create<Invoke>(function, result, num);
        end_ = end_->next_;
        end_->next_ = nullptr;
    }

    void QuadContext::insertParam(Value * value)
    {
        end_->next_ = Create<Param>(value);
        end_ = end_->next_;
        end_->next_ = nullptr;
    }

    void QuadContext::insertSingle(unsigned op, Value * source, Value * result)
    {
        end_->next_ = Create<Operation>(op, nullptr, source, result);
        end_ = end_->next_;
        end_->next_ = nullptr;
    }

    void QuadContext::insertOperation(unsigned op, Value * left, Value * right, Value * result)
    {
        end_->next_ = Create<Operation>(op, left, right, result);
        end_ = end_->next_;
        end_->next_ = nullptr;
    }

    void QuadContext::insertIf(Value * condition, Label * label)
    {
        end_->next_ = Create<If>(condition, label);
        end_ = end_->next_;
        end_->next_ = nullptr;
    }

}

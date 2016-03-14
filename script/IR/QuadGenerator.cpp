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

    void QuadContext::insertCall(std::string name, Value * result, int num, int total)
    {
        push(Create<Call>(std::move(name), result, num, total));
    }

    void QuadContext::insertInvoke(Value * function, Value * result, int num)
    {
        push(Create<Invoke>(function, result, num));
    }

    void QuadContext::destroy()
    {
        for (auto &i : codes_)
            delete i, i = nullptr;
        for (auto &i : values_)
            delete i, i = nullptr;
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

    LabelTarget & LabelTarget::instance()
    {
        static LabelTarget tar;
        return tar;
    }

    Quad * LabelTarget::getTarget(Quad * quad)
    {
        quad->accept(this);
        return target_;
    }

    bool LabelTarget::visit(Constant *v) 
    {
        return false; 
    }

    bool LabelTarget::visit(Temp *v) 
    { 
        return false; 
    }

    bool LabelTarget::visit(Identifier *v) 
    {
        return false; 
    }

    bool LabelTarget::visit(Array *v) 
    {
        return false; 
    }

    bool LabelTarget::visit(ArrayIndex *v) 
    {
        return false; 
    }

    bool LabelTarget::visit(If *v)
    {
        target_ = v; return true;
    }

    bool LabelTarget::visit(Call *v) 
    {
        target_ = v; return false; 
    }

    bool LabelTarget::visit(Goto *v) 
    {
        target_ = v; return true; 
    }

    bool LabelTarget::visit(Copy *v) 
    {
        target_ = v; return false;
    }

    bool LabelTarget::visit(Load *v) 
    { 
        target_ = v; return false; 
    }

    bool LabelTarget::visit(Store *v)
    {
        target_ = v; return false; 
    }

    bool LabelTarget::visit(Label *v) 
    {
        if (v->next_ != nullptr) 
            v->next_->accept(this);
        else 
            target_ = nullptr; 
        return true; 
    }

    bool LabelTarget::visit(Param *v) 
    {
        target_ = v; return false; 
    }

    bool LabelTarget::visit(Invoke *v) 
    {
        target_ = v; return false; 
    }

    bool LabelTarget::visit(Return *v) 
    {
        target_ = v; return true; 
    }
    // 
    bool LabelTarget::visit(IfFalse *v) 
    {
        target_ = v; return true; 
    }

    bool LabelTarget::visit(Operation *v) 
    {
        target_ = v; return false;
    }

    bool LabelTarget::visit(AssignArray *v) 
    {
        target_ = v; return false; 
    }

    bool LabelTarget::visit(ArrayAssign *v)
    {
        target_ = v; return false; 
    }

}

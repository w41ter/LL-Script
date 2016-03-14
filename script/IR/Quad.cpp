#include <list>

#include "Quad.h"

namespace script
{
    Constant::Constant(int num)
        : type_(T_Integer), num_(num) 
    {
    }

    Constant::Constant(char c)
        : type_(T_Character), c_(c) 
    {
    }

    Constant::Constant(float fnum)
        : type_(T_Float), fnum_(fnum) 
    {
    }

    Constant::Constant(std::string str)
        : type_(T_String), str_(std::move(str)) 
    {
    }

    unsigned Constant::kind() const
    {
        return V_Constant;
    }

    bool Constant::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }

    Identifier::Identifier(std::string & name)
        : name_(name) 
    {
    }

    unsigned Identifier::kind() const
    {
        return V_Identifier;
    }

    bool Identifier::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }

    Temp::Temp()
    {
        std::stringstream str;
        str << "temp@_" << getIndex();
        name_ = str.str();
    }

    bool Temp::isTemp() const
    {
        return true;
    }

    unsigned Temp::kind() const
    {
        return V_Temp;
    }

    bool Temp::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }

    int Temp::getIndex()
    {
        static int index = 0;
        return index++;
    }

    Array::Array(int total)
        : total_(total) 
    {
    }

    unsigned Array::kind() const
    {
        return V_Array;
    }

    bool Array::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }

    ArrayIndex::ArrayIndex(Value * value, Value * index)
        : value_(value), index_(index)
    {
    }

    unsigned ArrayIndex::kind() const
    {
        return V_ArrayIndex;
    }

    bool ArrayIndex::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }

    Goto::Goto(Label * label)
        : label_(label) 
    {
    }

    bool Goto::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }

    Label::Label()
    {
        std::stringstream str;
        str << "@label_" << getIndex();
        name_ = str.str();
    }

    bool Label::accept(QuadVisitor * v) 
    { 
        return v->visit(this); 
    }

    int Label::getIndex()
    {
        static int index = 0;
        return index++;
    }

    Operation::Operation(unsigned op, Value * left, Value * right, Value * result)
        : op_(op), left_(left), right_(right), result_(result)
    {
    }

    bool Operation::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }

    Copy::Copy(Value * sour, Value * dest)
        : sour_(sour), dest_(dest) 
    {
    }

    bool Copy::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }

    ArrayAssign::ArrayAssign(Value * arg, Value * index, Value * result)
        : arg_(arg), index_(index), result_(result)
    {
    }

    bool ArrayAssign::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }


    AssignArray::AssignArray(Value * arg, Value * index, Value * result)
        : arg_(arg), index_(index), result_(result)
    {
    }

    bool AssignArray::accept(QuadVisitor * v) 
    { 
        return v->visit(this); 
    }

    Load::Load(Value * id, Value * result)
        : id_(id), result_(result)
    {
    }

    bool Load::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }

    Store::Store(Value * id, Value * result)
        : id_(id), result_(result)
    {
    }

    bool Store::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }

    If::If(Value * condition, Label * label)
        : condition_(condition)
        , label_(label)
    {
    }

    bool If::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }

    IfFalse::IfFalse(Value * condition, Label * label)
        : If(condition, label)
    {
    }

    bool IfFalse::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }

    Param::Param(Value * v) 
        : Value_(v)
    {
    }

    bool Param::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }

    Call::Call(std::string name, Value * result, int num, int total)
        : name_(std::move(name)), result_(result), num_(num), total_(total)
    {
    }

    bool Call::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }

    Invoke::Invoke(Value * name, Value * result, int num)
        : name_(name), result_(result), num_(num)
    {
    }

    bool Invoke::accept(QuadVisitor * v)
    {
        return v->visit(this); 
    }

    Return::Return(Value * arg) 
        : arg_(arg) 
    {
    }

    bool Return::accept(QuadVisitor * v)
    {
        return v->visit(this);
    }

}

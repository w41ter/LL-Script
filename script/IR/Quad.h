#ifndef __QUAD_H__
#define __QUAD_H__

#include <vector>
#include <string>
#include <sstream>

namespace script
{
    class QuadVisitor;

    //
    // Value include character integer float string
    // id or temp
    //
    class Value
    {
    public:
        enum {
            V_None,
            V_Constant,
            V_Identifier,
            V_Temp,
            V_Array,
            V_ArrayIndex,
        };

        virtual ~Value() {}
        virtual bool accept(QuadVisitor *v) = 0;

        virtual unsigned kind() const { return V_None; }
        virtual bool isTemp() const { return false; }
    };

    class Constant : public Value
    {
    public:
        enum 
        {
            T_Character,
            T_Integer,
            T_Float,
            T_String,
        };

        Constant(int num);
        Constant(char c);
        Constant(float fnum);
        Constant(std::string str);

        virtual ~Constant() = default;
        virtual unsigned kind() const override;
        virtual bool accept(QuadVisitor *v) override;

        unsigned type_;
        int num_;
        char c_;
        float fnum_;
        std::string str_;
    };

    class Identifier : public Value
    {
    public:
        Identifier(std::string &name);
        virtual ~Identifier() = default;
        virtual unsigned kind() const override;
        virtual bool accept(QuadVisitor *v) override;
        std::string name_;
    };

    class Temp : public Value
    {
    public:
        Temp();
        virtual ~Temp() = default;

        virtual bool isTemp() const override;
        virtual unsigned kind() const override;
        virtual bool accept(QuadVisitor *v) override;

        static int getIndex();

        std::string name_;
    };

    class Array : public Value
    {
    public:
        Array(int total);

        virtual ~Array() = default;
        virtual unsigned kind() const override;
        virtual bool accept(QuadVisitor *v) override;

        int total_;
    };

    class ArrayIndex : public Value
    {
    public:
        ArrayIndex(Value *value, Value *index);

        virtual unsigned kind() const override;
        virtual bool accept(QuadVisitor *v) override;

        Value *value_;
        Value *index_;
    };

    // 
    // Stms goto call param if if_false label invoke
    // 
    class Quad
    {
    public:
        virtual ~Quad() = default;
        virtual bool accept(QuadVisitor *v) = 0;

        Quad *prev_ = nullptr;
        Quad *next_ = nullptr;
    };

    class Label : public Quad
    {
    public:
        Label();
        virtual ~Label() = default;
        virtual bool accept(QuadVisitor *v) override;

        static int getIndex();

        std::string name_;
    };

    //
    // goto specl label
    // 
    class Goto : public Quad
    {
    public:
        Goto(Label *label);
        virtual ~Goto() = default;
        virtual bool accept(QuadVisitor *v) override;

        Label *label_;
    };
    
    // 
    // define add sub mul div not g gt l lt e ne
    class Operation : public Quad
    {
    public: 
        Operation(unsigned op, Value *left, Value *right, Value *result);
        virtual ~Operation() = default;
        virtual bool accept(QuadVisitor *v) override;

        unsigned op_;
        Value *left_;
        Value *right_;
        Value *result_;
    };

    class Copy : public Quad 
    {
    public:
        Copy(Value *sour, Value *dest);
        virtual ~Copy() = default;
        virtual bool accept(QuadVisitor *v) override;

        Value *sour_;
        Value *dest_;
    };

    class ArrayAssign : public Quad
    {
    public:
        ArrayAssign(Value *arg, Value *index, Value *result);

        virtual ~ArrayAssign() = default;
        virtual bool accept(QuadVisitor *v) override;

        Value *arg_;
        Value *index_;
        Value *result_;
    };

    class AssignArray : public Quad
    {
    public:
        AssignArray(Value *arg, Value *index, Value *result);

        virtual ~AssignArray() = default;
        virtual bool accept(QuadVisitor *v) override;

        Value *arg_;
        Value *index_;
        Value *result_;
    };

    class Load : public Quad
    {
    public:
        Load(Value *id, Value *result);
        virtual bool accept(QuadVisitor *v) override;

        Value *id_;         // Identifier
        Value *result_;
    };

    class Store : public Quad
    {
    public:
        Store(Value *id, Value *result);
        virtual bool accept(QuadVisitor *v) override;

        Value *id_;         // Identifier
        Value *result_;
    };

    class If : public Quad
    {
    public:
        If(Value *condition, Label *label);
        virtual ~If() = default;
        virtual bool accept(QuadVisitor *v) override;

        Value *condition_;
        Label *label_;
    };

    class IfFalse : public If
    {
    public:
        IfFalse(Value *condition, Label *label);
        virtual ~IfFalse() = default;
        virtual bool accept(QuadVisitor *v) override;
    };

    class Param : public Quad
    {
    public:
        Param(Value *v);
        virtual ~Param() = default;
        virtual bool accept(QuadVisitor *v) override;

        Value *Value_;
    };

    class Call : public Quad
    {
    public:
        Call(std::string name, Value *result, int num, int total);
        virtual ~Call() = default;
        virtual bool accept(QuadVisitor *v) override;

        std::string name_;
        Value *result_;
        int num_;
        int total_;
    };

    class Invoke : public Quad
    {
    public:
        Invoke(Value *name, Value *result, int num);
        virtual ~Invoke() = default;
        virtual bool accept(QuadVisitor *v) override;

        Value *name_;
        Value *result_;
        int num_;
    };

    class Return : public Quad
    {
    public:
        Return(Value *arg);
        virtual ~Return() = default;
        virtual bool accept(QuadVisitor *v) override;

        Value *arg_;
    };

    //
    // Visitor
    //
    class QuadVisitor
    {
    public:
        virtual ~QuadVisitor() {}
        virtual bool visit(Constant *v) = 0;
        virtual bool visit(Temp *v) = 0;
        virtual bool visit(Identifier *v) = 0;
        virtual bool visit(Array *v) = 0;
        virtual bool visit(ArrayIndex *v) = 0;

        virtual bool visit(If *v) = 0;
        virtual bool visit(Call *v) = 0;
        virtual bool visit(Goto *v) = 0;
        virtual bool visit(Copy *v) = 0;
        virtual bool visit(Load *v) = 0;
        virtual bool visit(Store *v) = 0;
        virtual bool visit(Label *v) = 0;
        virtual bool visit(Param *v) = 0;
        virtual bool visit(Invoke *v) = 0;
        virtual bool visit(Return *v) = 0;
        virtual bool visit(IfFalse *v) = 0;
        virtual bool visit(Operation *v) = 0;
        virtual bool visit(AssignArray *v) = 0;
        virtual bool visit(ArrayAssign *v) = 0;
    };

}

#endif // !__QUAD_H__

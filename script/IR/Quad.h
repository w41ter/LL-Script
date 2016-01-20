#ifndef __QUAD_H__
#define __QUAD_H__

#include <vector>
#include <string>
#include <sstream>

namespace script
{
    
    class Value;
    class Identifier;
    class Temp;
    class Array;
    class ArrayIndex;
    class Constant;

    class If;
    class Call;
    class Goto;
    class Copy;
    class Load;
    class Store;
    class Label;
    class Param;
    class Return;
    class Invoke;
    class IfFalse;
    //class Function;
    class Operation;
    class AssignArray;
    class ArrayAssign;
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
        //virtual bool visit(Function *v) = 0;
        virtual bool visit(Operation *v) = 0;
        virtual bool visit(AssignArray *v) = 0;
        virtual bool visit(ArrayAssign *v) = 0;
    };

    //
    // Value include character integer float string
    // id or temp
    //
    class Value
    {
    public:
        virtual ~Value() {}
        virtual bool accept(QuadVisitor *v) = 0;

        virtual bool isVariable() const { return false; }
    };

    class Constant : public Value
    {
    public:
        enum {
            T_Character,
            T_Integer,
            T_Float,
            T_String,
        };
        Constant(int num) : type_(T_Integer), num_(num) {}
        Constant(char c) : type_(T_Character), c_(c) {}
        Constant(float fnum) : type_(T_Float), fnum_(fnum) {}
        Constant(std::string str) : type_(T_String), str_(std::move(str)) {}

        virtual ~Constant() = default;
        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }

        unsigned type_;
        int num_;
        char c_;
        float fnum_;
        std::string str_;
    };

    class Identifier : public Value
    {
    public:
        Identifier(std::string &name) : name_(name) {}
        virtual ~Identifier() = default;
        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }
        virtual bool isVariable() const override { return true; }
        std::string name_;
    };

    class Temp : public Value
    {
    public:
        Temp()
        {
            std::stringstream str;
            str << "temp@_" << getIndex();
            //str >> name_;
            name_ = str.str();
        }
        virtual ~Temp() = default;

        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }

        static int getIndex()
        {
            static int index = 0;
            return index++;
        }

        std::string name_;
    };

    class Array : public Value
    {
    public:
        Array(int total) : total_(total) {}
        virtual ~Array() = default;
        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }
        int total_;
    };

    class ArrayIndex : public Value
    {
    public:
        ArrayIndex(Value *value, Value *index)
            : value_(value), index_(index)
        {}

        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }
        virtual bool isVariable() const override { return true; }

        Value *value_;
        Value *index_;
    };

    // 
    // Stms goto call param if if_false label invoke
    // 
    class Quad
    {
    public:
        virtual ~Quad() {}
        virtual bool accept(QuadVisitor *v) = 0;

        Quad *prev_ = nullptr;
        Quad *next_ = nullptr;
    };

    //
    // goto specl label
    // 
    class Goto : public Quad
    {
    public:
        Goto(Label *label) : label_(label) {}
        virtual ~Goto() = default;
        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }
        Label *label_;
    };
    
    class Label : public Quad
    {
    public:
        Label()
        {
            std::stringstream str;
            str << "@label_" << getIndex();
            //str >> name_;
            name_ = str.str();
        }
        virtual ~Label() = default;

        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }

        static int getIndex()
        {
            static int index = 0;
            return index++;
        }

        std::string name_;
    };

    // 
    // define add sub mul div not g gt l lt e ne
    class Operation : public Quad
    {
    public: 
        Operation(unsigned op, Value *left, Value *right, Value *result)
            : op_(op), left_(left), right_(right), result_(result)
        {}
        virtual ~Operation() {}
        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }

        unsigned op_;
        Value *left_;
        Value *right_;
        Value *result_;
    };

    class Copy : public Quad 
    {
    public:
        Copy(Value *sour, Value *dest) : sour_(sour), dest_(dest) {}
        virtual ~Copy() {}

        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }

        Value *sour_;
        Value *dest_;
    };

    class ArrayAssign : public Quad
    {
    public:
        ArrayAssign(Value *arg, Value *index, Value *result)
            : arg_(arg), index_(index), result_(result)
        {}

        virtual ~ArrayAssign() = default;
        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }

        Value *arg_;
        Value *index_;
        Value *result_;
    };

    class AssignArray : public Quad
    {
    public:
        AssignArray(Value *arg, Value *index, Value *result)
            : arg_(arg), index_(index), result_(result)
        {}

        virtual ~AssignArray() = default;
        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }

        Value *arg_;
        Value *index_;
        Value *result_;
    };

    class Load : public Quad
    {
    public:
        Load(Value *id, Value *result)
            : id_(id), result_(result)
        {}
        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }
        Value *id_;         // Identifier
        Value *result_;
    };

    class Store : public Quad
    {
    public:
        Store(Value *id, Value *result)
            : id_(id), result_(result)
        {}
        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }
        Value *id_;         // Identifier
        Value *result_;
    };

    class If : public Quad
    {
    public:
        If(Value *condition, Label *label) 
            : condition_(condition)
            , label_(label)
        {}
        virtual ~If() = default;
        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }

        Value *condition_;
        Label *label_;
    };

    class IfFalse : public If
    {
    public:
        IfFalse(Value *condition, Label *label)
            : If(condition, label)
        {}
        virtual ~IfFalse() = default;
        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }
    };

    //class Function : public Quad
    //{
    //public:
    //    Function(std::string &name, Label *begin, Label *end)
    //        : name_(name), begin_(begin), end_(end)
    //    {}
    //    virtual ~Function() = default;
    //    virtual bool accept(QuadVisitor *v) override { return v->visit(this); }

    //    std::string name_;
    //    Label *begin_;
    //    Label *end_;
    //};

    class Param : public Quad
    {
    public:
        Param(Value *v) : Value_(v) {}
        virtual ~Param() = default;
        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }

        Value *Value_;
    };

    class Call : public Quad
    {
    public:
        Call(std::string name, Value *result, int num)
            : name_(std::move(name)), result_(result), num_(num)
        {}
        virtual ~Call() = default;
        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }

        std::string name_;
        //Label *position_;
        Value *result_;
        int num_;
    };

    class Invoke : public Quad
    {
    public:
        Invoke(Value *name, Value *result, int num)
            : name_(name), result_(result), num_(num)
        {}
        virtual ~Invoke() = default;
        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }
        Value *name_;
        Value *result_;
        int num_;
    };

    class Return : public Quad
    {
    public:
        Return(Value *arg) : arg_(arg) {}
        virtual ~Return() = default;
        virtual bool accept(QuadVisitor *v) override { return v->visit(this); }
        Value *arg_;
    };

    //class QuadManager
    //{
    //    QuadManager() = default;
    //    ~QuadManager() { destroy(); }

    //    std::list<Quad*> manager_;
    //    std::list<Value*> values_;
    //public:
    //    static QuadManager &instance()
    //    {
    //        static QuadManager manager;
    //        return manager;
    //    }

    //    void push_back(Quad *quad) { manager_.push_back(quad); }
    //    void insert_value(Value *value) { values_.push_back(value); }

    //    void destroy()
    //    {
    //        for (auto &i : manager_)
    //            delete i, i = nullptr;
    //        for (auto &i : values_)
    //            delete i, i = nullptr;
    //    }
    //};

    //template<typename T, typename ...Args>
    //T *Create(Args ...args)
    //{
    //    auto &ins = QuadManager::instance();
    //    auto *buffer = new T(args...);
    //    ins.push_back(buffer);
    //    return buffer;
    //}

    //template<typename T, typename ...Args>
    //T *CreateValue(Args ...args)
    //{
    //    auto &ins = QuadManager::instance();
    //    auto *buffer = new T(args...);
    //    ins.insert_value(buffer);
    //    return buffer;
    //}

}

#endif // !__QUAD_H__

#ifndef __QUAD_H__
#define __QUAD_H__

#include <vector>
#include <string>
#include <sstream>

namespace script
{
    class If;
    class Var;
    class Float;
    class Integer;
    class Character;
    class String;
    class Call;
    class Goto;
    class Temp;
    class Copy;
    class Label;
    class Param;
    class Return;
    class Array;
    class IfFalse;
    class Function;
    class RelopAssign;
    class ArrayAssign;
    class AssignArray;
    class Identifier;
    class SingleAssign;
    class BinaryAssign;

    class QuadVisitor
    {
    public:
        virtual ~QuadVisitor() = 0 {};
        virtual bool visit(If &v) = 0;
        virtual bool visit(Float &v) = 0;
        virtual bool visit(Integer &v) = 0;
        virtual bool visit(Character &v) = 0;
        virtual bool visit(String &v) = 0;
        virtual bool visit(Call &v) = 0;
        virtual bool visit(Goto &v) = 0;
        virtual bool visit(Temp &v) = 0;
        virtual bool visit(Copy &v) = 0;
        virtual bool visit(Label &v) = 0;
        virtual bool visit(Array &v) = 0;
        virtual bool visit(Param &v) = 0;
        virtual bool visit(Return &v) = 0;
        virtual bool visit(IfFalse &v) = 0;
        virtual bool visit(Function &v) = 0;
        virtual bool visit(RelopAssign &v) = 0;
        virtual bool visit(AssignArray &v) = 0;
        virtual bool visit(ArrayAssign &v) = 0;
        virtual bool visit(BinaryAssign &v) = 0;
        virtual bool visit(Identifier &v) = 0;
        virtual bool visit(SingleAssign &v) = 0;
    };

    class Quad
    {
    public:
        virtual ~Quad() = 0 {}
        virtual bool accept(QuadVisitor &v) = 0;
    };

    class Var : public Quad
    {
    public:
        virtual ~Var() {}
        virtual bool accept(QuadVisitor &v) = 0;
    };

    class Float : public Var
    {
    public:
        Float(float f) : f_(f) {}
        virtual ~Float() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }
        float f_;
    };

    class Integer : public Var
    {
    public:
        Integer(int i) : i_(i) {}
        virtual ~Integer() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }
        int i_;
    };

    class Character : public Var
    {
    public:
        Character(char c) : c_(c) {}
        virtual ~Character() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }
        char c_;
    };

    class String : public Var
    {
    public:
        String(std::string &str) : str_(str) {}
        virtual ~String() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }
        std::string &str_;
    };

    class Identifier : public Var
    {
    public:
        Identifier(std::string &name) : name_(name) {}
        virtual ~Identifier() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }
        std::string name_;
    };

    class Goto : public Quad
    {
    public:
        Goto(Label *label) : label_(label) {}
        virtual ~Goto() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }
        Label *label_;
    };

    class Array : public Var
    {
    public:
        Array(int total) : total_(total) {}
        virtual ~Array() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }
        int total_;
    };

    class Temp : public Var
    {
    public: 
        Temp() 
        { 
            std::stringstream str("temp@_");
            str << getIndex();
            str >> name_;
        }
        virtual ~Temp() = default;

        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }
        
        static int getIndex()
        {
            static int index = 0;
            return index++;
        }

        std::string name_;
    };

    class Label : public Quad
    {
    public:
        Label()
        {
            std::stringstream str("@label_");
            str << getIndex();
            str >> name_;
        }
        virtual ~Label() = default;

        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }

        static int getIndex()
        {
            static int index = 0;
            return index++;
        }

        std::string name_;
    };

    class BinaryAssign : public Quad
    {
    public:
        enum OP {
            ADD,
            SUB,
            MUL,
            DIV
        };
        BinaryAssign(OP op, Var *left, Var *right, Var *des)
            : op_(op), left_(left), right_(right), result_(des)
        {}

        virtual ~BinaryAssign() = default;

        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }

        OP op_;
        Var *left_;
        Var *right_;
        Var *result_;
    };

    class SingleAssign : public Quad
    {
    public:
        enum OP { NOT, NAGTIVE};

        SingleAssign(OP op, Var *arg, Var *res)
            : op_(op), arg_(arg), result_(res)
        {}

        virtual ~SingleAssign() = default;

        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }

        Var *arg_;
        Var *result_;
        OP op_;
    };

    class Copy : public Quad 
    {
    public:
        Copy(Var *arg, Var *res) : arg_(arg), result_(res) {}
        virtual ~Copy() {}

        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }

        Var *arg_;
        Var *result_;
    };

    class ArrayAssign : public Quad
    {
    public:
        ArrayAssign(Var *arg, Var *index, Var *result)
            : arg_(arg), index_(index), result_(result)
        {}

        virtual ~ArrayAssign() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }

        Var *arg_;
        Var *index_;
        Var *result_;
    };

    class AssignArray : public Quad
    {
    public:
        AssignArray(Var *arg, Var *index, Var *result)
            : arg_(arg), index_(index), result_(result)
        {}

        virtual ~AssignArray() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }

        Var *arg_;
        Var *index_;
        Var *result_;
    };

    class If : public Quad
    {
    public:
        If(Var *condition, Label *label) 
            : condition_(condition)
            , label_(label)
        {}
        virtual ~If() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }

        Var *condition_;
        Label *label_;
    };

    class IfFalse : public If
    {
    public:
        IfFalse(Var *condition, Label *label)
            : If(condition, label)
        {}
        virtual ~IfFalse() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }
    };

    class RelopAssign : public Quad
    {
    public:
        enum OP {
            G,
            GT,
            L,
            LT,
            NE,
            E
        };
        RelopAssign(OP op, Var *left, Var *right, Var *result)
            : op_(op), left_(left), right_(right), result_(result)
        {}
        virtual ~RelopAssign() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }

        OP op_;
        Var *left_;
        Var *right_;
        Var *result_;
    };

    class Function : public Quad
    {
    public:
        Function(std::string &name, Label *begin, Label *end)
            : name_(name), begin_(begin), end_(end)
        {}
        virtual ~Function() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }

        std::string name_;
        Label *begin_;
        Label *end_;
    };

    class Param : public Quad
    {
    public:
        Param(Var *v) : var_(v) {}
        virtual ~Param() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }

        Var *var_;
    };

    class Call : public Quad
    {
    public:
        Call(Var *name, Var *result, int num)
            : name_(name), result_(result), num_(num)
        {}
        virtual ~Call() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }
        Var *name_;
        Var *result_;
        int num_;
    };

    class Return : public Quad
    {
    public:
        Return(Var *arg) : arg_(arg) {}
        virtual ~Return() = default;
        virtual bool accept(QuadVisitor &v) override { return v.visit(*this); }
        Var *arg_;
    };

    class QuadManager
    {
        QuadManager() = default;
        ~QuadManager()
        {
            destroy();
        }

        std::vector<Quad*> manager_;
    public:
        static QuadManager &instance()
        {
            static QuadManager manager;
            return manager;
        }

        void push_back(Quad *quad) { manager_.push_back(quad); }

        void destroy()
        {
            for (auto &i : manager_)
            {
                delete i;
                i = nullptr;
            }
        }
    };

    template<typename T, typename ...Args>
    T *MallocQuad(Args ...args)
    {
        auto &ins = QuadManager::instance();
        auto *buffer = new T(args);
        ins.push_back(buffer);
        return buffer;
    }
}

#endif // !__QUAD_H__

#ifndef __QUAD_GENERATOR_H__
#define __QUAD_GENERATOR_H__

#include <map>
#include <list>

#include "Quad.h"

namespace script
{
    // 
    // a help class
    //
    class QuadContext
    {
        friend class CFG;
        friend class IRCode;
        friend class IRFunction;
        friend class IRModule;

    public:
        QuadContext() { end_ = &begin_; }
        ~QuadContext() { destroy(); }

        void insertParam(Value *value);
        void insertGoto(Label *label);
        void insertLabel(Label *label);
        void insertReturn(Value *value);
        void insertLoad(Value *id, Value *result);
        void insertStore(Value *id, Value *result);
        void insertCopy(Value *source, Value *dest);
        void insertIf(Value *condition, Label *label);
        void insertIfFalse(Value *condition, Label *label);
        void insertInvoke(Value *function, Value *result, int num);
        void insertSingle(unsigned op, Value *source, Value *result);
        void insertCall(std::string name, Value *result, int num, int total);
        void insertOperation(unsigned op, Value *left, Value *right, Value *result);

        template<typename T, typename ...Args>
        T *Create(Args ...args)
        {
            auto *buffer = new T(args...);
            codes_.push_back(buffer);
            return buffer;
        }

        template<typename T, typename ...Args>
        T *CreateValue(Args ...args)
        {
            auto *buffer = new T(args...);
            values_.push_back(buffer);
            return buffer;
        }

        Quad *begin() { return begin_.next_; }
        const Quad *end() const { return end_; }

    private:

        void destroy();

    private:
        void push(Quad *quad);

    private:
        // link
        Label begin_;
        Quad *end_;

        std::list<Quad*> codes_;
        std::list<Value*> values_;
    };

    // 
    // 找到该 label 对应的第一条指令
    //
    class LabelTarget : private QuadVisitor
    {
        LabelTarget() = default;
    public:
        static LabelTarget &instance();

        virtual ~LabelTarget() {}

        Quad *getTarget(Quad *quad);

    private:
        virtual bool visit(Constant *v);
        virtual bool visit(Temp *v);
        virtual bool visit(Identifier *v);
        virtual bool visit(Array *v);
        virtual bool visit(ArrayIndex *v);

        virtual bool visit(If *v);
        virtual bool visit(Call *v);
        virtual bool visit(Goto *v);
        virtual bool visit(Copy *v);
        virtual bool visit(Load *v);
        virtual bool visit(Store *v);
        virtual bool visit(Label *v);
        virtual bool visit(Param *v);
        virtual bool visit(Invoke *v);
        virtual bool visit(Return *v); // 
        virtual bool visit(IfFalse *v);
        virtual bool visit(Operation *v);
        virtual bool visit(AssignArray *v);
        virtual bool visit(ArrayAssign *v);

    private:
        Quad *target_;
    };
}

#endif // !__QUAD_GENERATOR_H__


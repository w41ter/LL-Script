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

        void destroy()
        {
            for (auto &i : codes_)
                delete i, i = nullptr;
            for (auto &i : values_)
                delete i, i = nullptr;
        }

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
        void insertCall(std::string name, Value *result, int num);
        void insertInvoke(Value *function, Value *result, int num);
        void insertSingle(unsigned op, Value *source, Value *result);
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
        // link
        Label begin_;
        Quad *end_;

        std::list<Quad*> codes_;
        std::list<Value*> values_;
    };
}

#endif // !__QUAD_GENERATOR_H__


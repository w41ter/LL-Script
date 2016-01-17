#ifndef __QUAD_GENERATOR_H__
#define __QUAD_GENERATOR_H__

#include <list>

#include "Quad.h"

namespace script
{
    class QuadGenerator
    {
    public:
        QuadGenerator() {}

        void insertFunction(std::string name, Label *begin, Label *end);
        void insertCall(Label *begin, Value *result, int num);
        void insertInvoke(Value *function, Value *result, int num);
        void insertParam(Value *value);
        void insertSingle(unsigned op, Value *source, Value *result);
        void insertOperation(unsigned op, Value *left, Value *right, Value *result);
        void insertIf(Value *condition, Label *label);
        void insertIfFalse(Value *condition, Label *label);
        void insertStore(Value *id, Value *result);
        void insertReturn(Value *value);
        void insertLabel(Label *label);
        void insertGoto(Label *label);
        void insertCopy(Value *source, Value *dest);

        std::list<Quad*> getCode()
        {
            return std::move(codes_);
        }
    private:
        std::list<Quad*> codes_;
    };
}

#endif // !__QUAD_GENERATOR_H__


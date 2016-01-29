#ifndef __REGISTER_ALLOCATOR_H__
#define __REGISTER_ALLOCATOR_H__

#include "../Runtime/opcode.h"
#include "cfg.h"
#include <vector>

namespace script
{
    class RegisterAllocator : private QuadVisitor
    {
    public:
        RegisterAllocator(OpcodeContext &context, BasicBlock *block);
        virtual ~RegisterAllocator();

        Register allocate(Value *temp);
        Register getReg(Value *temp);

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
        virtual bool visit(Return *v);
        virtual bool visit(IfFalse *v);
        virtual bool visit(Operation *v);
        virtual bool visit(AssignArray *v);
        virtual bool visit(ArrayAssign *v);

        void updateUse(Value *val);
        void removeDeadRegister(int index);

    private:
        int index_;
        OpcodeContext &context_;
        std::map<Value*, int> define_;
        std::map<Value*, int> use_;
        std::map<Value*, Register> reg_;
        Value *register_[RG_Total - RG_Begin];
    };
}

#endif // !__REGISTER_ALLOCATOR_H__

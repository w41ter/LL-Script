#ifndef __CODE_GEN_H__
#define __CODE_GEN_H__

#include <set>
#include <map>

#include "IRGenerator.h"
#include "../Runtime/opcode.h"

namespace script
{
    class RegisterAllocator;

    class CodeGenerator : private QuadVisitor
    {
        using String2Int = std::map<std::string, int>;
    public:
        CodeGenerator(OpcodeContext &context);

        void gen(IRModule &module);

    private:
        int backLabel(Quad *label);
        int backFunction(std::string &str);

        int genIRCode(IRCode &code);
        int genBasicBlock(BasicBlock *block);

        virtual bool visit(Constant *v) override;
        virtual bool visit(Temp *v) override;
        virtual bool visit(Identifier *v) override;
        virtual bool visit(Array *v) override;
        virtual bool visit(ArrayIndex *v) override;

        virtual bool visit(If *v) override;
        virtual bool visit(Call *v) override;
        virtual bool visit(Goto *v) override;
        virtual bool visit(Copy *v) override;
        virtual bool visit(Load *v) override;
        virtual bool visit(Store *v) override;
        virtual bool visit(Label *v) override;
        virtual bool visit(Param *v) override;
        virtual bool visit(Invoke *v) override;
        virtual bool visit(Return *v) override;
        virtual bool visit(IfFalse *v) override;
        virtual bool visit(Operation *v) override;
        virtual bool visit(AssignArray *v) override;
        virtual bool visit(ArrayAssign *v) override;

        BasicBlock *targetBasicBlock(Quad *label);
        
        int newLocalSlot(std::string &name);
    private:
        OpcodeContext &context_;

        RegisterAllocator *allocator_ = nullptr;
        String2Int *slotStack_;
        String2Int functions_;
        std::map<Quad*, int> labels_;
        std::map<Quad*, BasicBlock*> blocks_;
        std::set<BasicBlock*> hasVisit_;
    };
}

#endif // !__CODE_GEN_H__


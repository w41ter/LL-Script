#pragma once

#include <string>
#include <fstream>

namespace script
{
    class IRCode;
    class IRFunction;
    class IRModule;
    class BasicBlock;

namespace ir
{
    class Instruction;
    class Constant;
}

    class DumpIR
    {
        using Instruction = ir::Instruction;
        using Constant = ir::Constant;
    public:
        DumpIR(std::string &filename);
        ~DumpIR();

        void dump(const IRModule *module);
        void dump(const IRCode *code);
        void dump(const IRFunction *function);
        
    protected:
        void dumpBlock(const BasicBlock &block);
        void dumpInstr(const Instruction &instr);
        void dumpValue(const Constant *instr);
        void dumpAlloca(const Instruction &instr);
        void dumpLoad(const Instruction &instr);
        void dumpStore(const Instruction &instr);
        void dumpInvoke(const Instruction &instr);
        void dumpBranch(const Instruction &instr);
        void dumpGoto(const Instruction &instr);
        void dumpAssign(const Instruction &instr);
        void dumpNotOp(const Instruction &instr);
        void dumpReturn(const Instruction &instr);
        void dumpReturnVoid(const Instruction &instr);
        void dumpBinaryOps(const Instruction &instr);
        void dumpIndex(const Instruction &instr);
        void dumpSetIndex(const Instruction &instr);
        void dumpPhi(const Instruction &instr);
        void dumpCatch(const Instruction &instr);

    protected:
        std::string filename_;
        std::fstream file_;
    };

}


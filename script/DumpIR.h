#pragma once

#include <string>
#include <fstream>

namespace script
{
    class IRCode;
    class IRFunction;
    class IRModule;
    class BasicBlock;
    class Instruction;
    class Constant;
    class Value;
    
    class DumpIR
    {
    public:
        DumpIR(std::string &filename);
        ~DumpIR();

        void dump(IRModule *module);
        void dump(IRCode *code);
        void dump(IRFunction *function);
        
    protected:
        void dumpBlock(BasicBlock *block);
        void dumpInstr(Instruction *instr);
        void dumpValue(Value *value);
        void dumpConstant(Constant *instr);
        void dumpInvoke(Instruction *instr);
        void dumpBranch(Instruction *instr);
        void dumpGoto(Instruction *instr);
        void dumpAssign(Instruction *instr);
        void dumpNotOp(Instruction *instr);
        void dumpReturn(Instruction *instr);
        void dumpReturnVoid(Instruction *instr);
        void dumpBinaryOps(Instruction *instr);
        void dumpIndex(Instruction *instr);
        void dumpSetIndex(Instruction *instr);
        void dumpPhi(Instruction *instr);
		void dumpStore(Instruction *instr);
		void dumpNewClosure(Instruction *instr);

    protected:
        std::string filename_;
        std::fstream file_;
    };

}


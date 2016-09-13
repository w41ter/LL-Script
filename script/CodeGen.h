#pragma once

#include "Pass.h"
#include <map>

namespace script
{
    class Instruction;
	class Value;
	class Assign;
    class IRCode;
    class Opcodes;
    class IRFunction;
    class IRModule;
	class BasicBlock;
    class OpcodeModule;
    class OpcodeContext;
	class OpcodeFunction;
	class RegisterAllocator;

    class CodeGen : public FunctionPass
    {
    public:
        CodeGen(OpcodeModule &module);
		virtual ~CodeGen() = default;

		void runOnFunction(IRFunction *function);

    protected:
        void genFunction(IRFunction *func);
		void genInstr(
			OpcodeFunction &func,
			Instruction *instr); 
		void genAssign(
			OpcodeFunction &func,  
			Instruction *instr);
        void genBinaryOps(
			OpcodeFunction &func,
			Instruction *instr);
		void genBranch(
			OpcodeFunction &func,
			Instruction *instr);
		void genGoto(
			OpcodeFunction &func,
			Instruction *instr);
		void genIndex(
			OpcodeFunction &func,
			Instruction *instr);
		void genInvoke(
			OpcodeFunction &func,
			Instruction *instr);
		void genNotOp(
			OpcodeFunction &func,
			Instruction *instr);
		void genReturn(
			OpcodeFunction &func,
			Instruction *instr); 
		void genReturnVoid(
			OpcodeFunction &func,
			Instruction *instr); 
		void genSetIndex(
			OpcodeFunction &func,
			Instruction *instr); 
		void genAssignValue(
			OpcodeFunction &func,
			Assign *assign);

		void writeBacktrack(
			BasicBlock *block, 
			Opcodes &opcode, 
			unsigned offset);

		unsigned getParamsSlot(
			OpcodeFunction &func,
			const std::string &name);
    protected:
		size_t numOfRegister;

        OpcodeModule &module_;

        std::map<std::string, int> s2i_;

		std::map<BasicBlock*, unsigned> BBOffset;
		std::multimap<BasicBlock*, unsigned> backtrack;
    };
}
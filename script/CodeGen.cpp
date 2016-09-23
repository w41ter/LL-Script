#include "CodeGen.h"

#include <iostream>

#include "Instruction.h"
#include "IRModule.h"
#include "opcode.h"
#include "OpcodeModule.h"
#include "OpBuilder.h"

#include "LiveInterval.h"
#include "LiveIntervalAnalysis.h"
#include "SimpleRegisterAllocation.h"
#include "PHIElimination.h"

namespace script
{
    CodeGen::CodeGen(OpcodeModule &module)
        : module_(module)
    {
    }

	void CodeGen::runOnFunction(IRFunction *func)
	{
#ifdef _DEBUG
		std::cout << "Live intervals analysis: "
			<< func->getFunctionName() << std::endl;
#endif // _DEBUG 
		std::list<LiveInterval> intervals;
		{
			LiveIntervalAnalysis analysis;
			analysis.runOnFunction(func);
			analysis.swapIntervals(intervals);
		}

#ifdef _DEBUG
		std::cout << "Register allocation: "
			<< func->getFunctionName() << std::endl;
#endif // _DEBUG

		SimpleRegisterAllocation RA(255, intervals);
		RA.runOnFunction(func);

#ifdef _DEBUG
		std::cout << "Eliminate phi node:"
			<< func->getFunctionName() << std::endl;
#endif // _DEBUG

		PhiElimination PE;
		PE.runOnFunction(func);

		numOfRegister = RA.totalRegister();
		genFunction(func);
	}

    void CodeGen::genFunction(IRFunction * func)
    {
        OpcodeFunction &function = module_.getFunction(
			func->getFunctionName());
		function.numOfregisters = numOfRegister;
		function.paramSize = 0;
		for (auto P = func->param_begin();
			P != func->param_end();
			++P) {
			++function.paramSize;
			size_t idx = module_.push_string(*P);
			function.params.push_back(idx);
		}

		// Generate
		for (auto *block : *func) {
			unsigned offset = function.codes.size();
			BBOffset[block] = offset;
			writeBacktrack(block, function, offset);
			for (auto instr = block->instr_begin();
				instr != block->instr_end();
				++instr) {
				genInstr(function, *instr);
			}
		}
    }

	void CodeGen::genInstr(
		OpcodeFunction & function, 
		Instruction * instr)
	{
		Instruction *I = instr;
		switch (I->get_opcode())
		{
		case Instruction::AssignVal:
			genAssign(function, I);
			break;
		case Instruction::BinaryOpsVal:
			genBinaryOps(function, I);
			break;
		case Instruction::BranchVal:
			genBranch(function, I);
			break;
		case Instruction::GotoVal:
			genGoto(function, I);
			break;
		case Instruction::IndexVal:
			genIndex(function, I);
			break;
		case Instruction::InvokeVal:
			genInvoke(function, I);
			break;
		case Instruction::NotOpVal:
			genNotOp(function, I);
			break;
		case Instruction::ReturnVal:
			genReturn(function, I);
			break;
		case Instruction::ReturnVoidVal:
			genReturnVoid(function, I);
			break;
		case Instruction::SetIndexVal:
			genSetIndex(function, I);
			break;
		case Instruction::NewClosureVal:
			genNewClosure(function, I);
			break;
		case Instruction::StoreVal:
			genStore(function, I);
			break;
		default:	// ignore Phi instr.
			break;
		}
	}

	void CodeGen::genNewClosure(
		OpcodeFunction & func, 
		Instruction * I)
	{
		NewClosure *closure = static_cast<NewClosure*>(I);
		auto &registers = I->refInputRegisters();
		unsigned output = I->getOutputReg().getRegisterNum();
		unsigned paramSize = registers.size();
		for (size_t i = 0; i < registers.size(); ++i) {
			OPBuilder::GenParam(func, registers[i].getRegisterNum());
		}
		unsigned offset = module_.push_string(closure->get_func_name());
		OPBuilder::GenNewClosure(func, output, offset, paramSize);
	}

	void CodeGen::genStore(
		OpcodeFunction & func, 
		Instruction * instr)
	{
		Store *store = static_cast<Store*>(instr);
		unsigned offset = getParamsSlot(func, store->get_param_name());
		auto &registers = instr->refInputRegisters();
		assert(registers.size() == 1);
		OPBuilder::GenStore(func, registers[0].getRegisterNum(), offset);
	}

	void CodeGen::genAssign(
		OpcodeFunction &function, 
		Instruction *I)
	{
		Assign *assign = (Assign*)I;

		if (assign->get_num_operands() == 0) {
			unsigned to = assign->getLeftReg().getRegisterNum();
			unsigned from = assign->getRightReg().getRegisterNum();
			OPBuilder::GenMove(function, to, from);
			return;
		}
		Value *RHS = assign->get_value();
		if (RHS->is_instr()) {
			auto &registers = I->refInputRegisters();
			assert(registers.size() == 1);
			unsigned to = I->getOutputReg().getRegisterNum();
			unsigned from = registers[0].getRegisterNum();
			OPBuilder::GenMove(function, to, from);
		}
		else
			genAssignValue(function, assign);
	}

	void CodeGen::genBinaryOps(
		OpcodeFunction & func, 
		Instruction * I)
	{
		BinaryOperator *BO = static_cast<BinaryOperator*>(I);
		auto &registers = I->refInputRegisters();
		assert(registers.size() == 2);

		unsigned OP = BO->op();
		unsigned lhs = registers[0].getRegisterNum();
		unsigned rhs = registers[1].getRegisterNum();
		unsigned result = I->getOutputReg().getRegisterNum();
		OPBuilder::GenBinaryOP(func, OP, lhs, rhs, result);
	}

	void CodeGen::genBranch(
		OpcodeFunction & func,
		Instruction * I)
	{
		Branch *branch = static_cast<Branch*>(I);
		auto &registers = I->refInputRegisters();
		assert(registers.size() == 1);

		unsigned reg = registers[0].getRegisterNum();
		BasicBlock *then = branch->then();
		BasicBlock *else_ = branch->_else();
		if (!BBOffset.count(then)) {
			unsigned offset = OPBuilder::GenIf(func, reg, 0);
			backtrack.insert({ then, offset });
		}
		else {
			OPBuilder::GenIf(func, reg, BBOffset[then]);
		}

		if (!BBOffset.count(else_)) {
			unsigned offset = OPBuilder::GenJmp(func, -1);
			backtrack.insert({ else_, offset });
		}
		else {
			OPBuilder::GenJmp(func, BBOffset[else_]);
		}
	}

	void CodeGen::genGoto(
		OpcodeFunction & func,
		Instruction * I)
	{
		Goto *go = static_cast<Goto*>(I);
		BasicBlock *target = go->block();
		if (!BBOffset.count(target)) {
			unsigned offset = OPBuilder::GenJmp(func, -1);
			backtrack.insert({ target, offset });
		} 
		else {
			OPBuilder::GenJmp(func, BBOffset[target]);
		}
	}

	void CodeGen::genIndex(
		OpcodeFunction & func,
		Instruction * I)
	{
		auto &registers = I->refInputRegisters();
		assert(registers.size() == 2);
		unsigned table = registers[0].getRegisterNum();
		unsigned idx = registers[1].getRegisterNum();
		unsigned res = I->getOutputReg().getRegisterNum();
		OPBuilder::GenIndex(func, table, idx, res);
	}

	void CodeGen::genSetIndex(
		OpcodeFunction & func,
		Instruction * I)
	{
		auto &registers = I->refInputRegisters();
		assert(registers.size() == 3);
		unsigned table = registers[0].getRegisterNum();
		unsigned idx = registers[1].getRegisterNum();
		unsigned to = registers[2].getRegisterNum();
		OPBuilder::GenSetIndex(func, table, idx, to);
	}

	void CodeGen::genAssignValue(
		OpcodeFunction & func, 
		Assign * assign)
	{
		Value *RHS = assign->get_value();
		unsigned output = assign->getOutputReg().getRegisterNum();
		switch (RHS->get_subclass_id())
		{
		case Value::ConstantVal:
		{
			Constant *cons = static_cast<Constant*>(RHS);
			switch (cons->type())
			{
			case Constant::Null:
				OPBuilder::GenMoveN(func, output);
				break;
			case Constant::Boolean:
				OPBuilder::GenMoveInteger(
					func, output, cons->getBoolean());
				break;
			case Constant::Character:
				OPBuilder::GenMoveInteger(
					func, output, cons->getChar());
				break;
			case Constant::Integer:
				OPBuilder::GenMoveInteger(
					func, output, cons->getInteger());
				break;
			case Constant::Float:
				OPBuilder::GenMoveFloat(
					func, output, cons->getFloat());
				break;
			case Constant::String:
			{
				unsigned offset = module_.push_string(cons->getString());
				OPBuilder::GenMoveString(
					func, output, offset);
				break;
			}
			default:
				break;
			}
			break;
		}
		//case Value::FunctionVal:
		//{
		//	unsigned offset = module_.push_string(RHS->get_value_name());
		//	OPBuilder::GenNewClosure(func, output, offset);
		//	break;
		//}
		case Value::ParamVal:
		{
			unsigned index = getParamsSlot(func, RHS->get_value_name());
				OPBuilder::GenLoad(func, output, index);
			break;
		}
		case Value::TableVal:
			OPBuilder::GenNewHash(func, output);
			break;
		case Value::UndefVal:
			assert(0);
		default:
			break;
		}
	}

	void CodeGen::genInvoke(
		OpcodeFunction & func,
		Instruction * I)
	{
		Invoke *invoke = static_cast<Invoke*>(I);
		auto &registers = I->refInputRegisters();
		unsigned output = I->getOutputReg().getRegisterNum();
		assert(registers.size() >= 1);
		unsigned paramSize = registers.size() - 1;
		for (size_t i = 1; i < registers.size(); ++i) {
			OPBuilder::GenParam(func, registers[i].getRegisterNum());
		}

		unsigned reg = registers[0].getRegisterNum();
        if (invoke->is_enable_tail_call()) {
            OPBuilder::GenTailCall(func, output, reg, paramSize);
        }
        else {
            OPBuilder::GenCall(func, output, reg, paramSize);
        }
	}

	void CodeGen::genNotOp(
		OpcodeFunction & func,
		Instruction * I)
	{
		auto &registers = I->refInputRegisters();
		assert(registers.size() == 1);
		unsigned reg = registers[0].getRegisterNum();
		unsigned output = I->getOutputReg().getRegisterNum();
		OPBuilder::GenNotOp(func, output, reg);
	}

	void CodeGen::genReturn(
		OpcodeFunction & func,  
		Instruction * instr)
	{
		Return *ret = static_cast<Return*>(instr);
		auto &registers = instr->refInputRegisters();
		assert(registers.size() == 1);
		if (ret->is_tail_call_return()) {
			return;
		}
		unsigned reg = registers[0].getRegisterNum();
		OPBuilder::GenReturn(func, reg);
	}

	void CodeGen::genReturnVoid(
		OpcodeFunction & func,
		Instruction * instr)
	{
		OPBuilder::GenReturn(func, 0);
	}

	void CodeGen::writeBacktrack(
		BasicBlock * block, 
		Opcodes & opcode, 
		unsigned offset)
	{
		auto size = backtrack.count(block);
		auto iter = backtrack.find(block);
		while (size--) {
			OPBuilder::SetIntegerAt(
				opcode, iter->second, offset
			);
			++iter;
		}
	}

	unsigned CodeGen::getParamsSlot(
		OpcodeFunction & func, 
		const std::string & name)
	{
		unsigned idx = module_.push_string(name);
		for (size_t i = 0; i < func.params.size(); ++i) {
			if (func.params[i] == idx)
				return i;
		}
		return func.params.size();
	}
}
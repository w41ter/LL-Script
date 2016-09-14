#include "OpBuilder.h"

#include "opcode.h"
#include "OpcodeModule.h"
#include "Instruction.h"

namespace script
{
namespace 
{
	void MakeOpcode(
		Opcodes &opcode, 
		Byte op)
	{
		opcode.codes.push_back(op);
	}

	void MakeOpcode(
		Opcodes &opcode, 
		Byte op, 
		Byte one)
	{
		opcode.codes.push_back(op);
		opcode.codes.push_back(one);
	}

	void MakeOpcode(
		Opcodes &opcode, 
		Byte op, 
		Byte one, 
		Byte two)
	{
		opcode.codes.push_back(op);
		opcode.codes.push_back(one);
		opcode.codes.push_back(two);
	}

	void MakeOpcode(
		Opcodes &opcode, 
		Byte op, 
		Byte one, 
		Byte two, 
		Byte three)
	{
		opcode.codes.push_back(op);
		opcode.codes.push_back(one);
		opcode.codes.push_back(two);
		opcode.codes.push_back(three);
	}


	void PushInteger(
		Opcodes &opcode, 
		int32_t num)
	{
	    for (int i = 3; i >= 0; --i)
	    {
	        int offset = i * 8;
	        int v = (num >> offset) & 0xff;
			opcode.codes.push_back(v);
	    }
	}
	
	void SetInteger(
		Opcodes &opcode, 
		int index, 
		int32_t num)
	{
	    for (int i = 3; i >= 0; --i)
	    {
	        opcode.codes[index + i] = num & 0xff;
	        num >>= 8;
	    }
	}
	
}
	void OPBuilder::GenMove(
		Opcodes & opcode,
		unsigned to,
		unsigned from)
	{
		MakeOpcode(opcode, OK_Move, to, from);
	}

	void OPBuilder::GenMoveN(
		Opcodes & opcode, 
		unsigned to)
	{
		MakeOpcode(opcode, OK_MoveN, to);
	}

	void OPBuilder::GenMoveInteger(
		Opcodes & opcode, 
		unsigned to, 
		int32_t nums)
	{
		MakeOpcode(opcode, OK_MoveI, to);
		PushInteger(opcode, nums);
	}

	void OPBuilder::GenMoveFloat(
		Opcodes & opcode, 
		unsigned to, 
		float fnum)
	{
		MakeOpcode(opcode, OK_MoveF, to);
		PushInteger(opcode, *((int32_t*)&fnum));
	}

	void OPBuilder::GenMoveString(
		Opcodes & opcode, 
		unsigned to, 
		int32_t offset)
	{
		MakeOpcode(opcode, OK_MoveS, to);
		PushInteger(opcode, offset);
	}

	void OPBuilder::GenNewClosure(
		Opcodes & opcode, 
		unsigned to, 
		int32_t offset)
	{
		MakeOpcode(opcode, OK_NewClosure, to);
		PushInteger(opcode, offset);
	}

	void OPBuilder::GenNewHash(
		Opcodes & opcode, 
		unsigned to)
	{
		MakeOpcode(opcode, OK_NewHash, to);
	}

	void OPBuilder::GenHalt(
		Opcodes & opcode)
	{
	    MakeOpcode(opcode, OK_Halt);
	}

	void OPBuilder::SetIntegerAt(
		Opcodes & opcode, 
		unsigned offset, 
		int32_t num)
	{
		SetInteger(opcode, offset, num);
	}

	void OPBuilder::GenParam(
		Opcodes & opcode, 
		unsigned reg)
	{
	    MakeOpcode(opcode, OK_Param, reg);
	}

	void OPBuilder::GenNotOp(
		Opcodes & opcode,
		unsigned to,
		unsigned value)
	{
		MakeOpcode(opcode, OK_Not, to, value);
	}

	void OPBuilder::GenReturn(
		Opcodes & opcode, 
		unsigned value)
	{
		MakeOpcode(opcode, OK_Return, value);
	}

	void OPBuilder::GenStore(
		Opcodes & opcode, 
		unsigned from, 
		int32_t slot)
	{
		MakeOpcode(opcode, OK_Store, from);
		PushInteger(opcode, slot);
	}

	void OPBuilder::GenLoad(
		Opcodes & opcode, 
		unsigned to, 
		int32_t slot)
	{
		MakeOpcode(opcode, OK_Load, to);
		PushInteger(opcode, slot);
	}

	unsigned OPBuilder::GenJmp(
		Opcodes & opcode, 
		int32_t target)
	{
		MakeOpcode(opcode, OK_Goto);
		unsigned offset = opcode.codes.size();
		PushInteger(opcode, target);
		return offset;
	}

	unsigned OPBuilder::GenIf(
		Opcodes & opcode, 
		unsigned reg,
		int32_t target)
	{
		MakeOpcode(opcode, OK_If, reg);
		unsigned offset = opcode.codes.size();
		PushInteger(opcode, target);
		return offset;
	}

	void OPBuilder::GenBinaryOP(
		Opcodes & opcode, 
		unsigned op, 
		unsigned regLeft, 
		unsigned regRight, 
		unsigned result)
	{
		Byte OP;
	    switch (op)
	    {
		case BinaryOperator::Add:
			OP = OK_Add;
	        break;
	    case BinaryOperator::Sub:
			OP = OK_Sub;
	        break;
	    case BinaryOperator::Mul:
			OP = OK_Mul;
	        break;
	    case BinaryOperator::Div:
			OP = OK_Div;
	        break;
	    case BinaryOperator::Less:
			OP = OK_Less;
	        break;
	    case BinaryOperator::NotGreat:
			OP = OK_LessThan;
	        break;
	    case BinaryOperator::Great:
			OP = OK_Great;
	        break;
	    case BinaryOperator::NotLess:
			OP = OK_GreatThan;
	        break;
	    case BinaryOperator::NotEqual:
			OP = OK_NotEqual;
	        break;
	    case BinaryOperator::Equal:
			OP = OK_Equal;
	        break;
	    }
	    MakeOpcode(opcode, OP, 
			result, regLeft, regRight);
	}

	void OPBuilder::GenIndex(
		Opcodes & opcode, 
		unsigned table, 
		unsigned index, 
		unsigned result)
	{
		MakeOpcode(opcode, OK_Index, 
			result, table, index);
	}
	
	void OPBuilder::GenSetIndex(
		Opcodes &opcode,
		unsigned table,
		unsigned index,
		unsigned result
	)
	{
		MakeOpcode(opcode, OK_SetIndex, 
			table, index, result);
	}

	void OPBuilder::GenTailCall(
		Opcodes &opcode,
		unsigned to,
		unsigned reg,
		int32_t nums
	)
	{
		MakeOpcode(opcode, OK_TailCall, to, reg);
		PushInteger(opcode, nums);
	}

	void OPBuilder::GenCall(
		Opcodes &opcode,
		unsigned to,
		unsigned reg,
		int32_t nums
	)
	{
		MakeOpcode(opcode, OK_Call, to, reg);
		PushInteger(opcode, nums);
	}
}
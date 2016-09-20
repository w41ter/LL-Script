#pragma once

#include <cstdint>

namespace script
{
	class Opcodes;

	class OPBuilder
	{
	public:
		static void GenMove(
			Opcodes &opcode,
			unsigned to,
			unsigned from);  
		static void GenMoveN(
			Opcodes &opcode,
			unsigned to);
		static void GenMoveInteger(
			Opcodes &opcode,
			unsigned to,
			int32_t  nums);
		static void GenMoveFloat(
			Opcodes &opcode,
			unsigned to,
			float fnum);
		static void GenMoveString(
			Opcodes &opcode,
			unsigned to,
			int32_t offset);
		static void GenNewClosure(
			Opcodes &opcode,
			unsigned to,
			int32_t offset,
			int32_t paramSize);
		static void GenNewHash(
			Opcodes &opcode,
			unsigned to);
		static void GenBinaryOP(
			Opcodes & opcode, 
			unsigned op, 
			unsigned regLeft, 
			unsigned regRight, 
			unsigned result);
		static void GenIndex(
			Opcodes &opcode,
			unsigned table,
			unsigned index,
			unsigned result
		);
		static void GenSetIndex(
			Opcodes &opcode,
			unsigned table,
			unsigned index,
			unsigned result
		);

		static void GenTailCall(
			Opcodes &opcode,
			unsigned to,
			unsigned reg,
			int32_t nums
		);
		static void GenCall(
			Opcodes &opcode,
			unsigned to,
			unsigned reg,
			int32_t nums
		);
		static void GenParam(
			Opcodes &opcode,
			unsigned value
		);
		static void GenNotOp(
			Opcodes &opcode,
			unsigned to,
			unsigned value
		);
		static void GenReturn(
			Opcodes &opcode,
			unsigned value
		);
		static void GenStore(
			Opcodes &opcode,
			unsigned from,
			int32_t slot
		);
		static void GenLoad(
			Opcodes &opcode,
			unsigned to,
			int32_t slot
		);
		static unsigned GenJmp(
			Opcodes &opcode,
			int32_t target
		);
		static unsigned GenIf(
			Opcodes &opcode,
			unsigned reg,
			int32_t target
		);
		static void GenHalt(Opcodes & opcode);
		static void SetIntegerAt(
			Opcodes &opcode, 
			unsigned offset, 
			int32_t num);
	};
}
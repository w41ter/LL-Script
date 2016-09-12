#pragma once

#include "Pass.h"

namespace script
{
	class BasicBlock;
	class RegisterAllocator;

	// 
	class PhiElimination : public FunctionPass
	{
	public:
		PhiElimination() {}
		virtual ~PhiElimination() = default;

		void runOnFunction(IRFunction *function);

	private:
		bool eliminatePhiNodes(IRFunction *func, BasicBlock *block);
		void splitCriticalEdge(IRFunction *func, BasicBlock *block);
		BasicBlock *findEdge(BasicBlock *from, BasicBlock *to);
	};
}
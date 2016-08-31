#include "PHIElimination.h"

#include "CFG.h"
#include "Instruction.h"
#include "IRModule.h"
#include "IRContext.h"
#include "RegisterAllocation.h"

#include <queue>

namespace script
{
	void PhiElimination::runOnFunction(IRFunction *function)
	{
		for (auto *block : *function) {
			eliminatePhiNodes(function, block);
		}
	}

	bool PhiElimination::eliminatePhiNodes(
		IRFunction *function, BasicBlock *block)
	{
		if (!block->numOfInstrs() || !block->front()->is_phi_node())
			return false;

		splitCriticalEdge(function, block);

		for (auto iter = block->instr_begin();
			iter != block->instr_end();
			++iter) {
			Instruction *instr = *iter;
			if (!instr->is_phi_node())
				break;
			
			unsigned to = instr->getOutputReg().getRegisterNum();
			// Now loop over all of the incoming arguments,
			// changing them to copy into the destReg register 
			// in the corresponding predecessor basic block.
			for (auto op = instr->op_begin(); op != instr->op_end(); ++op) {
				Value *OPV = op->get_value();
				assert(!OPV->is_value());
				Instruction *opI = static_cast<Instruction*>(OPV);
				BasicBlock *opBlock = findEdge(opI->get_parent(), block);
				assert(opBlock);

				// Figure out where to insert the copy, which is at the end of the
				// predecessor basic block, but before any terminator/branch
				// instructions...
				BasicBlock::instr_iterator I = opBlock->instr_end();
				if (I == opBlock->instr_begin())
					assert(0 && "Not allow empty block.");
				--I;

				// because split ensure use this reg must be safe.
				IRContext::insertAfter<Assign>(
					I, to, opI->getOutputReg().getRegisterNum());
			}

			// Unlink the Phi node from the basic block.
			// instr->erase_from_parent();
			// jump at CodeGen
		}
		return true;
	}

	void PhiElimination::splitCriticalEdge(
		IRFunction * func, BasicBlock * block)
	{
		if (block->numOfPrecursors() <= 1)
			return;

		BasicBlock *tmpBlock = nullptr;
		auto GetInnerBlock = [&tmpBlock, func, block]
			(const std::string &from, const std::string &to) {
			if (tmpBlock == nullptr) {
				tmpBlock = func->createBasicBlock(from + "-" + to);
				IRContext::createGotoAtEnd(tmpBlock, block);
			}
			return tmpBlock;
		};

		for (auto pre = block->precursor_begin();
			pre != block->precursor_end();
			++pre) {
			BasicBlock *PBB = *pre;
			// Check whether split.
			if (PBB->numOfSuccessors() <= 1)
				continue;

			// split it.
			BasicBlock *BB = GetInnerBlock(
				PBB->getBlockName(), block->getBlockName());
			if (PBB->back()->is_goto()) {
				Goto *GT = static_cast<Goto*>(PBB->back());
				GT->setTarget(BB);
			}
			else if (PBB->back()->is_branch()) {
				Branch *branch = static_cast<Branch*>(PBB->back());
				if (branch->then() == block)
					branch->setThen(BB);
				else
					branch->setElse(BB);
			}
			else {
				assert(0 && "impossible");
			}
		}
	}

	BasicBlock * PhiElimination::findEdge(BasicBlock * from, BasicBlock * to)
	{
		std::queue<BasicBlock*> que;
		std::set<BasicBlock*> visit;

		que.push(from);
		while (!que.empty()) {
			BasicBlock *BB = que.front();
			que.pop();
			visit.insert(BB);

			for (size_t i = 0; i < BB->numOfSuccessors(); ++i) {
				BasicBlock *SBB = BB->successor(i);
				if (SBB == to)
					return BB;
				if (!visit.count(SBB))
					que.push(SBB);
			}
		}
		return nullptr;
	}
}
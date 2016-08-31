#include "RegisterAllocation.h"

#include <climits>
#include <algorithm>

#include "CFG.h"
#include "IRModule.h"
#include "Instruction.h"
#include "MachineRegister.h"

namespace script
{
    RegisterAllocator::RegisterAllocator(
		unsigned regNums, std::list<LiveInterval> &list)
        : RegisterNums(regNums)
		, intervals(list)
    {
    }

    void RegisterAllocator::initAndSortIntervals()
    {
		for (auto &interval : intervals) {
			unhandleSet.insert(&interval);
			V2L.insert({ interval.reg, &interval });
		}
    }

	void RegisterAllocator::resolveDataFlow(IRFunction *func)
	{
		MoveResolve resolver;
		for (auto *from : *func) {
			for (size_t i = 0; i < from->numOfSuccessors(); ++i) {
				BasicBlock *to = from->successor(i);
				// collect all resolving moves necessary between the 
				// block from and to 
				for (auto *value : to->liveIn_) {
					assert(value->is_instr());
					Instruction *I = static_cast<Instruction*>(value);
					LiveInterval *parent = V2L[value];
					LiveInterval *fromInterval = parent->childAt(from->back()->getID());
					LiveInterval *toInterval = parent->childAt(to->front()->getID());
					if (fromInterval != toInterval) {
						resolver.addMapping(fromInterval, toInterval);
					}
				}
				// the moves are inserted either at the end of block from 
				// or at the beginning of block to, depending on the control flow.
				resolver.findInsertPosition(from, to);

				// insert all moves in correct order 
				resolver.resolverMappings();
			}
		}
	}

	void RegisterAllocator::splitLiveIntervalAt(
		LiveInterval &interval, unsigned op)
	{
		LiveInterval tmp{ interval.reg };
	}

	void RegisterAllocator::splitLiveIntervalBefore(
		LiveInterval &interval, unsigned op)
	{
		LiveInterval tmp{ interval.reg };
		auto removeRanges = [op, &tmp](LiveRange &range) {
			if (range.contains(op)) {
				tmp.addRange({ op, range.end });
				range.setEnd(op);
				return false;
			}
			else if (op > range.end)
				return false;
			tmp.addRange(range);
			return true;
		};
		interval.ranges.remove_if(removeRanges);

		auto use = std::lower_bound(interval.usePositions.begin(),
			interval.usePositions.end(), op);
		tmp.usePositions.assign(use, interval.usePositions.end());
		interval.usePositions.erase(use, interval.usePositions.end());
		if (interval.isSplitParent())
			tmp.setSplitParent(&interval);
		else
			tmp.setSplitParent(interval.getSplitParent());
		intervals.push_back(tmp);
		unhandleSet.insert(&intervals.back());
	}

    void RegisterAllocator::runOnFunction(IRFunction *func) 
    {
        linearScanAllocate(func);
    }

    void RegisterAllocator::linearScanAllocate(IRFunction *func)
    {   
		initAndSortIntervals();
        while (!unhandleSet.empty()) {
			auto beg = unhandleSet.begin();
			LiveInterval &interval = **unhandleSet.begin();
            // expired Old Intervals.
			expiredOldIntervals(interval);
			if (tryAllocateFreeReg(interval)) {
				allocateBlockedReg(interval);
			}

			if (interval.isAssignReg())
				active.insert(&interval);
			unhandleSet.erase(&interval);
        }
		resolveDataFlow(func);
    }

	void RegisterAllocator::assignRegNum(IRFunction * func)
	{
		for (auto *block : *func) {
			for (auto instr = block->instr_begin();
				instr != block->instr_end();
				++instr) {
				Instruction *I = *instr;
				if (I->is_output()) {
					unsigned reg = V2L[I]->childAt(I->getID())->assignedReg;
					I->setOutputReg({ reg });
				}
				for (auto op = I->op_begin();
					op != I->op_end();
					++op) {
					Value *val = op->get_value();
					if (val->is_value())
						continue;
					Instruction *I = static_cast<Instruction*>(val);
					unsigned reg = V2L[I]->childAt(I->getID())->assignedReg;
					I->pushInputReg({ reg });
				}
			}
		}
	}

	bool RegisterAllocator::tryAllocateFreeReg(LiveInterval &interval)
	{
		std::vector<unsigned> freePos(RegisterNums, INT_MAX);

		for (auto *I : active) {
			freePos[I->getAssignReg()] = 0;
		}

		for (auto *I : inactive) {
			if (I->overlaps(interval)) {
				auto iter = I->advanceTo(
					I->begin(), interval.beginNumber());
				freePos[I->getAssignReg()] = std::min(
					iter->start, freePos[I->getAssignReg()]);
			}
		}

		auto reg = std::max_element(freePos.begin(), freePos.end());
		if (freePos[*reg] == 0)
			// allocation failed, no register available without spilling.
			return false;
		else if (freePos[*reg] > interval.endNumber())
			// register available for whole current.
			interval.assignedReg = *reg;
		else {
			// register available for first part of current.
			interval.assignedReg = *reg;
			splitLiveIntervalBefore(interval, freePos[*reg]);
		}
		return true;
	}

	void RegisterAllocator::allocateBlockedReg(LiveInterval &interval)
	{
		std::vector<unsigned> nextUsePos(RegisterNums, INT_MAX);
		std::vector<LiveInterval*> values(RegisterNums);

		unsigned pos = interval.beginNumber();
		for (auto *I : active) {
			if (nextUsePos[I->getAssignReg()] > I->nextUseAfter(pos)) {
				values[I->getAssignReg()] = I;
			}
		}

		for (auto *I : inactive) {
			if (!I->overlaps(interval)) 
				continue;
			if (nextUsePos[I->getAssignReg()] > I->nextUseAfter(pos)) {
				values[I->getAssignReg()] = I;
			}
		}

		auto reg = std::max_element(nextUsePos.begin(), nextUsePos.end());
		if (interval.firstUse() > nextUsePos[*reg]) {
			// all other intervals are used before current, 
			// so it is best to spill current itself
			// TODO: assign spill slot to current
			splitLiveIntervalBefore(interval, interval.firstUse());
		}
		else {
			interval.assignedReg = *reg;
			splitLiveIntervalAt(interval, interval.beginNumber());
			splitLiveIntervalAt(*values[*reg], interval.beginNumber());
		}
	}

	void RegisterAllocator::expiredOldIntervals(LiveInterval & interval)
	{
		ActiveSet newActive;
		ActiveSet newInactive;
		for (auto *I : active) {
			if (I->endNumber() < interval.beginNumber())
				continue;
			else if (!I->liveAt(interval.beginNumber()))
				newInactive.insert(I);
			else
				newActive.insert(I);
		};
		for (auto *I : inactive) {
			if (I->endNumber() < interval.beginNumber())
				continue;
			else if (I->liveAt(interval.beginNumber()))
				newActive.insert(I);
			else
				newInactive.insert(I);
		}
		std::swap(newActive, active);
		std::swap(newInactive, inactive);
	}

}
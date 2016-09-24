#include "SimpleRegisterAllocation.h"

#include <climits>

#include "CFG.h"
#include "IRModule.h"
#include "Instruction.h" 
#include "LiveInterval.h"
#include "MachineRegister.h"

#include <algorithm>

namespace script
{
	struct IntervalEndCmp {
		bool operator () (const LiveInterval *LHS,
			const LiveInterval *RHS) const {
			auto l = LHS->endNumber(), r = RHS->endNumber();
			if (l == r)
				return LHS < RHS;
			else
				return l < r;
		}
	};

	SimpleRegisterAllocation::SimpleRegisterAllocation(
		size_t regNums, std::list<LiveInterval> &I)
		: totalReg(0)
		, intervals(I)
		, maxRegs(regNums)
	{
	}

	void SimpleRegisterAllocation::runOnFunction(IRFunction * func)
	{
		assert(func);

		ActiveSet active, inactive;

		initAndSortIntervals();
		for (auto &interval : intervals) {
			expiredOldIntervals(interval.beginNumber(), active, inactive);
			if (!tryToAllocateRegister(interval, active, inactive)) {
				allocateNewRegister(interval);
			}
			active.insert(&interval);
			V2M.insert({ interval.reg, interval.assignedReg });
		}

		assignRegister(func);
	}

	void SimpleRegisterAllocation::initAndSortIntervals()
	{
		intervals.sort([](const LiveInterval &LHS,
			const LiveInterval &RHS) {
			return LHS.beginNumber() < RHS.beginNumber();
		});
	}

	void SimpleRegisterAllocation::assignRegister(IRFunction * func)
	{
		for (auto *block : *func) {
			for (auto instr = block->instr_begin();
				instr != block->instr_end();
				++instr) {
				Instruction *I = *instr;
				if (I->is_output()) {
					assert(V2M.count(I));
					I->setOutputReg(V2M[I]);
				}

				I->op_map([this, I](Value *val) {
					if (!val->is_instr())
						return;
					assert(V2M.count(val));
					I->pushInputReg(V2M[val]);
				});
			}
		}
	}

	void SimpleRegisterAllocation::allocateNewRegister(
		LiveInterval & interval)
	{
		interval.assignedReg = totalReg;
		if (++totalReg >= maxRegs) {
			// TODO:
			assert(0);
		}
	}

	void SimpleRegisterAllocation::expiredOldIntervals(
		unsigned current, ActiveSet & active, ActiveSet &inactive)
	{
		ActiveSet newActive;
		ActiveSet newInactive;
		for (auto *I : active) {
			if (I->endNumber() <= current)
				continue;
			else if (!I->liveAt(current))
				newInactive.insert(I);
			else
				newActive.insert(I);
		};
		for (auto *I : inactive) {
			if (I->endNumber() <= current)
				continue;
			else if (I->liveAt(current))
				newActive.insert(I);
			else
				newInactive.insert(I);
		}
		std::swap(newActive, active);
		std::swap(newInactive, inactive);
	}

	bool SimpleRegisterAllocation::tryToAllocateRegister(
		LiveInterval & interval, ActiveSet & active, ActiveSet & inactive)
	{
		std::vector<unsigned> freePos(totalReg, INT_MAX);

		for (auto *I : active)
			freePos[I->getAssignReg()] = 0;

		for (auto *I : inactive) {
			if (I->overlaps(interval)) {
				auto iter = I->advanceTo(
					I->begin(), interval.beginNumber());
				freePos[I->getAssignReg()] = std::min(
					iter->start, freePos[I->getAssignReg()]);
			}
		}

		auto reg = std::max_element(freePos.begin(), freePos.end());
		if (reg != freePos.end() 
			&& *reg > interval.endNumber()) {
			// register available for whole current.
			interval.assignedReg = reg - freePos.begin();
			return true;
		}
		else {
			// allocation failed, no register available without spilling,
			// or register available for first part of current.
			return false;
		}
	}
}
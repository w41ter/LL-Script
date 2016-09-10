#pragma once

#include <list>
#include <map>
#include <set>
#include <vector>
#include <queue>

#include "LiveInterval.h"
#include "Pass.h"

namespace script
{
    class Value;
    class BasicBlock;
	class Instruction;
	class MachineRegister;

	class MoveResolve
	{
	public:
		void addMapping(LiveInterval *from, LiveInterval *to) {}
		void findInsertPosition(BasicBlock *from, BasicBlock *to) {}
		void resolverMappings() {}
	};

    class RegisterAllocator : public FunctionPass
    {
    public:
        RegisterAllocator(unsigned regNums, std::list<LiveInterval> &list);
        virtual ~RegisterAllocator() = default;

        void runOnFunction(IRFunction *func);

    private:
		struct IntervalEndCmp {
			bool operator() (const LiveInterval *lhs, 
				const LiveInterval *rhs) const {
				return lhs->endNumber() < rhs->endNumber();
			}
		};
		struct IntervalStartCmp {
			bool operator() (const LiveInterval *lhs,
				const LiveInterval *rhs) const {
				return lhs->beginNumber() > rhs->beginNumber();
			}
		};
		typedef std::set<LiveInterval*, IntervalEndCmp> ActiveSet;
		typedef std::set<LiveInterval*, IntervalStartCmp> UnhandleSet;

		void linearScanAllocate(IRFunction *func);
		void assignRegNum(IRFunction *func);
		void initAndSortIntervals();
		void resolveDataFlow(IRFunction *func);
		bool tryAllocateFreeReg(LiveInterval &interval);
		void allocateBlockedReg(LiveInterval &interval);
		void expiredOldIntervals(LiveInterval &interval);
		void splitLiveIntervalAt(LiveInterval &interval, unsigned op);
		void splitLiveIntervalBefore(LiveInterval &interval, unsigned op);
	
	private:
        const unsigned RegisterNums;

        std::list<LiveInterval> &intervals;
		std::map<Value*, LiveInterval*> V2L;
		ActiveSet active;
		ActiveSet inactive;
		UnhandleSet unhandleSet;
	};
}
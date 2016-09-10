#pragma once

#include <set>
#include <map>
#include <list>
#include <vector>
#include "Pass.h"

namespace script
{
	class Value;
	struct LiveInterval;
	struct IntervalEndCmp;
	typedef std::set<LiveInterval*, IntervalEndCmp> ActiveSet; 

	class SimpleRegisterAllocation : FunctionPass
	{
	public:
		SimpleRegisterAllocation(
			size_t regNums, std::list<LiveInterval> &I);

		void runOnFunction(IRFunction *func);

		size_t totalRegister() const { return totalReg; }

	private:
		void initAndSortIntervals();
		void assignRegister(IRFunction *func);
		void allocateNewRegister(LiveInterval &interval);
		void expiredOldIntervals(
			unsigned current, 
			ActiveSet &active, 
			ActiveSet &inactive);
		bool tryToAllocateRegister(
			LiveInterval &interval,
			ActiveSet &active,
			ActiveSet &inactive);

		const size_t maxRegs;
		size_t totalReg;
		std::vector<int> registers;
		std::list<LiveInterval> &intervals;
		std::map<Value*, unsigned> V2M;
	};
}
#pragma once

#include <list>
#include <set>
#include <map>

#include "LiveInterval.h"
#include "Pass.h"

namespace script
{
    class Value;
    class BasicBlock;

    class LiveIntervalAnalysis : public FunctionPass
    {
    public:
        virtual ~LiveIntervalAnalysis() = default;

        virtual void runOnFunction(IRFunction *func);
        
        void swapIntervals(std::list<LiveInterval> &to);

    private:
		void buildIntervals(IRFunction *func);
		void computeLocalLiveSet(IRFunction *func);
		void computeGlobalLiveSet(IRFunction *func);

		LiveInterval *getInterval(Value *val);

        std::list<LiveInterval>         intervals;
        std::map<Value*, LiveInterval*> val2inter;
    };
}
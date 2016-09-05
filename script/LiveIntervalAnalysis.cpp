#include "LiveIntervalAnalysis.h"

#include <cassert>
#include <iostream>

#include "CFG.h"
#include "Instruction.h"
#include "IRModule.h"

namespace script
{
namespace {
    static inline void operator += (
        std::set<Value*> &lhs,
        std::set<Value*> &rhs)
    {
        for (auto *V : rhs)
            lhs.insert(V);
    }

#ifdef _DEBUG
    static void dumpValue(Value *value)
    {
        if (value == nullptr)
            return;
        switch (value->get_subclass_id())
        {
        case Value::UndefVal:
        {
            std::cout << "[[Undef]]";
            break;
        }
        case Value::ConstantVal:
        {
            std::cout << "[[Constant]]";;
            break;
        }
        case Value::TableVal:
        {
            std::cout << "[[Table]]";
            break;
        }
        case Value::FunctionVal:
        {
            std::cout << "[Function]:" << ((Function*)value)->getFuncName();
            break;
        }
        case Value::ParamVal:
        {
            std::cout << "[Param]:" << ((Param*)value)->getParamName();
            break;
        }
        case Value::InstructionVal:
            std::cout << ((Instruction*)value)->get_value_name();
            break;
            //dump((Instruction*)value);
        }
    }
#endif // _DEBUG
}

    void LiveIntervalAnalysis::runOnFunction(IRFunction *func)
    {
		computeLocalLiveSet(func);
		computeGlobalLiveSet(func);

        buildIntervals(func);

#ifdef _DEBUG
		std::cout << "Begin dump intervals of function: "
			<< func->getFunctionName() << std::endl;
		for (auto &interval : intervals) {
			std::cout << "    ";
			dumpValue(interval.reg);
			std::cout << "  [" << interval.beginNumber()
				<< ", " << interval.endNumber() << ")\n";
		}
#endif // _DEBUG

    }

	LiveInterval *LiveIntervalAnalysis::getInterval(Value *val)
    {
        auto res = val2inter.find(val);
        if (res == val2inter.end()) 
        {
            intervals.push_back({ val });
			LiveInterval *I = &intervals.back();
			val2inter.insert({ val, I });
            return I;
        }
        return res->second;
    }

    void LiveIntervalAnalysis::buildIntervals(IRFunction *func) 
    {
        // The iteration is in reverse order so that all uses of 
        // a virtual register are seen before its deﬁnition.
		for (auto block = func->rbegin(), e = func->rend();
			block != e; ++block) {
			BasicBlock *BB = *block;
			unsigned from = BB->getStart();
			unsigned to = BB->getEnd();

			for (auto *val : BB->liveOut_) {
				getInterval(val)->addRange({ from, to });
			}
			
			for (auto instr = BB->instr_rbegin();
				instr != BB->instr_rend();
				++instr) {
				Instruction *I = *instr;

				if (I->is_output()) {
					LiveInterval *interval = getInterval(I);
					interval->setStart(I->getID());
					interval->addUsePosition(I->getID());
				}

				for (auto op = I->op_begin();
					op != I->op_end();
					++op) {
					Value *val = op->get_value();
					if (val->is_value())
						continue;

					LiveInterval *interval = getInterval(I);
					Instruction *OPI = static_cast<Instruction*>(val);
					interval->addRange({ from, OPI->getID() });
					interval->addUsePosition(OPI->getID());
				}
			}
        }
    }

    void LiveIntervalAnalysis::swapIntervals(
		std::list<LiveInterval> &to)
    {
        val2inter.clear();
        std::swap(this->intervals, to);
    }

	void LiveIntervalAnalysis::computeLocalLiveSet(IRFunction *func)
	{
		for (auto block = func->begin(), e = func->end();
			block != e; ++block) {
			BasicBlock *BB = *block;
			BB->liveGen_.clear();
			BB->liveKill_.clear();
			for (auto instr = BB->instr_begin();
				instr != BB->instr_end();
				++instr) {
				Instruction *I = *instr;
				if (I->is_output())
					BB->liveKill_.insert(I);
				for (auto op = I->op_begin();
					op != I->op_end();
					++op) {
					BB->liveGen_.insert(op->get_value());
				}
			}
		}
	}

	void LiveIntervalAnalysis::computeGlobalLiveSet(IRFunction * func)
	{
		bool isChange = false;
		do {
			isChange = false;
			for (auto block = func->rbegin(), e = func->rend();
				block != e; ++block) {
				BasicBlock *BB = *block;
				BB->liveOut_.clear();
				for (auto *succ : BB->successors_) {
					size_t before = BB->liveOut_.size();
					BB->liveOut_ += succ->liveIn_;
					if (before != BB->liveOut_.size())
						isChange = true;
				}
				size_t before = BB->liveIn_.size();
				BB->liveIn_ = BB->liveGen_;
				for (auto *val : BB->liveOut_) {
					if (!BB->liveKill_.count(val))
						BB->liveIn_.insert(val);
				}
				if (before != BB->liveIn_.size())
					isChange = true;
			}
		} while (isChange);
	}
}
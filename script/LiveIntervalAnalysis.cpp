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
        case Value::UserClosureVal:
        {
            std::cout << "[function]:" <<
				((UserClosure*)value)->getClosureName();
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
		func->computeBlockOrder();
		computeLocalLiveSet(func);
		computeGlobalLiveSet(func);

#ifdef _DEBUG
		std::cout << "Live analysis result:" << std::endl;
		for (auto block = func->begin(), e = func->end();
			block != e; ++block) {
			BasicBlock *BB = *block;
			std::cout << " " << BB->name_
				<< "(" << BB->start_ 
				<< ", " << BB->end_
				<< "): " << std::endl;
			std::cout << "  LiveIn = { ";
			for (auto *val : BB->liveIn_) {
				dumpValue(val);
				std::cout << ", ";
			}
			std::cout << "}\n  LiveOut = { ";
			for (auto *val : BB->liveOut_) {
				dumpValue(val);
				std::cout << ", ";
			}
			std::cout << "}\n  LiveGen = { ";
			for (auto *val : BB->liveGen_) {
				dumpValue(val);
				std::cout << ", ";
			}
			std::cout << "}\n  LiveKill = { ";
			for (auto *val : BB->liveKill_) {
				dumpValue(val);
				std::cout << ", ";
			}
			std::cout << "}\n" << std::endl;
		}
#endif // _DEBUG

        buildIntervals(func);

#ifdef _DEBUG
		std::cout << "Begin dump intervals of function: "
			<< func->getFunctionName() << std::endl;
		for (auto &interval : intervals) {
			std::cout << "    ";
			dumpValue(interval.reg);
			for (auto &range : interval.ranges) {
				std::cout << " [" << range.start
					<< ", " << range.end << ") ";
			}
			std::cout << std::endl;
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
					//if (interval->empty())
					//	interval->addRange({ I->getID(), to });
					interval->setStart(I->getID());
					interval->addUsePosition(I->getID());
				}

				for (auto op = I->op_begin();
					op != I->op_end();
					++op) {
					Value *val = op->get_value();
					if (val->is_value())
						continue;

					LiveInterval *interval = getInterval(val);
					interval->addRange({ from, I->getID() + 2 });
					interval->addUsePosition(I->getID());
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
					Value *val = op->get_value();
					if (val->is_value())
						break;
					if (!BB->liveKill_.count(val))
						BB->liveGen_.insert(val);
					//if (!I->is_phi_node()) {
					//	if (!BB->liveKill_.count(val))
					//		BB->liveGen_.insert(val);
					//}
					//else {
					//	// FIXME: B1->B2->B3
					//	// B1:
					//	//		a <- 1
					//	//		goto B2
					//	// B2:
					//	//		...
					//	//		goto B3
					//	// B3:
					//	//		a.1 = phi<a, b>
					//	Instruction *phi = static_cast<Instruction*>(val);
					//	phi->get_parent()->liveOut_.insert(val);
					//}
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
				size_t before = BB->liveOut_.size();
				BB->liveOut_.clear();	 
				for (auto *succ : BB->successors_) {
					BB->liveOut_ += succ->liveIn_;
				}
				if (before != BB->liveOut_.size())
					isChange = true;
				
				before = BB->liveIn_.size();
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
#include "Mem2Reg.h"

#include "../IR/IRModule.h"
#include "../IR/Instruction.h"
#include <vector>

namespace script
{
    struct AllocaInfo 
    {
        std::vector<BasicBlock *> DefiningBlocks;
        std::vector<BasicBlock *> UsingBlocks;

        ir::Store *OnlyStore;
        BasicBlock *OnlyBlock;
        bool OnlyUsedInOneBlock;

        ir::Value *AllocaPointerVal;

        void clear() {
            DefiningBlocks.clear();
            UsingBlocks.clear();
            OnlyStore = nullptr;
            OnlyBlock = nullptr;
            OnlyUsedInOneBlock = true;
            AllocaPointerVal = nullptr;
        }

        /// Scan the uses of the specified alloca, filling in the AllocaInfo used
        /// by the rest of the pass to reason about the uses of this alloca.
        void AnalyzeAlloca(ir::Alloca *AI) {
            clear();

            // As we scan the uses of the alloca instruction, keep track of stores,
            // and decide whether all of the loads and stores to the alloca are within
            // the same basic block.
            for (auto UI = AI->use_begin(), E = AI->use_end(); UI != E;) {
                ir::Instruction *User = (ir::Instruction*)UI++->getUser();

                if (User->instance() == ir::Instructions::IR_Store)
                {
                    ir::Store *SI = (ir::Store*)User;
                    // Remember the basic blocks which define new values for the alloca
                    DefiningBlocks.push_back(SI->getParent());
                    AllocaPointerVal = SI->getOperand(0).getValue();
                    OnlyStore = SI;
                }
                else
                {
                    ir::Load *LI = (ir::Load*)User;
                    // Otherwise it must be a load instruction, keep track of variable
                    // reads.
                    UsingBlocks.push_back(LI->getParent());
                    AllocaPointerVal = LI;
                }

                if (OnlyUsedInOneBlock) 
                {
                    if (!OnlyBlock)
                        OnlyBlock = User->getParent();
                    else if (OnlyBlock != User->getParent())
                        OnlyUsedInOneBlock = false;
                }
            }
        }
    };

    class PromoteMem2Reg
    {
    public:
        PromoteMem2Reg(std::vector<ir::Alloca*> &allocas)
            : allocas_(allocas) {}

        void run();
    protected:
        std::vector<ir::Alloca *> &allocas_;
    };
    
    Mem2Reg::Mem2Reg()
    {
    }


    Mem2Reg::~Mem2Reg()
    {
    }

    void Mem2Reg::runOnFunction(IRFunction *func)
    {
        // 遍历基本块，找到分配信息
        std::vector<ir::Alloca*> allocas;
        BasicBlock *allocaBlock = func->getEntryBlock();
        while (true)
        {
            auto *instr = allocaBlock->begin();
            if (instr == nullptr)
                break;
            if (instr->instance() == ir::Instructions::IR_Alloca)
                allocas.push_back((ir::Alloca*)instr);
            while (instr != allocaBlock->end())
            {
                instr = instr->next();
                if (instr->instance() == ir::Instructions::IR_Alloca)
                    allocas.push_back((ir::Alloca*)instr);
            }

            PromoteMem2Reg(allocas).run();
        }
    }

    void PromoteMem2Reg::run()
    {
        AllocaInfo info;
    }
}

#include "UnreachableBlockElimination.h"

#include <cassert>
#include <vector>

#include "CFG.h"
#include "Instruction.h"
#include "IRModule.h"

namespace script
{
    void UnreachableBlockElimination::runOnFunction(IRFunction *func) 
    {
        assert(func);
        reachable.clear();

        visit(func->getEntryBlock());
        eraseUnreachableCode(func);
    }

    // deep first.
    void UnreachableBlockElimination::visit(BasicBlock *block) 
    {
        assert(block);

        // is visited!
        if (reachable.count(block))
            return;
        reachable.insert(block);

        for (auto succ = block->successor_begin(), 
            end = block->successor_end();
            succ != end; ++succ) {
            visit(*succ);
        }
    }

    void UnreachableBlockElimination::eraseUnreachableCode(IRFunction *func)
    {
        std::vector<BasicBlock*> deadBlocks;
        for (auto *block : *func) {
            if (!reachable.count(block)) {
                deadBlocks.push_back(block);
            }
        }

        for (auto *block : deadBlocks) {
            func->erase(block);
        }
    }
}
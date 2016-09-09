#include "DeadCodeElimination.h"

#include "CFG.h"
#include <cassert>

namespace script
{
    void DeadCodeElimination::runOnFunction(IRFunction *func) 
    {
        assert(func);


    }

    void DeadCodeElimination::visit(BasicBlock *block)
    {

    }

    void DeadCodeElimination::eraseUnreachableCode() 
    {

    }
}
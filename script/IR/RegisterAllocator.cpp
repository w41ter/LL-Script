#include "RegisterAllocator.h"

namespace script
{
    struct UseDefine {
        int define;
        int use;
    };

    RegisterAllocator::RegisterAllocator(BasicBlock * block)
    {
        Quad *begin = block->begin(), *end = block->end();
        int index = 0;
        while (begin != nullptr)
        {
            index++;
            begin->accept(this);
            // fill use and define
            if (begin == end) break;
            begin = begin->next_;
        }
    }

    Register RegisterAllocator::allocate(Temp * temp)
    {
        // for each register if no use insert push 
        return;
    }

    int RegisterAllocator::getReg(Temp * temp)
    {
        // if current temp not in any register, allocate new register and pop it.
        return 0;
    }
}


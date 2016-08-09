#pragma once

#include "Optimize.h"

namespace script
{
    class Mem2Reg : public FunctionPass
    {
    public:
        Mem2Reg();
        virtual ~Mem2Reg();

        virtual void runOnFunction(IRFunction *func);
    };
}


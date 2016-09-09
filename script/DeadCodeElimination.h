#pragma once 

#include "Pass.h"
#include <set>

namespace script
{
    class BasicBlock;
    
    class DeadCodeElimination : public FunctionPass
    {
    public:
        virtual ~DeadCodeElimination() = default;

        void runOnFunction(IRFunction *func);

    private:
        void visit(BasicBlock *block);
        void eraseUnreachableCode();
    };
}
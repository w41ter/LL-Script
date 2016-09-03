#pragma once 

#include <set>

#include "Pass.h"

namespace script
{
    class BasicBlock;

    class UnreachableBlockElimination : public FunctionPass
    {
    public:
        virtual ~UnreachableBlockElimination() = default;

        void runOnFunction(IRFunction *func);

    private:
        void visit(BasicBlock *block);
        void eraseUnreachableCode(IRFunction *func);

        std::set<BasicBlock*> reachable;
    };
}
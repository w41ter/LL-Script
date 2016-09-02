#pragma once

#include <cassert>

namespace script
{
    class IRFunction;

    class Pass
    {
    public:
        virtual ~Pass() = default;
    };

    class FunctionPass : public Pass
    {
    public:
        FunctionPass() : becalled(false) {}
        virtual ~FunctionPass() = default;

        virtual void runOnFunction(IRFunction *func) = 0
        {
            assert(func && "Must point a IRFunction!");
            assert(!becalled && " Each function pass can be used only once!");
        }
    protected:
        bool becalled;
    };
}
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

		virtual void runOnFunction(IRFunction *func) = 0;
    protected:
        bool becalled;
    };
}
#pragma once

namespace script
{
    class IRFunction;

    class Optimize
    {
    public:
        virtual ~Optimize() = default;
    };

    class FunctionPass : public Optimize
    {
    public:
        virtual ~FunctionPass() = default;

        virtual void runOnFunction(IRFunction *func) = 0;
    };
}
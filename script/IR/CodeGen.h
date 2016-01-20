#ifndef __CODE_GEN_H__
#define __CODE_GEN_H__

#include "IRGenerator.h"
#include "../Runtime/opcode.h"

namespace script
{
    class CodeGenerator
    {
    public:
        CodeGenerator(OpcodeContext &context) : context_(context) {}

        void gen(IRModule &module);

    private:
        OpcodeContext &context_;
    };
}

#endif // !__CODE_GEN_H__


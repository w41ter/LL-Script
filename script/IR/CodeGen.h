#ifndef __CODE_GEN_H__
#define __CODE_GEN_H__

#include <map>

#include "IRGenerator.h"
#include "../Runtime/opcode.h"

namespace script
{
    class CodeGenerator
    {
    public:
        CodeGenerator(OpcodeContext &context);

        void gen(IRModule &module);

    private:
        int backLabel(Quad *label);
        int backFunction(std::string &str);

        int genIRCode(IRCode &code);
        int genBasicBlock(BasicBlock *block);
    private:
        OpcodeContext &context_;

        std::map<std::string, int> functions_;
        std::map<Quad*, int> labels_;
    };
}

#endif // !__CODE_GEN_H__


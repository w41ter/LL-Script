#include <functional>

#include "CodeGen.h"

namespace script
{
    CodeGenerator::CodeGenerator(OpcodeContext & context)
        : context_(context)
    {
        context_.bind(std::bind(&CodeGenerator::backLabel, this, std::placeholders::_1));
        context_.bindFunction(std::bind(&CodeGenerator::backFunction, this, std::placeholders::_1));
    }

    void CodeGenerator::gen(IRModule & module)
    {
        for (auto &i : module.functions_)
        {
            int offset = genIRCode(*i.second);
            functions_.insert(std::pair<std::string, int>(i.first, offset));
        }
        genIRCode(module);
    }

    int CodeGenerator::genIRCode(IRCode & code)
    {
        
    }

}

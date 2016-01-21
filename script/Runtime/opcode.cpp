#include "opcode.h"

namespace script
{
    Pointer * OpcodeContext::getOpcodes()
    {
        if (opcodes_ == nullptr)
        {
            opcodes_ = new Pointer[codeList_.size()];
            int length = 0;
            for (auto i : codeList_)
                opcodes_[length++] = i;
        }
        return opcodes_;
    }

    size_t OpcodeContext::opcodeLength()
    {
        return codeList_.size();
    }

    int OpcodeContext::insertString(std::string & str)
    {
        if (stringPool_.count(str) == 0)
        {
            int index = stringNum_++;
            stringPool_.insert(std::pair<std::string, int>(str, index));
            return index;
        }
        return stringPool_[str];
    }
}


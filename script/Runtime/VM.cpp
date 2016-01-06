#include "VM.h"

namespace script
{
    bool IsCalcu(Pointer p)
    {
        return IsFixnum(p) || IsReal(p);
    }

    void VM::excute()
    {
        while (true)
        {
            Code op = opcode_[ip_++];
            Pointer *value = sizeOfOpcode(op) == 2 ?
                (Pointer*)&opcode_[ip_++] : nullptr;

            switch (op)
            {
            case ADD:
                Pointer left = pop(), right = pop();
                if (!IsCalcu(left) || !IsCalcu(right))
                    throw std::runtime_error("不能进行运算");

                break;
            case SUB:
                break;
            case MUL:
                break;
            case DIV:
                break;
            case ICONST:
                int num = *value;
                operandStack_[++osp_] = MakeFixnum(num);
                break;
            case CCONST:
                char c = *value;
                operandStack_[++osp_] = SetChar(c);
                break;
            case SCONST:
                size_t index = *value;
                std::string &str = globalString_[index];
                Pointer addr = gc.allocate(sizeof(String) + str.length());
                operandStack_[++osp_] = MakeString(addr, str.c_str(), str.length());
                break;
            case RCONST:
                float fnum = *value;
                operandStack_[++osp_] = MakeReal(fnum);
                break;
            case CLOSURE:
                size_t length = *value;
                Pointer closure = gc.allocate(sizeof(String) + length);
                MakeClosure(closure, length);
                Pointer *p = ClosureParams(closure);
                while (psp_ > 0)
                {
                    auto param = paramsStack_[psp_--];
                    p[psp_] = param;
                }
                operandStack_[++osp_] = closure;
                break;
            case NEW_ARRAY:
                break;
            case LOAD_LOCAL:
                break;
            case SET_LOCAL:
                break;
            case PARAM:
                paramsStack_[++psp_] = *value;
                break;
            case CALL:
                ipStack_[++isp_] = ip_;
                // 填充参数结构
                break;
            case RET:
                operandStack_[++osp_] = *value;
                ip_ = ipStack_[isp_--];
                break;
            case HALT:
                return;
                break;
            }
        }
    }
}
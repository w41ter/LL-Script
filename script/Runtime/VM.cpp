#include "VM.h"

namespace script
{
    bool IsCalcu(Pointer p)
    {
        return IsFixnum(p) || IsReal(p);
    }

    void VM::callClosure(Pointer closure, size_t length)
    {
        if (!IsClosure(closure)) {
            // error
            return;
        }
        size_t need = ClosureNeed(closure), has = ClosureLength(closure);
        if (has + length > need) {
            // error
            return;
        }

        size_t position = ClosurePosition(closure);
        // closure
        if (has + length < need)
        {
            Pointer newClosure = gc.allocate(has + length);
            MakeClosure(newClosure, position, has + length, need);
            Pointer *newParams = ClosureParams(newClosure);
            Pointer *params = ClosureParams(closure);
            for (size_t i = 0; i < has; ++i)
                newParams[i] = params[i];
            for (size_t i = length; i >= 0; --i)
            {
                newParams[has + i] = paramsStack_.top();
                paramsStack_.pop();
            }
            operandStack_.push(newClosure);
        }
        else    // function call
        {
            localStack_.push(local_);
            local_ = new Pointer[];
            Pointer *params = ClosureParams(closure);
            for (size_t i = 0; i < has; ++i)
                local_[i] = params[i];
            for (size_t i = 0; i < length; ++i)
            {
                local_[has + i] = paramsStack_.top();
                paramsStack_.pop();
            }
            ipStack_.push(ip_);
            ip_ = position;
        }
    }

    void VM::excute()
    {
        while (true)
        {
            Code op = opcode_[ip_++];

            switch (op)
            {
            case CALL:
            {
                size_t index = opcode_[ip_++];
                size_t length = opcode_[ip_++];
                Pointer closure = local_[index];
                callClosure(closure, length);
                break;
            }

            case CALL_GLOBAL:
            {
                size_t index = opcode_[ip_++];
                size_t length = opcode_[ip_++];
                Pointer closure = global_[index];
                callClosure(closure, length);
                break;
            }

            case ADD: case SUB: case MUL: case DIV:
            {
                Pointer left = operandStack_.top(); 
                operandStack_.pop();
                Pointer right = operandStack_.top();
                operandStack_.pop();
                calculate(op, left, right);
                break;
            } 

            case GREAT: case GREAT_THAN: case LESS:
            case LESS_THAN: case NOT_EQUAL: case EQUAL:
            {
                Pointer left = operandStack_.top();
                operandStack_.pop();
                Pointer right = operandStack_.top();
                operandStack_.pop();
                compare(op, left, right);
                break;
            }

            case ICONST:
            {
                int integer = (int)opcode_[ip_++];
                operandStack_.push(MakeFixnum(integer));
                break;
            }

            case CCONST:
            {
                char c = (char)opcode_[ip_++];
                operandStack_.push(SetChar(c));
                break;
            }

            case SCONST:
            {
                size_t index = opcode_[ip_++];
                const std::string &strs = globalString_[index];
                Pointer str = gc.allocate(sizeof(String) + strs.size());
                MakeString(str, strs.c_str(), strs.size());
                operandStack_.push(str);
                break;
            }

            case RCONST:
            {
                float val = opcode_[ip_++];
                operandStack_.push(MakeReal(val));
                break;
            }

            case NEW_ARRAY:
                break;

            case LOAD_LOCAL:
            {
                size_t index = opcode_[ip_++];
                operandStack_.push(local_[index]);
                break;
            }

            case SET_LOCAL:
            {
                size_t index = opcode_[ip_++];
                local_[index] = operandStack_.top();
                operandStack_.pop();
                break;
            }

            case LOAD_GLOBAL:
            {
                size_t index = opcode_[ip_++];
                operandStack_.push(global_[index]);
                break;
            }

            case SET_GLOBAL:
            {
                size_t index = opcode_[ip_++];
                global_[index] = operandStack_.top();
                operandStack_.pop();
                break;
            }

            case PARAM:
            {
                Pointer val = (Pointer)opcode_[ip_++];
                paramsStack_.push(val);
                break;
            }
            case RET:
            {
                delete[]local_;
                local_ = localStack_.top();
                localStack_.pop();
                break;
            }
            case HALT:
                return;
                break;
            }
        }
    }
}
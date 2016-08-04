#include "VM.h"

#include <string>
#include <vector>
#include <functional>
#include <cassert>

#include "../BuildIn/BuildIn.h"

using std::vector;
using std::string;

using namespace buildin;

namespace script
{
    VirtualMachine::VirtualMachine() : gc_(1024 * 1024)
    {
        gc_.bindGlobals(std::bind(&VirtualMachine::processGlobals, this));
        gc_.bindReference(std::bind(&VirtualMachine::variableReference, this, std::placeholders::_1));
    }

    void VirtualMachine::excute(Byte * opcode, size_t length)
    {
        opcodes_ = opcode + 8;
        length_ = length;

        int32_t *temp = (int32_t *)opcode, offsetOfEntry = *temp++;
        opcodeLength_ = *temp;
        size_t ip = offsetOfEntry;

        loadStringPool();

        bool halt = false;
        while (!halt)
        {
            switch (opcodes_[ip++])
            {
            case OK_Add:
            case OK_Sub:
            case OK_Div:
            case OK_Mul:
            case OK_Great:
            case OK_GreatThan:
            case OK_Less:
            case OK_LessThan:
            case OK_NotEqual:
            case OK_Equal:
                excuteBinary(ip, opcodes_[ip-1]);
                break;
            case OK_Negtive:
            case OK_Not:
                excuteSingle(ip, opcodes_[ip-1]);
                break;
            case OK_Call:
                excuteCall(ip);
                break;
            case OK_Invoke:
                excuteInvoke(ip);
                break;
            case OK_BuildIn:
                excuteBuildIn(ip);
                break;
            case OK_Goto:
                excuteGoto(ip);
                break;
            case OK_If:
                excuteIf(ip);
                break;
            case OK_IfFalse:
                excuteIfFalse(ip);
                break;
            case OK_Return:
                excuteReturn(ip);
                break;
            case OK_Load:
                excuteLoad(ip);
                break;
            case OK_LoadA:
                excuteLoadA(ip);
                break;
            case OK_Store:
                excuteStore(ip);
                break;
            case OK_StoreA:
                excuteStoreA(ip);
                break;
            case OK_Move:
                excuteMove(ip);
                break;
            case OK_MoveF:
                excuteMoveF(ip);
                break;
            case OK_MoveI:
                excuteMoveI(ip);
                break;
            case OK_MoveS:
                excuteMoveS(ip);
                break;
            case OK_Halt:
                halt = true;
                break;
            case OK_Param:
                excuteParam(ip);
                break;
            case OK_PushR:
                excutePushR(ip);
                break;
            case OK_PopR:
                excutePopR(ip);
                break;
            case OK_Entry:
                excuteEntry(ip);
                break;
            case OK_NewSlot:
                excuteNewSlot(ip);
                break;
            case OK_NewArray:
                excuteNewArray(ip);
                break;
            }
        }

        return;
    }

    GarbageCollector * VirtualMachine::getGC()
    {
        return &gc_;
    }

    void VirtualMachine::excuteBinary(size_t & ip, unsigned op)
    {
        unsigned result = opcodes_[ip++];
        Pointer left = getRegister(opcodes_[ip++]), right = getRegister(opcodes_[ip++]);
        //std::function<Pointer(Pointer, Pointer)> callable;
        Pointer calRes = 0;
        switch (op)
        {
        case OK_Add: calRes = add(left, right); break;
        case OK_Sub: calRes = sub(left, right); break;
        case OK_Div: calRes = div(left, right); break;
        case OK_Mul: calRes = mul(left, right); break;
        case OK_Great: calRes = g(left, right); break;
        case OK_GreatThan: calRes = gt(left, right); break;
        case OK_Less: calRes = l(left, right); break;
        case OK_LessThan: calRes = lt(left, right); break;
        case OK_NotEqual: calRes = ne(left, right); break;
        case OK_Equal: calRes = et(left, right); break;
        }
        setRegister(result, calRes);
    }

    void VirtualMachine::excuteSingle(size_t & ip, unsigned op)
    {
        unsigned result = opcodes_[ip++];
        switch (op)
        {
        case OK_Not: setRegister(result, not(getRegister(opcodes_[ip++]))); break;
        case OK_Negtive: setRegister(result, negative(getRegister(opcodes_[ip++]))); break;
        }
    }

    void VirtualMachine::excuteCall(size_t & ip)
    {
        unsigned result = opcodes_[ip++];
        int params = opcodes_[ip++], total = opcodes_[ip++], offset = getInteger(ip);
        Pointer value = gc_.allocate(CLOSURE_SIZE(total));
        MakeClosure(value, offset, params, total);
        setRegister(result, value);
    }

    void VirtualMachine::excuteInvoke(size_t & ip)
    {
        unsigned result = opcodes_[ip++];
        int num = (int)opcodes_[ip++];
        Pointer value = getRegister(opcodes_[ip++]);
        assert(IsClosure(value) || IsBuildInClosure(value));

        if (IsClosure(value))
            excuteClosure(result, num, value, ip);
        else
            excuteBuildInClosure(result, num, value, ip);
    }

    void VirtualMachine::excuteBuildIn(size_t & ip)
    {
        unsigned result = opcodes_[ip++];
        int params = opcodes_[ip++], total = opcodes_[ip++], offset = getInteger(ip);
        Pointer value = gc_.allocate(CLOSURE_SIZE(total));
        MakeBuildInClosure(value, offset, params, total);
        setRegister(result, value);
    }

    void VirtualMachine::excuteGoto(size_t & ip)
    {
        int offset = getInteger(ip);
        // FIXME:
        ip = offset;
    }

    void VirtualMachine::excuteIf(size_t & ip)
    {
        Pointer value = getRegister(opcodes_[ip++]);
        int offset = getInteger(ip);
        if (IsFixnum(value) && GetFixnum(value) != 0)
        {
            // FIXME: 判断这里是否需要
            ip = offset;
        }
    }

    void VirtualMachine::excuteIfFalse(size_t &ip)
    {
        Pointer value = getRegister(opcodes_[ip++]);
        int offset = getInteger(ip);
        if (IsFixnum(value) && GetFixnum(value) == 0)
        {
            // FIXME: 判断这里是否需要
            ip = offset;
        }
    }

    void VirtualMachine::excuteReturn(size_t & ip)
    {
        Pointer value = getRegister(opcodes_[ip++]);
        Frame *temp = currentFrame_;
        currentFrame_ = currentFrame_->previous_;
        setRegister(temp->result_, value);
        ip = temp->ip_;
        delete temp;
        //frameStack_.pop();
    }

    void VirtualMachine::excuteLoad(size_t &ip)
    {
        unsigned reg = opcodes_[ip++];
        setRegister(reg, currentFrame_->localSlot_[getInteger(ip)]);
    }

    void VirtualMachine::excuteLoadA(size_t &ip)
    {
        unsigned regid = opcodes_[ip++], regindex = opcodes_[ip++], regresult = opcodes_[ip++];
        Pointer array = getRegister(regid), index = getRegister(regindex);
        if (!IsArray(array)) throw std::runtime_error("求值对象非数组类型！");
        RArray *value = (RArray*)array;
        if (!IsFixnum(index)) throw std::runtime_error("数组索引必须是数值类型！");
        size_t num = GetFixnum(index);
        if (value->length_ <= num || num < 0) throw std::runtime_error("数组索引 out of range!");
        setRegister(regresult, value->data[num]);
    }

    void VirtualMachine::excuteParam(size_t & ip)
    {
        paramStack_.push_back(getRegister(opcodes_[ip++]));
    }

    void VirtualMachine::excuteStore(size_t & ip)
    {
        Pointer value = getRegister(opcodes_[ip++]);
        currentFrame_->localSlot_[getInteger(ip)] = value;
    }

    void VirtualMachine::excuteStoreA(size_t &ip)
    {
        unsigned regid = opcodes_[ip++], regindex = opcodes_[ip++], regresult = opcodes_[ip++];
        Pointer array = getRegister(regid), index = getRegister(regindex), result = getRegister(regresult);
        if (!IsArray(array)) throw std::runtime_error("求值对象非数组类型！");
        RArray *value = (RArray*)array;
        if (!IsFixnum(index)) throw std::runtime_error("数组索引必须是数值类型！");
        size_t num = GetFixnum(index);
        if (value->length_ <= num || num < 0) throw std::runtime_error("数组索引 out of range!");
        value->data[num] = result;
    }

    void VirtualMachine::excuteMove(size_t &ip)
    {
        Pointer value = getRegister(opcodes_[ip++]);
        setRegister(opcodes_[ip++], value);
    }

    void VirtualMachine::excuteMoveF(size_t &ip)
    {
        unsigned reg = opcodes_[ip++];
        Pointer f = MakeReal(getFloat(ip));
        setRegister(reg, f);
    }

    void VirtualMachine::excuteMoveI(size_t &ip)
    {
        unsigned reg = opcodes_[ip++];
        Pointer integer = MakeFixnum(getInteger(ip));
        setRegister(reg, integer);
    }

    void VirtualMachine::excuteMoveS(size_t &ip)
    {
        unsigned reg = opcodes_[ip++];
        string &origin = stringPool_[getInteger(ip)];
        Pointer str = gc_.allocate(STRING_SIZE(origin.size()));
        MakeString(str, origin.c_str(), origin.size());
        setRegister(reg, str);
    }

    void VirtualMachine::excutePushR(size_t &ip)
    {
        setRegister(opcodes_[ip++], currentFrame_->registerStack_.top());
        currentFrame_->registerStack_.pop();
    }

    void VirtualMachine::excutePopR(size_t & ip)
    {
        currentFrame_->registerStack_.push(getRegister(opcodes_[ip++]));
    }

    void VirtualMachine::excuteEntry(size_t &ip)
    {
        int offset = getInteger(ip);
        currentFrame_ = globalFrame_ = new Frame(getInteger(ip));
        //frameStack_.push(std::move(Frame(getInteger(ip))));
        //currentFrame_ = globalFrame_ = &frameStack_.top();
        //FIXME
        ip = offset;
    }

    void VirtualMachine::excuteNewSlot(size_t & ip)
    {
        currentFrame_->localSlot_.resize(getInteger(ip));
        //currentFrame_->resize(getInteger(ip));
    }

    void VirtualMachine::excuteNewArray(size_t &ip)
    {
        unsigned reg = opcodes_[ip++];
        int length = getInteger(ip);
        Pointer array = gc_.allocate(ARRAY_SIZE(length));
        MakeArray(array, length);
        setRegister(reg, array);
    }

    void VirtualMachine::excuteClosure(unsigned result, int num, Pointer value, size_t &ip)
    {
        size_t need = ClosureNeed(value), length = ClosureLength(value);
        if (length + num == need)
        {
            // call

            //frameStack_.push(std::move(Frame(need, currentFrame_)));
            Frame *temp = new Frame(need, currentFrame_); //&frameStack_.top();
            temp->ip_ = ip;
            ip = ClosurePosition(value);
            Pointer *params = ClosureParams(value);
            for (size_t i = 0; i < length; ++i)
            {
                temp->localSlot_[i] = params[i];
            }
            for (size_t i = 0; i < num; ++i)
                temp->localSlot_[length + i] = paramStack_[i];
            paramStack_.clear();

            temp->result_ = result;
            currentFrame_ = temp;
        }
        else
        {
            // new 
            Pointer newValue = gc_.allocate(CLOSURE_SIZE(need));
            MakeClosure(newValue, ClosurePosition(value), length + num, need);
            setRegister(result, newValue);
        }
    }

    void VirtualMachine::excuteBuildInClosure(unsigned result, int num, Pointer value, size_t &ip)
    {
        size_t need = BuildInClosureNeed(value), length = BuildInClosureLength(value);
        if (length + num == need)
        {
            vector<Pointer> argument(need);
            Pointer *params = BuildInClosureParams(value);
            for (size_t i = 0; i < length; ++i)
                argument[i] = params[i];
            for (int i = 0; i < num; ++i)
                argument[length + i] = paramStack_[i];
            paramStack_.clear();
            int pos = BuildInClosureIndex(value);
            setRegister(result, BuildIn::getInstance().excute(pos, std::move(argument)));
        }
        else
        {
            // new 
            Pointer newValue = gc_.allocate(CLOSURE_SIZE(need));
            MakeBuildInClosure(newValue, BuildInClosureIndex(value), length + num, need);
            setRegister(result, newValue);
        }
    }

    void VirtualMachine::setRegister(unsigned reg, Pointer value)
    {
        switch (reg)
        {
        case RG_A: currentFrame_->register_.regA_ = value; break;
        case RG_B: currentFrame_->register_.regB_ = value; break;
        case RG_C: currentFrame_->register_.regC_ = value; break;
        case RG_D: currentFrame_->register_.regD_ = value; break;
        case RG_E: currentFrame_->register_.regE_ = value; break;
        case RG_F: currentFrame_->register_.regF_ = value; break;
        case RG_H: currentFrame_->register_.regH_ = value; break;
        case RG_I: currentFrame_->register_.regI_ = value; break;
        case RG_J: currentFrame_->register_.regJ_ = value; break;
        case RG_K: currentFrame_->register_.regK_ = value; break;
        case RG_L: currentFrame_->register_.regL_ = value; break;
        case RG_M: currentFrame_->register_.regM_ = value; break;
        case RG_N: currentFrame_->register_.regN_ = value; break;
        case RG_O: currentFrame_->register_.regO_ = value; break;
        case RG_P: currentFrame_->register_.regP_ = value; break;
        case RG_Q: currentFrame_->register_.regQ_ = value; break;
        }
    }

    Pointer VirtualMachine::getRegister(unsigned reg)
    {
        Pointer value;
        switch (reg)
        {
        case RG_A: value = currentFrame_->register_.regA_; break;
        case RG_B: value = currentFrame_->register_.regB_; break;
        case RG_C: value = currentFrame_->register_.regC_; break;
        case RG_D: value = currentFrame_->register_.regD_; break;
        case RG_E: value = currentFrame_->register_.regE_; break;
        case RG_F: value = currentFrame_->register_.regF_; break;
        case RG_H: value = currentFrame_->register_.regH_; break;
        case RG_I: value = currentFrame_->register_.regI_; break;
        case RG_J: value = currentFrame_->register_.regJ_; break;
        case RG_K: value = currentFrame_->register_.regK_; break;
        case RG_L: value = currentFrame_->register_.regL_; break;
        case RG_M: value = currentFrame_->register_.regM_; break;
        case RG_N: value = currentFrame_->register_.regN_; break;
        case RG_O: value = currentFrame_->register_.regO_; break;
        case RG_P: value = currentFrame_->register_.regP_; break;
        case RG_Q: value = currentFrame_->register_.regQ_; break;
        }
        return value;
    }

    void VirtualMachine::processGlobals()
    {
        Frame *temp = currentFrame_;
        while (true)
        {
            for (size_t i = 0; i < temp->localSlot_.size(); ++i)
            {
                gc_.processReference(&(temp->localSlot_[i]));
            }
            temp = temp->previous_;
            if (temp == nullptr) break;
        }
    }

    void VirtualMachine::variableReference(Pointer *pointer)
    {
        if (IsClosure(*pointer))
        {
            size_t need = ClosureNeed(*pointer);
            Pointer *params = ClosureParams(*pointer);
            for (size_t i = 0; i < need; ++i)
            {
                gc_.processReference(&(params[i]));
            }
        }
    }

    bool IsCalable(Pointer lhs)
    {
        return (IsFixnum(lhs) || IsReal(lhs));
    }

    Pointer VirtualMachine::add(Pointer lhs, Pointer rhs)
    {
        if (!IsCalable(lhs) || !IsCalable(rhs))
        {
            throw std::runtime_error("类型不匹配");
        }
        if (IsFixnum(lhs) && IsFixnum(rhs))
        {
            return MakeFixnum(GetFixnum(lhs) + GetFixnum(rhs));
        }
        else
        {
            float left = IsReal(lhs) ? GetReal(lhs) : GetFixnum(lhs);
            float right = IsReal(rhs) ? GetReal(rhs) : GetFixnum(rhs);
            return MakeReal(left + right);
        }
    }

    Pointer VirtualMachine::sub(Pointer lhs, Pointer rhs)
    {
        if (!IsCalable(lhs) || !IsCalable(rhs))
        {
            throw std::runtime_error("类型不匹配");
        }
        if (IsFixnum(lhs) && IsFixnum(rhs))
        {
            return MakeFixnum(GetFixnum(lhs) - GetFixnum(rhs));
        }
        else
        {
            float left = IsReal(lhs) ? GetReal(lhs) : GetFixnum(lhs);
            float right = IsReal(rhs) ? GetReal(rhs) : GetFixnum(rhs);
            return MakeReal(left - right);
        }
    }

    Pointer VirtualMachine::mul(Pointer lhs, Pointer rhs)
    {
        if (!IsCalable(lhs) || !IsCalable(rhs))
        {
            throw std::runtime_error("类型不匹配");
        }
        if (IsFixnum(lhs) && IsFixnum(rhs))
        {
            return MakeFixnum(GetFixnum(lhs) * GetFixnum(rhs));
        }
        else
        {
            float left = IsReal(lhs) ? GetReal(lhs) : GetFixnum(lhs);
            float right = IsReal(rhs) ? GetReal(rhs) : GetFixnum(rhs);
            return MakeReal(left * right);
        }
    }

    Pointer VirtualMachine::div(Pointer lhs, Pointer rhs)
    {
        if (!IsCalable(lhs) || !IsCalable(rhs))
        {
            throw std::runtime_error("类型不匹配");
        }
        if (IsFixnum(lhs) && IsFixnum(rhs))
        {
            int right = GetFixnum(rhs);
            return MakeFixnum(GetFixnum(lhs) / right);
        }
        else
        {
            float left = IsReal(lhs) ? GetReal(lhs) : GetFixnum(lhs);
            float right = IsReal(rhs) ? GetReal(rhs) : GetFixnum(rhs);
            if (right == 0.0f) throw std::runtime_error("div 0 error!");
            return MakeReal(left / right);
        }
    }

    Pointer VirtualMachine::g(Pointer lhs, Pointer rhs)
    {
        if (!IsFixnum(lhs) || !IsFixnum(rhs))
        {
            throw std::runtime_error("类型不匹配");
        }
        return MakeFixnum(GetFixnum(lhs) > GetFixnum(rhs));
    }

    Pointer VirtualMachine::gt(Pointer lhs, Pointer rhs)
    {
        if (!IsFixnum(lhs) || !IsFixnum(rhs))
        {
            throw std::runtime_error("类型不匹配");
        }
        return MakeFixnum(GetFixnum(lhs) >= GetFixnum(rhs));
    }

    Pointer VirtualMachine::l(Pointer lhs, Pointer rhs)
    {
        if (!IsFixnum(lhs) || !IsFixnum(rhs))
        {
            throw std::runtime_error("类型不匹配");
        }
        return MakeFixnum(GetFixnum(lhs) < GetFixnum(rhs));
    }

    Pointer VirtualMachine::lt(Pointer lhs, Pointer rhs)
    {
        if (!IsFixnum(lhs) || !IsFixnum(rhs))
        {
            throw std::runtime_error("类型不匹配");
        }
        return MakeFixnum(GetFixnum(lhs) <= GetFixnum(rhs));
    }

    Pointer VirtualMachine::et(Pointer lhs, Pointer rhs)
    {
        if (IsFixnum(lhs) && IsFixnum(rhs))
        {
            return MakeFixnum(lhs == rhs);
        }
        else if (IsReal(lhs) && IsReal(rhs))
        {
            return MakeFixnum(lhs == rhs);
        }
        else if (IsString(lhs) && IsString(rhs))
        {
            return MakeFixnum(strcmp(GetString(lhs), GetString(rhs)) == 0);
        }
        else
        {
            return MakeFixnum(lhs == rhs);
        }
    }

    Pointer VirtualMachine::ne(Pointer lhs, Pointer rhs)
    {
        if (IsFixnum(lhs) && IsFixnum(rhs))
        {
            return MakeFixnum(lhs != rhs);
        }
        else if (IsReal(lhs) && IsReal(rhs))
        {
            return MakeFixnum(lhs != rhs);
        }
        else if (IsString(lhs) && IsString(rhs))
        {
            return MakeFixnum(strcmp(GetString(lhs), GetString(rhs)));
        }
        else
        {
            return MakeFixnum(lhs != rhs);
        }
    }

    Pointer VirtualMachine::not(Pointer lhs)
    {
        if (!IsFixnum(lhs))
        {
            throw std::runtime_error("类型不匹配");
        }
        return MakeFixnum(!GetFixnum(lhs));
    }

    Pointer VirtualMachine::negative(Pointer lhs)
    {
        if (IsFixnum(lhs))
        {
            return MakeFixnum(-GetFixnum(lhs));
        }
        else if (IsReal(lhs))
        {
            return MakeReal(-GetReal(lhs));
        }
        else
        {
            throw std::runtime_error("类型不匹配");
        }
    }

    void VirtualMachine::loadStringPool()
    {
        int32_t *base = (int32_t*)&opcodes_[opcodeLength_];
        int32_t total = *base;

        int32_t *offset = base + 1, *begin = offset + total, count = 0;
        for (int i = 0; i < total; ++i)
        {
            string str;
            int length = *(offset + i);
            for (int j = 0; j < length; ++j)
            {
                str += *((char*)begin + count + j);
            }
            count += length;
            stringPool_.push_back(str);
        }
    }

    int32_t VirtualMachine::getInteger(size_t & ip)
    {
        int32_t result = 0;
        for (int i = 0; i < 4; ++i)
        {
            result <<= 8; result |= (unsigned char)opcodes_[ip++];
        }
        return result;
    }

    float VirtualMachine::getFloat(size_t & ip)
    {
        int ival = getInteger(ip), *iptr = &ival;
        double *fptr = reinterpret_cast<double*>(iptr);
        return *fptr;
    }

}
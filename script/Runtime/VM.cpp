#include "VM.h"

#include <string>
#include <vector>
#include <functional>

using std::vector;
using std::string;

namespace script
{
    VirtualMachine::VirtualMachine() : gc(1024 * 1024)
    {
        gc.bindGlobals(std::bind(&VirtualMachine::processGlobals, this));
        gc.bindReference(std::bind(&VirtualMachine::variableReference, this, std::placeholders::_1));
    }

    void VirtualMachine::excute(Byte * opcode, size_t length)
    {
        opcodes_ = opcode;
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
            }
        }

        return;
    }

    void VirtualMachine::excuteBinary(size_t & ip, unsigned op)
    {
    }

    void VirtualMachine::excuteSingle(size_t & ip, unsigned op)
    {
    }

    void VirtualMachine::excuteCall(size_t & ip)
    {
    }

    void VirtualMachine::excuteInvoke(size_t & ip)
    {
    }

    void VirtualMachine::excuteGoto(size_t & ip)
    {
        int offset = getInteger(ip);
        // FIXME:
        ip -= offset;
    }

    void VirtualMachine::excuteIf(size_t & ip)
    {
        Pointer value = getRegister(opcodes_[ip++]);
        int offset = getInteger(ip);
        if (IsFixnum(value) && GetFixnum(value) != 0)
        {
            // FIXME: 判断这里是否需要
            ip -= offset;
        }
    }

    void VirtualMachine::excuteIfFalse(size_t &ip)
    {
        Pointer value = getRegister(opcodes_[ip++]);
        int offset = getInteger(ip);
        if (IsFixnum(value) && GetFixnum(value) == 0)
        {
            // FIXME: 判断这里是否需要
            ip -= offset;
        }
    }

    void VirtualMachine::excuteReturn(size_t & ip)
    {
        result_ = getRegister(opcodes_[ip++]);
    }

    void VirtualMachine::excuteLoad(size_t &ip)
    {
        unsigned reg = opcodes_[ip++];
        setRegister(reg, localSlot[getInteger(ip)]);
    }

    void VirtualMachine::excuteLoadA(size_t &ip)
    {
        // TODO;
    }

    void VirtualMachine::excuteParam(size_t & ip)
    {
    }

    void VirtualMachine::excuteStore(size_t & ip)
    {
        unsigned reg = opcodes_[ip++];
        localSlot[getInteger(ip)] = getRegister(reg);
    }

    void VirtualMachine::excuteStoreA(size_t &ip)
    {
        unsigned id = opcodes_[ip++], index = opcodes_[ip++], result = opcodes_[ip++];
        Pointer array = getRegister(id);
        // TODO
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
        Pointer str = gc.allocate(STRING_SIZE(origin.size()));
        MakeString(str, origin.c_str(), origin.size());
        setRegister(reg, str);
    }

    void VirtualMachine::excutePushR(size_t &ip)
    {
        setRegister(opcodes_[ip++], currentStack_->top());
        currentStack_->pop();
    }

    void VirtualMachine::excutePopR(size_t & ip)
    {
        currentStack_->push(getRegister(opcodes_[ip++]));
    }

    void VirtualMachine::excuteEntry(size_t &ip)
    {
        offset_ = getInteger(ip);
        frame_ = getInteger(ip);
    }

    void VirtualMachine::setRegister(unsigned reg, Pointer value)
    {
        switch (reg)
        {
        case RG_A: regA_ = value; break;
        case RG_B: regB_ = value; break;
        case RG_C: regC_ = value; break;
        case RG_D: regD_ = value; break;
        case RG_E: regE_ = value; break;
        case RG_F: regF_ = value; break;
        case RG_H: regH_ = value; break;
        case RG_I: regI_ = value; break;
        case RG_J: regJ_ = value; break;
        case RG_K: regK_ = value; break;
        case RG_L: regL_ = value; break;
        case RG_M: regM_ = value; break;
        case RG_N: regN_ = value; break;
        case RG_O: regO_ = value; break;
        case RG_P: regP_ = value; break;
        case RG_Q: regQ_ = value; break;
        }
    }

    Pointer VirtualMachine::getRegister(unsigned reg)
    {
        Pointer value;
        switch (reg)
        {
        case RG_A: value = regA_; break;
        case RG_B: value = regB_; break;
        case RG_C: value = regC_; break;
        case RG_D: value = regD_; break;
        case RG_E: value = regE_; break;
        case RG_F: value = regF_; break;
        case RG_H: value = regH_; break;
        case RG_I: value = regI_; break;
        case RG_J: value = regJ_; break;
        case RG_K: value = regK_; break;
        case RG_L: value = regL_; break;
        case RG_M: value = regM_; break;
        case RG_N: value = regN_; break;
        case RG_O: value = regO_; break;
        case RG_P: value = regP_; break;
        case RG_Q: value = regQ_; break;
        }
        return value;
    }

    void VirtualMachine::processGlobals()
    {
    }

    void VirtualMachine::variableReference(Pointer *pointer)
    {
    }

    void VirtualMachine::loadStringPool()
    {
        int32_t *base = (int32_t*)&opcodes_[opcodeLength_ + 8];
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
            result <<= 8; result |= opcodes_[ip++];
        }
        return result;
    }

    float VirtualMachine::getFloat(size_t & ip)
    {
        return (float)getInteger(ip);
    }

}
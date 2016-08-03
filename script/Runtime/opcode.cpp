#include "opcode.h"

#include <algorithm>

#include "../BuildIn/BuildIn.h"
#include "../Parser/lexer.h"

struct CmpByValue 
{
    bool operator()(const std::pair<std::string, int>& lhs,
        const std::pair<std::string, int>& rhs) 
    {
        return lhs.second < rhs.second;
    }
};

namespace script
{
    Byte * OpcodeContext::getOpcodes(int &length)
    {
        if (opcodes_ == nullptr)
        {
            for (auto &i : functionBack_)
            {
                int index = getFunctionTarget_(i.first);
                setInteger(i.second, index);
            }
            functionBack_.clear();
            for (auto &i : labelBack_)
            {
                int index = getLabelTarget_(i.first);
                setInteger(i.second, index);
            }
            labelBack_.clear();
            for (auto &i : slotBack_)
            {
                int index = getFunctionSlotTarget_(i.first);
                setInteger(i.second, index);
            }
            // cal string pool size : opcode length + string nums + string offset + string length
            length = 8 + codeList_.size() + 4 * stringNum_ + 4;
            for (auto &i : stringPool_)
            {
                length += i.first.size();
            }

            opcodes_ = new Byte[length + 1];

            int l = 8;
            for (auto i : codeList_)
                opcodes_[l++] = i;

            int32_t *temp = (int32_t*)opcodes_;
            *temp++ = entryOffset_;
            int32_t t = codeList_.size() - 8;
            *temp = codeList_.size();

            // add string information
            int32_t *offset = (int32_t*)&opcodes_[l], *point = offset + (stringNum_ + 1);

            *offset++ = stringNum_;
            std::vector<std::pair<std::string, int>> pool(stringPool_.begin(), stringPool_.end());
            sort(pool.begin(), pool.end(), CmpByValue());
            for (auto &i : pool)
            {
                *offset++ = i.first.size(); 
                char *base = (char*)point;
                for (auto c : i.first)
                {
                    *base++ = c;
                }
                point = (int32_t*)base;
            }
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

    void OpcodeContext::makeOpcode(Byte one)
    {
        codeList_.push_back(one);
    }

    void OpcodeContext::makeOpcode(Byte opcode, Register reg)
    {
        codeList_.push_back(opcode);
        codeList_.push_back(reg);
    }

    void OpcodeContext::makeOpcode(Byte opcode, Register reg, Byte b)
    {
        codeList_.push_back(opcode);
        codeList_.push_back(reg);
        codeList_.push_back(b);
    }

    void OpcodeContext::makeOpcode(Byte opcode, Byte one, Byte two, Byte three)
    {
        codeList_.push_back(opcode);
        codeList_.push_back(one);
        codeList_.push_back(two);
        codeList_.push_back(three);
    }

    void OpcodeContext::pushInteger(int32_t num)
    {
        for (int i = 3; i >= 0; --i)
        {
            int offset = i * 8;
            int v = (num >> offset) & 0xff;
            codeList_.push_back(v);
        }
    }

    void OpcodeContext::setInteger(int index, int32_t num)
    {
        for (int i = 3; i >= 0; --i)
        {
            codeList_[index + i] = num & 0xff;
            num >>= 8;
        }
    }

    void OpcodeContext::bindGetLabelTarget(GetLabelTarget func)
    {
        getLabelTarget_ = std::move(func);
    }

    void OpcodeContext::bindGetFunctionTarget(GetFunctionTarget func)
    {
        getFunctionTarget_ = std::move(func);
    }

    void OpcodeContext::bindGetFunctionSlotTarget(GetFunctionSlotTarget func)
    {
        getFunctionSlotTarget_ = std::move(func);
    }

    void OpcodeContext::insertNewArray(Register reg, int32_t total)
    {
        makeOpcode(OK_NewArray, reg);
        pushInteger(total);
    }

    void OpcodeContext::insertNewSlot(const std::string &name)
    {
        makeOpcode(OK_NewSlot);
        slotBack_.push_back(std::pair<std::string, int>(name, getNextPos()));
        pushInteger(0);
    }

    void OpcodeContext::insertHalt()
    {
        makeOpcode(OK_Halt);
    }

    void OpcodeContext::insertEntry(int32_t offset, int32_t num)
    {
        entryOffset_ = getNextPos();
        makeOpcode(OK_Entry);
        pushInteger(offset);
        pushInteger(num);
    }

    void OpcodeContext::insertParam(Register reg)
    {
        makeOpcode(OK_Param, reg);
    }

    void OpcodeContext::insertIf(Register reg, Quad * label)
    {
        makeOpcode(OK_If, reg);
        labelBack_.push_back(std::pair<Quad*, int>(label, getNextPos()));
        pushInteger(0);
    }

    void OpcodeContext::insertIfFalse(Register reg, Quad * label)
    {
        makeOpcode(OK_IfFalse, reg);
        labelBack_.push_back(std::pair<Quad*, int>(label, getNextPos()));
        pushInteger(0);
    }

    // 
    // insert call opcode for function and buildin-function
    // 
    void OpcodeContext::insertCall(std::string & name, int num, int total, Register reg)
    {
        int index = buildin::BuildIn::getInstance().getFunctionIndex(name);
        if (index == -1)
        {
            makeOpcode(OK_Call, reg, (Byte)num, (Byte)total);
            functionBack_.push_back(std::pair<std::string, int>(name, getNextPos()));
            pushInteger(0);
        }
        else
        {
            makeOpcode(OK_BuildIn, reg, (Byte)num, (Byte)total);
            pushInteger(index);
        }
    }

    void OpcodeContext::insertGoto(Quad * label)
    {
        makeOpcode(OK_Goto);
        labelBack_.push_back(std::pair<Quad*, int>(label, getNextPos()));
        pushInteger(0);
    }

    void OpcodeContext::insertLoadA(Register regID, Register regIndex, Register regResult)
    {
        makeOpcode(OK_LoadA, regID, regIndex, regResult);
    }

    void OpcodeContext::insertStoreA(Register regID, Register regIndex, Register regResult)
    {
        makeOpcode(OK_StoreA, regID, regIndex, regResult);
    }

    void OpcodeContext::insertLoad(int index, Register reg)
    {
        makeOpcode(OK_Load, reg);
        pushInteger(index);
    }

    void OpcodeContext::insertStore(int index, Register reg)
    {
        makeOpcode(OK_Store, reg);
        pushInteger(index);
    }

    void OpcodeContext::insertInvoke(Register regID, int num, Register regResult)
    {
        makeOpcode(OK_Invoke, regResult, num, regID);
    }

    void OpcodeContext::insertReturn(Register reg)
    {
        makeOpcode(OK_Return, reg);
    }

    void OpcodeContext::insertSingleOP(unsigned op, Register from, Register result)
    {
        switch (op)
        {
        case TK_Sub:
            makeOpcode(OK_Negtive, from, result);
            break;
        case TK_Not:
            makeOpcode(OK_Not, from, result);
            break;
        }
    }

    void OpcodeContext::insertBinaryOP(unsigned op, Register regLeft, Register regRight, Register result)
    {
        Byte opcode;
        switch (op)
        {
        case TK_Plus:
            opcode = OK_Add;
            break;
        case TK_Sub:
            opcode = OK_Sub;
            break;
        case TK_Mul:
            opcode = OK_Mul;
            break;
        case TK_Div:
            opcode = OK_Div;
            break;
        case TK_Less:
            opcode = OK_Less;
            break;
        case TK_LessThan:
            opcode = OK_LessThan;
            break;
        case TK_Great:
            opcode = OK_Great;
            break;
        case TK_GreatThan:
            opcode = OK_GreatThan;
            break;
        case TK_NotEqual:
            opcode = OK_NotEqual;
            break;
        case TK_EqualThan:
            opcode = OK_Equal;
            break;
        }
        makeOpcode(opcode, result, regLeft, regRight);
    }

    void OpcodeContext::insertPushR(Register reg)
    {
        makeOpcode(OK_PushR, reg);
    }

    void OpcodeContext::insertPopR(Register reg)
    {
        makeOpcode(OK_PopR, reg);
    }

    void OpcodeContext::insertMove(Register dest, Register from)
    {
        makeOpcode(OK_Move, dest, from);
    }

    void OpcodeContext::insertMoveI(Register dest, int from)
    {
        makeOpcode(OK_MoveI, dest);
        pushInteger(from);
    }

    void OpcodeContext::insertMoveF(Register dest, float from)
    {
        makeOpcode(OK_MoveF, dest);
        pushInteger(from);
    }

    void OpcodeContext::insertMoveS(Register dest, int from)
    {
        makeOpcode(OK_MoveS, dest);
        pushInteger(from);
    }

    int OpcodeContext::getNextPos()
    {
        return codeList_.size();
    }
}


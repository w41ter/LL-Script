#include "opcode.h"

#include "../Parser/lexer.h"
#include "../IR/Quad.h"

namespace script
{
    Byte * OpcodeContext::getOpcodes()
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

            opcodes_ = new Byte[codeList_.size()];
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

    void OpcodeContext::pushInteger(int num)
    {
        for (int i = 3; i >= 0; --i)
        {
            int offset = i * 8;
            int v = (num >> offset) & 0xff;
            codeList_.push_back(v);
        }
    }

    void OpcodeContext::setInteger(int index, int num)
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

    void OpcodeContext::insertHalt()
    {
        makeOpcode(OK_Halt);
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

    void OpcodeContext::insertCall(std::string & name, int num, Register reg)
    {
        makeOpcode(OK_Call, reg, (Byte)num);
        functionBack_.push_back(std::pair<std::string, int>(name, getNextPos()));
        pushInteger(0);
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


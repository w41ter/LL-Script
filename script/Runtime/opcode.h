#ifndef __OPCODE_H__
#define __OPCODE_H__

#include <functional>
#include <string>
#include <map>
#include <list>
#include <vector>

#include "Runtime.h"

namespace script
{
    enum Register {
        RG_Begin = 0,
        RG_A = 0,
        RG_B = 1,
        RG_C = 2,
        RG_D = 3,
        RG_E = 4,
        RG_F = 5,
        RG_H = 6,
        RG_I = 7,
        RG_J = 8,
        RG_K = 9,
        RG_L = 10,
        RG_M = 11,
        RG_N = 12,
        RG_O = 13, 
        RG_P = 14,
        RG_Q = 15,
        RG_Total = 16
    };

    enum Opcode {
        OK_Goto = 0,    // goto lable@_addr

        // operator
        // single 
        OK_Negtive,     // temp = -temp
        OK_Not,         // temp = !temp
        // binary 
        OK_Add,         // temp = temp + temp
        OK_Sub,         // temp = temp - temp
        OK_Mul,         // temp = temp * temp
        OK_Div,         // temp = temp / temp
        // relop
        OK_Great,       // temp = temp > temp
        OK_GreatThan,   // temp = temp >= temp
        OK_Less,        // temp = temp < temp
        OK_LessThan,    // temp = temp <= temp
        OK_Equal,       // temp = temp == temp
        OK_NotEqual,    // temp = temp != temp

        // move
        OK_MoveS,       // temp = string index
        OK_MoveI,       // temp = constant
        OK_MoveF,
        OK_Move,        // temp = temp

        // memory
        OK_Load,        // load id to temp
        OK_LoadA,       // load id [ temp ] to temp
        OK_Store,       // store id from temp
        OK_StoreA,      // store id [ temp ] from temp

        // condition jmp
        OK_If,          // if temp goto label
        OK_IfFalse,     // if_false temp goto label

        // call 
        OK_Param,       // push temp
        OK_Call,        // temp = call Label in num params
        OK_Invoke,      // temp = invoke temp in num params
        OK_BuildIn,     // temp = call buildin function in num params

        OK_Return,      // return temp

        OK_PushR,       // push temp
        OK_PopR,        // pop temp

        OK_NewArray,    // reg = array [total]

        OK_NewSlot,     // slot , size of slot
        OK_Entry,        // entry of programs.  entry offset num
        OK_Halt,        // stop
    };

    class Quad;

    typedef int8_t Byte;

    //
    // +------------------+
    // | entry offset     |
    // +------------------+
    // | opcode length    |
    // +------------------+
    // | opcodes          |
    // +------------------+
    // | string nums      |
    // +------------------+
    // | string offsets   |
    // +------------------+
    // | string pool      |
    // +------------------+
    // 
    class OpcodeContext
    {
        using GetLabelTarget = std::function<int(Quad*)>;
        using GetFunctionTarget = std::function<int(std::string)>;
        using GetFunctionSlotTarget = std::function<int(std::string)>;

    public:
        OpcodeContext() : opcodes_(nullptr) { }

        void bindGetLabelTarget(GetLabelTarget func);
        void bindGetFunctionTarget(GetFunctionTarget func);
        void bindGetFunctionSlotTarget(GetFunctionSlotTarget func);

        void insertNewArray(Register reg, int32_t total);
        void insertNewSlot(const std::string &name);
        void insertHalt();
        void insertEntry(int32_t offset, int32_t num);
        void insertParam(Register reg);
        void insertIf(Register reg, Quad *label);
        void insertIfFalse(Register reg, Quad *label);
        void insertCall(std::string &name, int num, int total, Register reg);
        void insertGoto(Quad *label);
        void insertLoadA(Register regID, Register regIndex, Register regResult);
        void insertStoreA(Register regID, Register regIndex, Register regResult);
        void insertLoad(int index, Register reg);
        void insertStore(int index, Register reg);
        void insertInvoke(Register regID, int num, Register regResult);
        void insertReturn(Register reg);
        void insertSingleOP(unsigned op, Register from, Register result);
        void insertBinaryOP(unsigned op, Register regLeft, Register regRight, Register result);
        void insertPushR(Register reg);
        void insertPopR(Register reg);
        void insertMove(Register dest, Register from);
        void insertMoveI(Register dest, int from);
        void insertMoveF(Register dest, float from);
        void insertMoveS(Register dest, int from);
        int getNextPos();

        Byte *getOpcodes(int &length);
        size_t opcodeLength();

        int insertString(std::string &str);

    private:
        void makeOpcode(Byte opcode);
        void makeOpcode(Byte opcode, Register reg);
        void makeOpcode(Byte opcode, Register reg, Byte b);
        void makeOpcode(Byte opcode, Byte one, Byte two, Byte three);
        
        void pushInteger(int32_t i);
        void setInteger(int index, int32_t i);

        GetLabelTarget getLabelTarget_;
        GetFunctionTarget getFunctionTarget_;
        GetFunctionSlotTarget getFunctionSlotTarget_;

    private:
        Byte *opcodes_;
        std::vector<Byte> codeList_;

        int stringNum_ = 0;
        std::map<std::string, int> stringPool_;

        std::list<std::pair<std::string, int>> functionBack_;
        std::list<std::pair<Quad*, int>> labelBack_;
        std::list<std::pair<std::string, int>> slotBack_;

        int opcodeLength_;
        int entryOffset_;
    };
}

#endif // !__OPCODE_H__


#ifndef __OPCODE_H__
#define __OPCODE_H__

#include <string>
#include <map>
#include <list>

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
        RG_End = 16
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
        OK_MoveC,       // temp = constant
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

        OK_Return,      // return temp
    };

    class OpcodeContext
    {
    public:
        OpcodeContext() : opcodes_(nullptr) {}

        void insertSingleOP(unsigned op);
        void insertBinaryOP(unsigned op);

        void push();
        void push(std::string str);
        void push(int value);
        void push(float value);



        void insertMove();
        void insertMove(std::string str);
        void insertMove(int value);
        void insertMove(float value);

        void insertLoad(int index);
        void insertLoadA(int index);
        Pointer *getOpcodes();
        size_t opcodeLength();

        int insertString(std::string &str);

    private:
        Pointer *opcodes_;

        std::list<Pointer> codeList_;

        int stringNum_ = 0;
        std::map<std::string, int> stringPool_;
    };
}

#endif // !__OPCODE_H__


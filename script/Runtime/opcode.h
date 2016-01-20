#ifndef __OPCODE_H__
#define __OPCODE_H__

namespace script
{
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

    };
}
#endif // !__OPCODE_H__


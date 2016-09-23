#ifndef __OPCODE_H__
#define __OPCODE_H__

#include <functional>
#include <string>
#include <map>
#include <list>
#include <vector>
#include <memory>

#include "Runtime.h"

namespace script
{
	typedef int8_t Byte;

    enum Opcode {
        OK_Goto = 0,    // goto lable@_addr

        // operator
        // single 
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
        OK_MoveF,		// temp = float
		OK_MoveN,		// temp = null
        OK_Move,        // temp = temp

        // memory
        OK_Load,        // load id to temp
        OK_Index,       // load id [ temp ] to temp
        OK_Store,       // store id from temp
        OK_SetIndex,      // store id [ temp ] from temp

        // condition jmp
        OK_If,          // if temp goto label

        // call 
        OK_Param,       // push temp
        OK_Call,        // temp = call Label in num params
        OK_TailCall,
        OK_Return,      // return temp
		OK_NewHash,		// tmp = new hash
		OK_NewClosure,	// tmp = new string(idx)
		OK_UserClosure, // tmp = new user closure
        OK_Halt,        // stop
    };
    
}

#endif // !__OPCODE_H__


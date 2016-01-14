#ifndef __VM_H__
#define __VM_H__

#include <stdexcept>
#include <vector>
#include <string>
#include <stack>

#include "Runtime.h"
#include "GC.h"

namespace script
{
    enum Opcode
    {
        ADD, SUB, MUL, DIV,

        GREAT, GREAT_THAN, LESS, LESS_THAN, NOT_EQUAL, EQUAL,

        ICONST, CCONST, SCONST, RCONST, 
        NEW_ARRAY, 
        LOAD_LOCAL, SET_LOCAL, LOAD_GLOBAL, SET_GLOBAL, 
        PARAM, CALL, CALL_GLOBAL, RET, HALT
    };

    class VM
    {
        typedef unsigned char Code;
    public:
        VM(Code *opcode, size_t length, size_t osl, size_t isl)
            : opcode_(opcode)
            , length_(length)
            , gc(1024 * 1024)
            , ip_(0)
        {}

        ~VM()
        {
        }

        void excute();

        void push_back(std::string str)
        {
            globalString_.push_back(std::move(str));
        }

    private:

        void callClosure(Pointer closure, size_t length);
        void calculate(unsigned op, Pointer left, Pointer right);
        void compare(unsigned op, Pointer left, Pointer right);

    private:
        Pointer *global_;
        Pointer *local_;
        std::stack<Pointer*> localStack_;

        // op stack
        std::stack<Pointer> operandStack_;
        std::stack<Pointer> ipStack_;
        std::stack<Pointer> paramsStack_;

        // code
        Code *opcode_;
        size_t length_;
        size_t ip_;

        std::vector<std::string> globalString_;

        GC gc;
    };
}

#endif // !__VM_H__

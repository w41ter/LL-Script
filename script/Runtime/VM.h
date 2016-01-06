#ifndef __VM_H__
#define __VM_H__

#include <stdexcept>
#include <vector>
#include <string>

#include "Runtime.h"
#include "GC.h"

namespace script
{
    enum Opcode
    {
        ADD, SUB, MUL, DIV,

        

        SINGLE_OPCODE = 50,

        ICONST, CCONST, SCONST, RCONST, CLOSURE,
        NEW_ARRAY, 
        LOAD_LOCAL, SET_LOCAL, 
        PARAM, CALL, RET, HALT
    };

    class VM
    {
        typedef unsigned char Code;
    public:
        VM(Code *opcode, size_t length, size_t osl, size_t isl)
            : operandStack_(new Pointer[osl])
            , operandStackLength_(osl)
            , ipStack_(new size_t[isl])
            , ipStackLength_(isl)
            , opcode_(opcode)
            , length_(length)
            , gc(1024 * 1024)
        {}

        ~VM()
        {
            delete []operandStack_;
            delete []ipStack_;
        }

        void excute();

    private:
        size_t sizeOfOpcode(Code op)
        {
            return op < SINGLE_OPCODE ? 1 : 2;
        }

    private:
        // op stack
        Pointer *operandStack_;
        size_t operandStackLength_;
        size_t osp_ = 3;

        size_t *ipStack_;
        size_t ipStackLength_;
        size_t isp_ = 0;

        Pointer paramsStack_[32];
        size_t paramsStackLength = 32;
        size_t psp_ = 0;

        // code
        Code *opcode_;
        size_t length_;
        size_t ip_;

        std::vector<std::string> globalString_;

        const int tag_ = 3;

        GC gc;
    };
}

#endif // !__VM_H__

#ifndef __VM_H__
#define __VM_H__

#include <stdexcept>
#include <vector>
#include <string>
#include <stack>

#include "opcode.h"
#include "Runtime.h"
#include "GC.h"

namespace script
{
    class VirtualMachine
    {
    public:
        VirtualMachine();
        void excute(Byte *opcode, size_t length);

    private:
        void loadStringPool();

        int32_t getInteger(size_t & ip);
        float getFloat(size_t & ip);

        void excuteBinary(size_t &ip, unsigned op);
        void excuteSingle(size_t &ip, unsigned op);
        void excuteCall(size_t &ip);
        void excuteInvoke(size_t &ip);
        void excuteGoto(size_t &ip);
        void excuteIf(size_t &ip);
        void excuteIfFalse(size_t &ip);
        void excuteReturn(size_t &ip);
        void excuteLoad(size_t & ip);
        void excuteLoadA(size_t & ip);
        void excuteParam(size_t &ip);
        void excuteStore(size_t &ip);
        void excuteStoreA(size_t & ip);
        void excuteMove(size_t & ip);
        void excuteMoveF(size_t & ip);
        void excuteMoveI(size_t & ip);
        void excuteMoveS(size_t & ip);
        void excutePushR(size_t & ip);
        void excutePopR(size_t & ip);
        void excuteEntry(size_t & ip);

        void setRegister(unsigned reg, Pointer value);
        Pointer getRegister(unsigned reg);

        void processGlobals();
        void variableReference(Pointer *);
    private:
        Byte *opcodes_ = nullptr;
        size_t length_;
        size_t opcodeLength_;
        int offset_;
        int frame_;
        std::vector<std::string> stringPool_;
        std::stack<Pointer> *currentStack_;
        Pointer *localSlot;
        Pointer result_;
        GarbageCollector gc;
        struct {
            Pointer regA_, regB_, regC_, regD_, regE_, regF_, regH_, regI_, regJ_,
                regK_, regL_, regM_, regN_, regO_, regP_, regQ_;
        };

    };
}

#endif // !__VM_H__

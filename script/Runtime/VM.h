#ifndef __VM_H__
#define __VM_H__

#include <stdexcept>
#include <vector>
#include <string>
#include <stack>
#include <queue>

#include "opcode.h"
#include "Runtime.h"
#include "GC.h"

namespace script
{
    // 
    // TODO: 这个结构需要优化
    //
    struct Frame 
    {
        Frame(size_t slot, Frame *parent = nullptr) 
            : localSlot_(slot)
            , previous_(parent)
        {}

        struct 
        {
            Pointer regA_, regB_, regC_, regD_, regE_, 
                regF_, regH_, regI_, regJ_, regK_, 
                regL_, regM_, regN_, regO_, regP_, regQ_;
        } register_;

        std::vector<Pointer> localSlot_;
        unsigned result_;
        std::stack<Pointer> registerStack_;
        Frame *previous_;
        size_t ip_;
    };

    class VirtualMachine
    {
    public:
        VirtualMachine();
        void excute(Byte *opcode, size_t length);
        GarbageCollector *getGC();

    private:
        void loadStringPool();

        int32_t getInteger(size_t & ip);
        float getFloat(size_t & ip);

        void excuteBinary(size_t &ip, unsigned op);
        void excuteSingle(size_t &ip, unsigned op);
        void excuteCall(size_t &ip);
        void excuteInvoke(size_t &ip);
        void excuteBuildIn(size_t &ip);
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
        void excuteNewSlot(size_t &ip);
        void excuteNewArray(size_t & ip);

        void excuteClosure(unsigned result, int num, Pointer value, size_t &ip);
        void excuteBuildInClosure(unsigned result, int num, Pointer value, size_t &ip);

        void setRegister(unsigned reg, Pointer value);
        Pointer getRegister(unsigned reg);

        void processGlobals();
        void variableReference(Pointer *);

        Pointer add(Pointer lhs, Pointer rhs);
        Pointer sub(Pointer lhs, Pointer rhs);
        Pointer mul(Pointer lhs, Pointer rhs);
        Pointer div(Pointer lhs, Pointer rhs);
        Pointer g(Pointer lhs, Pointer rhs);
        Pointer gt(Pointer lhs, Pointer rhs);
        Pointer l(Pointer lhs, Pointer rhs);
        Pointer lt(Pointer lhs, Pointer rhs);
        Pointer et(Pointer lhs, Pointer rhs);
        Pointer ne(Pointer lhs, Pointer rhs);
        Pointer not(Pointer lhs);
        Pointer negative(Pointer lhs);

    private:
        Byte *opcodes_ = nullptr;
        size_t length_;
        size_t opcodeLength_;

        GarbageCollector gc_;

        Frame *globalFrame_ = nullptr;
        Frame *currentFrame_ = nullptr;
        
        // TODO: paramStatck_ 需要优化
        std::vector<Pointer> paramStack_;
        std::vector<std::string> stringPool_;
        //std::stack<Frame> frameStack_;
    };
}

#endif // !__VM_H__

#ifndef __DUMP_OPCODE_H__
#define __DUMP_OPCODE_H__

#include <fstream>

#include "opcode.h"

namespace script
{
    
    class DumpOpcode
    {
    public:
        DumpOpcode(std::fstream &file);

        void dump(Byte *opcode, size_t length);

    private:
        void dumpBinary(size_t &ip);
        void dumpSingle(size_t &ip);
        void dumpCall(size_t &ip);
        void dumpInvoke(size_t &ip);
        void dumpGoto(size_t &ip);
        void dumpIf(size_t &ip);
        void dumpIfFalse(size_t &ip);
        void dumpReturn(size_t &ip);
        void dumpLoad(size_t &ip);
        void dumpLoadA(size_t &ip);
        void dumpStore(size_t &ip);
        void dumpStoreA(size_t &ip);
        void dumpMove(size_t &ip);
        void dumpMoveF(size_t &ip);
        void dumpMoveI(size_t &ip);
        void dumpMoveS(size_t &ip);
        void dumpHalt(size_t &ip);
        void dumpParam(size_t &ip);
        void dumpPushR(size_t &ip);
        void dumpPopR(size_t &ip);
        void dumpEntry(size_t &ip);
        void dumpNewSlot(size_t &ip);
        void dumpNewArray(size_t & ip);

        void dumpRegister(unsigned reg);
        void dumpStringPool();

        int32_t getInteger(size_t &ip);
        float getFloat(size_t &ip);
    private:
        std::fstream &file_;

        Byte *opcode_;
        size_t length_;
        size_t opcodeLength_;
    };
}

#endif // !__DUMP_OPCODE_H__



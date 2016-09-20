#ifndef __DUMP_OPCODE_H__
#define __DUMP_OPCODE_H__

#include <fstream>
#include <string>

#include "opcode.h"

namespace script
{
	class OpcodeModule;
	class OpcodeFunction;

    class DumpOpcode
    {
    public:
		typedef std::vector<Byte> Opcode;

        DumpOpcode(OpcodeModule &OM, std::string &file);

        void dump();
		void dumpInstruction(Opcode &opcode, size_t &ip);
    private:
		void dumpFunction(OpcodeFunction &func);
        void dumpBinary(const Opcode &opcode, size_t &ip);
        void dumpNotOP(const Opcode &opcode, size_t &ip);
        void dumpCall(const Opcode &opcode, size_t &ip);
        void dumpTailCall(const Opcode &opcode, size_t &ip);
        void dumpGoto(const Opcode &opcode, size_t &ip);
        void dumpIf(const Opcode &opcode, size_t &ip);
		void dumpReturn(const Opcode &opcode, size_t &ip);
        void dumpLoad(const Opcode &opcode, size_t &ip);
        void dumpStore(const Opcode &opcode, size_t &ip);
		void dumpIndex(const Opcode &opcode, size_t &ip);
		void dumpSetIndex(const Opcode &opcode, size_t &ip);
        void dumpMove(const Opcode &opcode, size_t &ip);
        void dumpMoveF(const Opcode &opcode, size_t &ip);
        void dumpMoveI(const Opcode &opcode, size_t &ip);
		void dumpMoveS(const Opcode &opcode, size_t &ip);
		void dumpMoveN(const Opcode &opcode, size_t &ip);
        void dumpHalt(const Opcode &opcode, size_t &ip);
        void dumpParam(const Opcode &opcode, size_t &ip);
        void dumpNewClosure(const Opcode &opcode, size_t &ip);
        void dumpNewHash(const Opcode &opcode, size_t & ip);

        void dumpRegister(unsigned reg);
        void dumpStringPool();

        int32_t getInteger(const Opcode &opcode, size_t &ip);
        float getFloat(const Opcode &opcode, size_t &ip);
    private:
        std::fstream file_;

		OpcodeModule &module;
    };
}

#endif // !__DUMP_OPCODE_H__



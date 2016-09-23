#include "dumpOpcode.h"

#include <iomanip>

#include "OpcodeModule.h"

using std::endl;

namespace script
{
    DumpOpcode::DumpOpcode(OpcodeModule &OM, std::string &file)
        : file_(file, std::ios::out), module(OM)
    {
    }

    void DumpOpcode::dump()
    {
		for (auto i : module.functions_) {
			dumpFunction(i.second);
		}

        dumpStringPool();
    }

	void DumpOpcode::dumpInstruction(Opcode & opcode, size_t & ip)
	{
	}

	void DumpOpcode::dumpFunction(OpcodeFunction & func)
	{
		const auto &opcode = func.codes;

		file_ << "[Function]: " << module.getString(func.name) 
			<< "\nbegin : " << endl;

		size_t ip = 0;
		while (true)
		{
			if (ip >= opcode.size())
				break;

			file_ << "  0x" << std::setfill('0') 
				<< std::setw(8) << ip << ":\t";

			switch (opcode[ip++])
			{
			case OK_Add:
			case OK_Sub:
			case OK_Div:
			case OK_Mul:
			case OK_Great:
			case OK_GreatThan:
			case OK_Less:
			case OK_LessThan:
			case OK_NotEqual:
			case OK_Equal:
				dumpBinary(opcode, ip);
				break;
			case OK_Not:
				dumpNotOP(opcode, ip);
				break;
			case OK_Call:
				dumpCall(opcode, ip);
				break;
			case OK_TailCall:
				dumpTailCall(opcode, ip);
				break;
			case OK_Goto:
				dumpGoto(opcode, ip);
				break;
			case OK_If:
				dumpIf(opcode, ip);
				break;
			case OK_Return:
				dumpReturn(opcode, ip);
				break;
			case OK_Load:
				dumpLoad(opcode, ip);
				break;
			case OK_Store:
				dumpStore(opcode, ip);
				break;
			case OK_Move:
				dumpMove(opcode, ip);
				break;
			case OK_MoveF:
				dumpMoveF(opcode, ip);
				break;
			case OK_MoveI:
				dumpMoveI(opcode, ip);
				break;
			case OK_MoveS:
				dumpMoveS(opcode, ip);
				break;
			case OK_MoveN:
				dumpMoveN(opcode, ip);
				break;
			case OK_Halt:
				dumpHalt(opcode, ip);
				break;
			case OK_Param:
				dumpParam(opcode, ip);
				break;
			case OK_NewClosure:
				dumpNewClosure(opcode, ip);
				break;
			case OK_UserClosure:
				dumpUserClosure(opcode, ip);
				break;
			case OK_NewHash:
				dumpNewHash(opcode, ip);
				break;
			case OK_Index:
				dumpIndex(opcode, ip);
				break;
			case OK_SetIndex:
				dumpSetIndex(opcode, ip);
				break;
			}
		}
		file_ << "\n\n";
	}

	void DumpOpcode::dumpBinary(const Opcode &opcode, size_t & ip)
    {
        dumpRegister(opcode[ip++]); file_ << " = ";
        dumpRegister(opcode[ip++]); 
        switch (opcode[ip - 3])
        {
        case OK_Add: file_ << " + "; break;
        case OK_Sub: file_ << " - "; break;
        case OK_Div: file_ << " / "; break;
        case OK_Mul: file_ << " * "; break;
        case OK_Great: file_ << " > "; break;
        case OK_GreatThan: file_ << " >= "; break;
        case OK_Less: file_ << " < "; break;
        case OK_LessThan: file_ << " <= "; break;
        case OK_NotEqual: file_ << " != "; break;
        case OK_Equal: file_ << " == "; break;
        }
        dumpRegister(opcode[ip++]);
        file_ << endl;
    }

    void DumpOpcode::dumpNotOP(const Opcode &opcode, size_t & ip)
    {
        dumpRegister(opcode[ip++]); 
		file_ << " = !";
        dumpRegister(opcode[ip++]);
        file_ << endl;
    }

    void DumpOpcode::dumpCall(const Opcode &opcode, size_t & ip)
    {
        dumpRegister(opcode[ip++]);
        file_ << " = call "; 
		dumpRegister(opcode[ip++]);
        file_ << " <params>:";
        file_ << getInteger(opcode, ip) << endl;
    }

    void DumpOpcode::dumpTailCall(const Opcode &opcode, size_t & ip)
    {
		dumpRegister(opcode[ip++]);
		file_ << " = tail_call ";
		dumpRegister(opcode[ip++]);
		file_ << " <params>:";
		file_ << getInteger(opcode, ip) << endl;
    }

    void DumpOpcode::dumpGoto(const Opcode &opcode, size_t & ip)
    {
        file_ << "goto "; 
        file_ << "@0x" << std::setfill('0')
			<< std::setw(8) 
			<< getInteger(opcode, ip) << endl;
    }

    void DumpOpcode::dumpIf(const Opcode &opcode, size_t & ip)
    {
        file_ << "if ";
        dumpRegister(opcode[ip++]);
		file_ << " to @0x" << std::setfill('0')
			<< std::setw(8)
			<< getInteger(opcode, ip) << endl;
    }

    void DumpOpcode::dumpReturn(const Opcode &opcode, size_t & ip)
    {
        file_ << "return ";
        dumpRegister(opcode[ip++]);
        file_ << endl;
    }

    void DumpOpcode::dumpLoad(const Opcode &opcode, size_t & ip)
    {
        file_ << "load ";
        dumpRegister(opcode[ip++]);
        file_ << " at " << getInteger(opcode, ip) << endl;
    }

    void DumpOpcode::dumpStore(const Opcode &opcode, size_t & ip)
    {
        file_ << "store ";
        dumpRegister(opcode[ip++]);
        file_ << " into " << getInteger(opcode, ip) << endl;
    }

	void DumpOpcode::dumpIndex(const Opcode & opcode, size_t & ip)
	{
		dumpRegister(opcode[ip++]);
		file_ << " = ";
		dumpRegister(opcode[ip++]);
		file_ << "[";
		dumpRegister(opcode[ip++]);
		file_ << "]" << endl;
	}

	void DumpOpcode::dumpSetIndex(const Opcode & opcode, size_t & ip)
	{
		dumpRegister(opcode[ip++]);
		file_ << "[";
		dumpRegister(opcode[ip++]);
		file_ << "] = ";
		dumpRegister(opcode[ip++]);
		file_ << endl;
	}

    void DumpOpcode::dumpMove(const Opcode &opcode, size_t & ip)
    {
        dumpRegister(opcode[ip++]);
        file_ << " = ";
        dumpRegister(opcode[ip++]);
        file_ << endl;
    }

    void DumpOpcode::dumpMoveF(const Opcode &opcode, size_t & ip)
    {
        dumpRegister(opcode[ip++]);
        file_ << " = " << getFloat(opcode, ip) << endl;
    }

    void DumpOpcode::dumpMoveI(const Opcode &opcode, size_t & ip)
    {
        dumpRegister(opcode[ip++]);
        file_ << " = " << getInteger(opcode, ip) << endl;
    }

    void DumpOpcode::dumpMoveS(const Opcode &opcode, size_t & ip)
    {
        dumpRegister(opcode[ip++]);
        file_ << " = ";
        file_ << "<string offset>:" << getInteger(opcode, ip);
        file_ << endl;
    }

	void DumpOpcode::dumpMoveN(const Opcode & opcode, size_t & ip)
	{
		dumpRegister(opcode[ip++]);
		file_ << " = null";
		file_ << endl;
	}

    void DumpOpcode::dumpHalt(const Opcode &opcode, size_t & ip)
    {
        file_ << "halt" << endl;
    }

    void DumpOpcode::dumpParam(const Opcode &opcode, size_t & ip)
    {
        file_ << "param ";
        dumpRegister(opcode[ip++]);
        file_ << endl;
    }

    void DumpOpcode::dumpNewClosure(const Opcode &opcode, size_t & ip)
    {
		dumpRegister(opcode[ip++]);
		int32_t offset = getInteger(opcode, ip);
		const std::string &name = module.getString(offset);
		file_ << " = new closure : " << name;
		file_ << " <params>:";
		file_ << getInteger(opcode, ip) << endl;
    }

	void DumpOpcode::dumpUserClosure(const Opcode & opcode, size_t & ip)
	{
		dumpRegister(opcode[ip++]);
		int32_t offset = getInteger(opcode, ip);
		const std::string &name = module.getString(offset);
		file_ << " = new user closure : " << name << endl;
	}

    void DumpOpcode::dumpNewHash(const Opcode &opcode, size_t &ip)
    {
        dumpRegister(opcode[ip++]);
        file_ << " = new hash " << endl;
    }
    
    void DumpOpcode::dumpRegister(unsigned reg)
    {
		file_ << "$" << reg;
    }

    void DumpOpcode::dumpStringPool()
    {
        file_ << "\n\n[String pool]: \n  size = " 
			<< module.stringPool_.size() << endl;

		for (size_t i = 0; i < module.stringPool_.size(); ++i) {
			file_ << "\t" << i << " : " << module.stringPool_[i] << endl;
		}
    }

    int32_t DumpOpcode::getInteger(const Opcode &opcode, size_t & ip)
    {
        int32_t result = 0;
        for (int i = 0; i < 4; ++i)
        {
            result <<= 8; result |= (unsigned char)opcode[ip++];
        }
        return result;
    }

    float DumpOpcode::getFloat(const Opcode &opcode, size_t & ip)
    {
		int val = getInteger(opcode, ip);
        return *(float*)&val;
    }
}


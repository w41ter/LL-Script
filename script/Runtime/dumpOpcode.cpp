#include "dumpOpcode.h"

using std::endl;

namespace script
{
    DumpOpcode::DumpOpcode(std::fstream & file) 
        : file_(file)
    {
    }

    void DumpOpcode::dump(Byte * opcode, size_t length)
    {
        opcode_ = opcode; length_ = length;

        size_t ip = 0;
        while (true)
        {
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
                dumpBinary(ip);
                break;
            case OK_Negtive:
            case OK_Not:
                dumpSingle(ip);
                break;
            case OK_Call:
                dumpCall(ip);
                break;
            case OK_Invoke:
                dumpInvoke(ip);
                break;
            case OK_Goto:
                dumpGoto(ip);
                break;
            case OK_If:
                dumpIf(ip);
                break;
            case OK_IfFalse:
                dumpIfFalse(ip);
                break;
            case OK_Return:
                dumpReturn(ip);
                break;
            case OK_Load:
                dumpLoad(ip);
                break;
            case OK_LoadA:
                dumpLoadA(ip);
                break;
            case OK_Store:
                dumpStore(ip);
                break;
            case OK_StoreA:
                dumpStoreA(ip);
                break;
            case OK_Move:
                dumpMove(ip);
                break;
            case OK_MoveF:
                dumpMoveF(ip);
                break;
            case OK_MoveI:
                dumpMoveI(ip);
                break;
            case OK_MoveS:
                dumpMoveS(ip);
                break;
            case OK_Halt:
                dumpHalt(ip);
                break;
            case OK_Param:
                dumpParam(ip);
                break;
            case OK_PushR:
                dumpPushR(ip);
                break;
            case OK_PopR:
                dumpPopR(ip);
                break;
            }
            if (ip > length) break;
        }
    }

    void DumpOpcode::dumpBinary(size_t & ip)
    {
        dumpRegister(opcode_[ip++]); file_ << "= ";
        dumpRegister(opcode_[ip++]); 
        switch (opcode_[ip - 3])
        {
        case OK_Add: file_ << "+ "; break;
        case OK_Sub: file_ << "- "; break;
        case OK_Div: file_ << "/ "; break;
        case OK_Mul: file_ << "* "; break;
        case OK_Great: file_ << "> "; break;
        case OK_GreatThan: file_ << ">= "; break;
        case OK_Less: file_ << "< "; break;
        case OK_LessThan: file_ << "<= "; break;
        case OK_NotEqual: file_ << "!= "; break;
        case OK_Equal: file_ << "== "; break;
        }
        dumpRegister(opcode_[ip++]);
        file_ << endl;
    }

    void DumpOpcode::dumpSingle(size_t & ip)
    {
        dumpRegister(opcode_[ip++]); file_ << "= ";
        switch (opcode_[ip - 2])
        {
        case OK_Not: file_ << "! "; break;
        case OK_Negtive: file_ << "- "; break;
        }
        dumpRegister(opcode_[ip++]);
        file_ << endl;
    }

    void DumpOpcode::dumpCall(size_t & ip)
    {
        dumpRegister(opcode_[ip++]);
        file_ << "= call "; 
        file_ << opcode_[ip++] << ' ';
        file_ << *((int*)(opcode_ + ip));
        ip += 4;
        file_ << endl;
    }

    void DumpOpcode::dumpInvoke(size_t & ip)
    {
        dumpRegister(opcode_[ip++]);
        file_ << "= invoke ";
        dumpRegister(opcode_[ip++]);
        dumpRegister(opcode_[ip++]);
        file_ << endl;
    }

    void DumpOpcode::dumpGoto(size_t & ip)
    {
        file_ << "goto "; 
        dumpRegister(opcode_[ip++]);
        file_ << endl;
    }

    void DumpOpcode::dumpIf(size_t & ip)
    {
        file_ << "if ";
        dumpRegister(opcode_[ip++]);
        file_ << *((int*)(opcode_ + ip));
        ip += 4;
        file_ << endl;
    }

    void DumpOpcode::dumpIfFalse(size_t & ip)
    {
        file_ << "if_false ";
        dumpRegister(opcode_[ip++]);
        file_ << *((int*)(opcode_ + ip));
        ip += 4;
        file_ << endl;
    }

    void DumpOpcode::dumpReturn(size_t & ip)
    {
        file_ << "return ";
        dumpRegister(opcode_[ip++]);
        file_ << endl;
    }

    void DumpOpcode::dumpLoad(size_t & ip)
    {
        file_ << "load ";
        dumpRegister(opcode_[ip++]);
        file_ << *((int*)(opcode_ + ip));
        ip += 4;
        file_ << endl;
    }

    void DumpOpcode::dumpLoadA(size_t & ip)
    {
        file_ << "load_array ";
        dumpRegister(opcode_[ip++]);
        dumpRegister(opcode_[ip++]);
        dumpRegister(opcode_[ip++]);
        file_ << endl;
    }

    void DumpOpcode::dumpStore(size_t & ip)
    {
        file_ << "store ";
        dumpRegister(opcode_[ip++]);
        file_ << *((int*)(opcode_ + ip));
        ip += 4;
        file_ << endl;
    }

    void DumpOpcode::dumpStoreA(size_t & ip)
    {
        file_ << "store_array ";
        dumpRegister(opcode_[ip++]);
        dumpRegister(opcode_[ip++]);
        dumpRegister(opcode_[ip++]);
        file_ << endl;
    }

    void DumpOpcode::dumpMove(size_t & ip)
    {
        dumpRegister(opcode_[ip++]);
        file_ << "= ";
        dumpRegister(opcode_[ip++]);
        file_ << endl;
    }

    void DumpOpcode::dumpMoveF(size_t & ip)
    {
        dumpRegister(opcode_[ip++]);
        file_ << "= ";
        file_ << *((float*)(opcode_ + ip));
        ip += 4;
        file_ << endl;
    }

    void DumpOpcode::dumpMoveI(size_t & ip)
    {
        dumpRegister(opcode_[ip++]);
        file_ << "= ";
        file_ << *((int*)(opcode_ + ip));
        ip += 4;
        file_ << endl;
    }

    void DumpOpcode::dumpMoveS(size_t & ip)
    {
        dumpRegister(opcode_[ip++]);
        file_ << "= ";
        file_ << "<string> ";
        file_ << endl;
    }

    void DumpOpcode::dumpHalt(size_t & ip)
    {
        file_ << "halt" << endl;
    }

    void DumpOpcode::dumpParam(size_t & ip)
    {
        file_ << "param ";
        dumpRegister(opcode_[ip++]);
        file_ << endl;
    }

    void DumpOpcode::dumpPushR(size_t & ip)
    {
        file_ << "push ";
        dumpRegister(opcode_[ip++]);
        file_ << endl;
    }

    void DumpOpcode::dumpPopR(size_t & ip)
    {
        file_ << "pop ";
        dumpRegister(opcode_[ip++]);
        file_ << endl;
    }
    
    void DumpOpcode::dumpRegister(unsigned reg)
    {
        switch (reg)
        {
        case RG_A: file_ << "reg_1 "; break;
        case RG_B: file_ << "reg_2 "; break;
        case RG_C: file_ << "reg_3 "; break;
        case RG_D: file_ << "reg_4 "; break;
        case RG_E: file_ << "reg_5 "; break;
        case RG_F: file_ << "reg_6 "; break;
        case RG_H: file_ << "reg_7 "; break;
        case RG_I: file_ << "reg_8 "; break;
        case RG_J: file_ << "reg_9 "; break;
        case RG_K: file_ << "reg_10 "; break;
        case RG_L: file_ << "reg_11 "; break;
        case RG_M: file_ << "reg_12 "; break;
        case RG_N: file_ << "reg_13 "; break;
        case RG_O: file_ << "reg_14 "; break;
        case RG_P: file_ << "reg_15 "; break;
        case RG_Q: file_ << "reg_16 "; break;
        }
    }
}


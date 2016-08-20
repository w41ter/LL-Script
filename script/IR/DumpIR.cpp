#include "DumpIR.h"

#include "Instruction.h"
#include "IRModule.h"

using namespace script::ir;

using std::endl;

namespace script
{
    DumpIR::DumpIR(std::string &file)
        : filename_(file)
    {
        file_ = std::fstream(file, std::ios::out);
    }

    DumpIR::~DumpIR()
    {
    }

    void DumpIR::dump(IRModule *module)
    {
        for (auto pair : module->functions_)
        {
            dump(pair.second);
        }

        file_ << "Begin for expressions:" << endl;
        dump((IRCode*)module);
    }

    void DumpIR::dump(IRFunction *function)
    {
        file_ << "Define " << function->getName() << " params: ";
        for (auto i = function->param_begin(), e = function->param_end();
            i != e; ++i)
            file_ << i->first << ' ';
        file_ << "{\n";
        dump((IRCode*)function);
        file_ << "}\n\n";
    }

    void DumpIR::dump(IRCode *code)
    {
        CFG *cfg = code;
        for (auto i = cfg->begin(), e = cfg->end(); i != e; ++i)
            dumpBlock(*i);
    }

    void DumpIR::dumpBlock(BasicBlock *block)
    {
        file_ << block->getName() << ':' << endl;
        for (auto i = block->instr_begin(), e = block->instr_end();
            i != e; ++i)
            dumpInstr(*i);
    }

    void DumpIR::dumpInstr(Instruction *instr)
    {
        switch (instr->instance())
        {
        case Instructions::IR_Constant:
        {
            file_ << "[[Error]]";
            break;
        }
        case Instructions::IR_Table:
        {
            file_ << "[[Error]]";
            break;
        }
        case Instructions::IR_Undef:
        {
            file_ << "[[Undef]]";
            break;
        }
        case Instructions::IR_Invoke:
            dumpInvoke(instr); break;
        case Instructions::IR_Branch:
            dumpBranch(instr); break;
        case Instructions::IR_Goto:
            dumpGoto(instr); break;
        case Instructions::IR_Assign:
            dumpAssign(instr); break;
        case Instructions::IR_NotOp:
            dumpNotOp(instr); break;
        case Instructions::IR_Return:
            dumpReturn(instr); break;
        case Instructions::IR_ReturnVoid:
            dumpReturnVoid(instr); break;
        case Instructions::IR_BinaryOps:
            dumpBinaryOps(instr); break;
        case Instructions::IR_Index:
            dumpIndex(instr); break;
        case Instructions::IR_SetIndex:
            dumpSetIndex(instr); break;
        case Instructions::IR_Phi:
            dumpPhi(instr); break;
        case Instructions::IR_Store:
            dumpStore(instr); break;
        default:
            break;
        }
    }

    void DumpIR::dumpValue(Constant *cons)
    {
        switch (cons->type())
        {
        case Constant::Null:
            file_ << "nil";
            break;
        case Constant::Boolean:
            file_ << (cons->getBoolean() ? "True" : "False");
            break;
        case Constant::Character:
            file_ << '\'' << cons->getChar() << '\'';
            break;
        case Constant::Integer:
            file_ << cons->getInteger();
            break;
        case Constant::Float:
            file_ << cons->getFloat();
            break;
        case Constant::String:
            file_ << '"' << cons->getString() << '"';
        default:
            break;
        }
    }

    void DumpIR::dumpInvoke(Instruction *instr)
    {
        Invoke *invoke = (Invoke*)instr;
        auto op = invoke->op_begin();
        file_ << "    " << invoke->name() << " = Invoke ";
        if (invoke->isCalledByName())
        {
            file_ << "$" << invoke->invokedName();
        }
        else
        {
            file_ << ((Instruction*)op++->getValue())->name();
        }
        file_ << "( ";
        while (op != invoke->op_end())
        {
            Instruction *in = (Instruction*)op->getValue();
            file_ << in->name() << ' ';
            ++op;
        }
        file_ << ")\n";
    }

    void DumpIR::dumpBranch(Instruction *instr)
    {
        Branch *branch = (Branch*)instr;
        Value *value = branch->op_begin()->getValue();
        Instruction *cond = (Instruction*)value;
        std::string name = value->instance() == Instructions::IR_Undef
            ? "[[Undef]]" : cond->name();
        file_ << "    Branch " << name << ' '
            << branch->then()->getName() << ' '
            << branch->_else()->getName() << endl;
        //if (branch->hasElseBlock())
    }

    void DumpIR::dumpGoto(Instruction *instr)
    {
        Goto *go = (Goto*)instr;
        file_ << "    Goto " << go->block()->getName() << endl;
    }

    void DumpIR::dumpAssign(Instruction *instr)
    {
        Assign *assign = (Assign*)instr;
        file_ << "    " << assign->name() << " = ";
        Value *value = assign->op_begin()->getValue();
        if (value->instance() == Instructions::IR_Constant)
            dumpValue((Constant*)value);
        else if (value->instance() == Instructions::IR_Undef)
            file_ << "[[Undef]]";
        else if (value->instance() == Instructions::IR_Table)
            file_ << "[]";
        else
            file_ << ((Instruction*)value)->name();
        file_ << endl;
    }

    void DumpIR::dumpNotOp(Instruction *instr)
    {
        NotOp * not = (NotOp*)instr;
        Instruction *from = (Instruction*)not->op_begin()->getValue();
        file_ << "    " << not->name() << " = !" << from->name() << endl;
    }

    void DumpIR::dumpReturn(Instruction *instr)
    {
        Return *ret = (Return*)instr;
        Instruction *from = (Instruction*)ret->op_begin()->getValue();
        file_ << "    Return " << from->name() << endl;
    }

    void DumpIR::dumpReturnVoid(Instruction *instr)
    {
        file_ << "    ReturnVoid" << endl;
    }

    void DumpIR::dumpBinaryOps(Instruction *instr)
    {
        BinaryOperator *ops = (BinaryOperator*)instr;
        BinaryOps op = ops->op();
        auto iter = ops->op_begin();
        Instruction *lhs = (Instruction*)iter++->getValue();
        Instruction *rhs = (Instruction*)iter++->getValue();
        file_ << "    " << ops->name() << " = " << lhs->name() << ' ';
        switch (op)
        {
        case BinaryOps::BO_Add:
            file_ << '+';
            break;
        case BinaryOps::BO_Sub:
            file_ << '-';
            break;
        case BinaryOps::BO_Mul:
            file_ << '*';
            break;
        case BinaryOps::BO_Div:
            file_ << '/';
            break;
        case BinaryOps::BO_And:
            file_ << '&';
            break;
        case BinaryOps::BO_Or:
            file_ << '|';
            break;
        case BinaryOps::BO_Great:
            file_ << '>';
            break;
        case BinaryOps::BO_NotLess:
            file_ << ">=";
            break;
        case BinaryOps::BO_Less:
            file_ << '<';
            break;
        case BinaryOps::BO_NotGreat:
            file_ << "<=";
            break;
        case BinaryOps::BO_Equal:
            file_ << "==";
            break;
        case BinaryOps::BO_NotEqual:
            file_ << "!=";
            break;
        default:
            break;
        }
        file_ << ' ' << rhs->name() << endl;
    }

    void DumpIR::dumpIndex(Instruction *instr)
    {
        Index *index = (Index*)instr;
        auto op = index->op_begin();
        Instruction *table = (Instruction*)op++->getValue();
        Instruction *idx = (Instruction*)op++->getValue();
        file_ << "    " << index->name() << " = " << table->name()
            << '[' << idx->name() << ']' << endl;
    }

    void DumpIR::dumpSetIndex(Instruction *instr)
    {
        SetIndex *set = (SetIndex*)instr;
        auto op = set->op_begin();
        Instruction *table = (Instruction*)op++->getValue();
        Instruction *idx = (Instruction*)op++->getValue();
        Instruction *from = (Instruction*)op++->getValue();
        file_ << "    " << table->name() << '[' << idx->name()
            << ']' << " = " << from->name() << endl;
    }

    void DumpIR::dumpPhi(Instruction *instr)
    {
        Phi *phi = (Phi*)instr;
        file_ << "    " << phi->name() << " = Phi<";
        bool first = true;
        for (auto &use = phi->op_begin(); use != phi->op_end(); ++use)
        {
            if (!first)
                file_ << ", ";
            else
                first = false;
            Value *value = use->getValue();
            //if (value->instance() >= Instructions::IR_Undef)
            file_ << ((Instruction*)use->getValue())->name();
        }
        file_ << ">\n";
    }

    void DumpIR::dumpStore(Instruction *instr)
    {
        Store *store = (Store*)instr;
        Instruction *from = (Instruction*)store->op_begin()->getValue();
        file_ << "    " << store->name() << " := " << from->name() << endl;
    }
}


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

    void DumpIR::dump(const IRModule *module)
    {
        for (auto pair : module->functions_)
        {
            dump(pair.second);
        }

        file_ << "Begin for expressions:" << endl;
        dump((IRCode*)module);
    }

    void DumpIR::dump(const IRFunction *function)
    {
        file_ << "Define " << function->name_ << " params: ";
        for (auto &name : function->params_)
            file_ << name.first << ' ';
        file_ << "{\n";
        dump((IRCode*)function);
        file_ << "}\n\n";
    }

    void DumpIR::dump(const IRCode *code)
    {
        const CFG *cfg = code;
        for (auto *block : cfg->blocks_)
            dumpBlock(*block);
    }

    void DumpIR::dumpBlock(const BasicBlock &block)
    {
        file_ << block.getName() << ':' << endl;
        const Instruction *instr = block.begin();
        if (instr == nullptr)
            return;
        dumpInstr(*instr);
        while (instr != block.end())
        {
            instr = instr->next();
            dumpInstr(*instr);
        }
    }

    void DumpIR::dumpInstr(const Instruction &instr)
    {
        switch (instr.instance())
        {
        case Instructions::IR_Value:
        {
            file_ << "[[Error]]";
            break;
        }
        case Instructions::IR_Alloca:
            dumpAlloca(instr); break;
        case Instructions::IR_Load:
            dumpLoad(instr); break;
        case Instructions::IR_Store:
            dumpStore(instr); break;
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
        case Instructions::IR_Catch:
            dumpCatch(instr); break;
        default:
            break;
        }
    }

    void DumpIR::dumpValue(const Constant *cons)
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

    void DumpIR::dumpAlloca(const Instruction &instr)
    {
        const Alloca *alloca = (Alloca*)&instr;
        file_ << "    " << alloca->name() << " Alloca memory" << endl;
    }

    void DumpIR::dumpLoad(const Instruction &instr)
    {
        const Load *load = (const Load*)&instr;
        const Instruction *from = (Instruction*)(load->op_begin()->getValue());
        file_ << "    " << load->name() << " Load "
            << from->name() << endl;
    }

    void DumpIR::dumpStore(const Instruction &instr)
    {
        const Store *store = (Store*)&instr;
        auto op = store->op_begin();
        const Instruction *value = (Instruction*)op++->getValue();
        const Instruction *addr = (Instruction*)op++->getValue();
        file_ << "    Store " << value->name() << " to " << addr->name() << endl;
    }

    void DumpIR::dumpInvoke(const Instruction &instr)
    {
        const Invoke *invoke = (Invoke*)&instr;
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
            const Instruction *in = (Instruction*)op->getValue();
            file_ << in->name() << ' ';
            ++op;
        }
        file_ << ")\n";
    }

    void DumpIR::dumpBranch(const Instruction &instr)
    {
        const Branch *branch = (Branch*)&instr;
        const Instruction *cond = (Instruction*)branch->op_begin()->getValue();
        file_ << "    Branch " << cond->name() << ' '
            << branch->then()->getName() << ' '
            << branch->_else()->getName() << endl;
        //if (branch->hasElseBlock())
    }

    void DumpIR::dumpGoto(const Instruction &instr)
    {
        const Goto *go = (Goto*)&instr;
        file_ << "    Goto " << go->block()->getName() << endl;
    }

    void DumpIR::dumpAssign(const Instruction &instr)
    {
        const Assign *assign = (Assign*)&instr;
        file_ << "    " << assign->name() << " = ";
        if (assign->instance() == Instructions::IR_Value)
            dumpValue((Constant*)assign->op_begin()->getValue());
        else
            dumpInstr(*(Instruction*)assign->op_begin()->getValue());
        file_ << endl;
    }

    void DumpIR::dumpNotOp(const Instruction &instr)
    {
        const NotOp * not = (NotOp*)&instr;
        const Instruction *from = (Instruction*)not->op_begin()->getValue();
        file_ << "    " << not->name() << " = !" << from->name() << endl;
    }

    void DumpIR::dumpReturn(const Instruction &instr)
    {
        const Return *ret = (Return*)&instr;
        const Instruction *from = (Instruction*)ret->op_begin()->getValue();
        file_ << "    Return " << from->name() << endl;
    }

    void DumpIR::dumpReturnVoid(const Instruction &instr)
    {
        file_ << "    ReturnVoid" << endl;
    }

    void DumpIR::dumpBinaryOps(const Instruction &instr)
    {
        const BinaryOperator *ops = (BinaryOperator*)&instr;
        BinaryOps op = ops->op();
        auto iter = ops->op_begin();
        const Instruction *lhs = (Instruction*)iter++->getValue();
        const Instruction *rhs = (Instruction*)iter++->getValue();
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

    void DumpIR::dumpIndex(const Instruction &instr)
    {
        const Index *index = (Index*)&instr;
        auto op = index->op_begin();
        const Instruction *table = (Instruction*)op++->getValue();
        const Instruction *idx = (Instruction*)op++->getValue();
        file_ << "    " << index->name() << " = " << table->name()
            << '[' << idx->name() << ']' << endl;
    }

    void DumpIR::dumpSetIndex(const Instruction &instr)
    {
        const SetIndex *set = (SetIndex*)&instr;
        auto op = set->op_begin();
        const Instruction *table = (Instruction*)op++->getValue();
        const Instruction *idx = (Instruction*)op++->getValue();
        const Instruction *from = (Instruction*)op++->getValue();
        file_ << "    " << table->name() << '[' << idx->name()
            << ']' << " = " << from->name() << endl;
    }

    void DumpIR::dumpPhi(const Instruction &instr)
    {
        const Phi *phi = (Phi*)&instr;
        file_ << "    " << phi->name() << " = Phi<";
        bool first = true;
        for (auto &use = phi->op_begin(); use != phi->op_end(); ++use)
        {
            if (!first)
                file_ << ", ";
            else
                first = false;
            file_ << ((Instruction*)use->getValue())->name();
        }
        file_ << ">\n";
    }

    void DumpIR::dumpCatch(const Instruction &instr)
    {
        const Catch *cat = (Catch*)&instr;
        const Instruction *from = (Instruction*)cat->op_begin()->getValue();
        file_ << "    " << cat->name() << " = Catch from Outside." << from->name() << endl;
    }
}


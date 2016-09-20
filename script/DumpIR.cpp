#include "DumpIR.h"

#include "Value.h"
#include "Instruction.h"
#include "IRModule.h"

using std::endl;

namespace script
{
    DumpIR::DumpIR(std::string &file)
        : filename_(file), file_(file, std::ios::out)
    {
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
    }

    void DumpIR::dump(IRFunction *function)
    {
        file_ << "Define " << function->getFunctionName() << " params: ";
        for (auto i = function->param_begin(), e = function->param_end();
            i != e; ++i)
            file_ << *i << ' ';
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
        file_ << block->getBlockName() << ':' << endl;
        for (auto i = block->instr_begin(), e = block->instr_end();
            i != e; ++i)
            dumpInstr(*i);
    }

    void DumpIR::dumpValue(Value *value)
    {
		if (value == 0)
			return;
        switch (value->get_subclass_id())
        {
        case Value::UndefVal:
        {
            file_ << "[[Undef]]";
            break;
        }
        case Value::ConstantVal:
        {
            dumpConstant((Constant*)value);
            break;
        }
        case Value::TableVal:
        {
            file_ << "[[Table]]";
            break;
        }
        //case Value::FunctionVal:
        //{
        //    file_ << "[Function]:" << ((Function*)value)->getFuncName();
        //    break;
        //}
		case Value::ParamVal:
		{
			file_ << "[Param]:" << ((Param*)value)->getParamName();
			break;
		}
        case Value::InstructionVal:
            file_ << ((Instruction*)value)->get_value_name();
            break;
            //dump((Instruction*)value);
        }
    }

    void DumpIR::dumpInstr(Instruction *instr)
    {
        switch (instr->get_opcode())
        {
        case Instruction::InvokeVal:
            dumpInvoke(instr); break;
        case Instruction::BranchVal:
            dumpBranch(instr); break;
        case Instruction::GotoVal:
            dumpGoto(instr); break;
        case Instruction::AssignVal:
            dumpAssign(instr); break;
        case Instruction::NotOpVal:
            dumpNotOp(instr); break;
        case Instruction::ReturnVal:
            dumpReturn(instr); break;
        case Instruction::ReturnVoidVal:
            dumpReturnVoid(instr); break;
        case Instruction::BinaryOpsVal:
            dumpBinaryOps(instr); break;
        case Instruction::IndexVal:
            dumpIndex(instr); break;
        case Instruction::SetIndexVal:
            dumpSetIndex(instr); break;
        case Instruction::PhiVal:
            dumpPhi(instr); break;
		case Instruction::StoreVal:
			dumpStore(instr); break;
		case Instruction::NewClosureVal:
			dumpNewClosure(instr); break;
        default:
            break;
        }
    }

    void DumpIR::dumpConstant(Constant *cons)
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
        file_ << "    " << invoke->get_value_name() << " = Invoke ";
        dumpValue(invoke->get_func());
        file_ << " (";
        
		size_t op_size = invoke->get_num_operands() - 1;
        for (auto op = invoke->param_begin(); 
            op != invoke->param_end(); ++op)
        {
			dumpValue(op->get_value());
            if (--op_size)
				file_ << ", ";
        }
        file_ << ")\n";
    }

    void DumpIR::dumpBranch(Instruction *instr)
    {
        Branch *branch = (Branch*)instr;
        file_ << "    Branch "; 
        dumpValue(branch->get_cond());
        file_ << ' ' << branch->then()->getBlockName() << ' ';
        file_ << branch->_else()->getBlockName() << endl;
    }

    void DumpIR::dumpGoto(Instruction *instr)
    {
        Goto *go = (Goto*)instr;
        file_ << "    Goto " << go->block()->getBlockName() << endl;
    }

    void DumpIR::dumpAssign(Instruction *instr)
    {
		Assign *assign = (Assign*)instr;
        file_ << "    " << assign->get_value_name() << " = ";
        dumpValue(assign->get_value());
        file_ << endl;
    }

    void DumpIR::dumpNotOp(Instruction *instr)
    {
        NotOp * not_ = (NotOp*)instr;
        file_ << "    " << not_->get_value_name() << " = ! ";
        dumpValue(not_->get_value());
        file_ << endl;
    }

    void DumpIR::dumpReturn(Instruction *instr)
    {
        Return *ret = (Return*)instr;
        file_ << "    Return ";
        dumpValue(ret->get_value());
        file_ << endl;
    }

    void DumpIR::dumpReturnVoid(Instruction *instr)
    {
        file_ << "    ReturnVoid" << endl;
    }

    void DumpIR::dumpBinaryOps(Instruction *instr)
    {
        BinaryOperator *ops = (BinaryOperator*)instr;
        unsigned op = ops->op();
		file_ << "    " << ops->get_value_name() << " = ";
		dumpValue(ops->get_lhs());
        file_ << ' ';
        switch (op)
        {
        case BinaryOperator::Add:
            file_ << '+';
            break;
        case BinaryOperator::Sub:
            file_ << '-';
            break;
        case BinaryOperator::Mul:
            file_ << '*';
            break;
        case BinaryOperator::Div:
            file_ << '/';
            break;
        case BinaryOperator::And:
            file_ << '&';
            break;
        case BinaryOperator::Or:
            file_ << '|';
            break;
        case BinaryOperator::Great:
            file_ << '>';
            break;
        case BinaryOperator::NotLess:
            file_ << ">=";
            break;
        case BinaryOperator::Less:
            file_ << '<';
            break;
        case BinaryOperator::NotGreat:
            file_ << "<=";
            break;
        case BinaryOperator::Equal:
            file_ << "==";
            break;
        case BinaryOperator::NotEqual:
            file_ << "!=";
            break;
        default:
            break;
        }
		file_ << ' ';
		dumpValue(ops->get_rhs());
		file_ << endl;
    }

    void DumpIR::dumpIndex(Instruction *instr)
    {
        Index *index = (Index*)instr;
		file_ << "    " << index->get_value_name() << " = ";
		dumpValue(index->table());
		file_ << '[';
		dumpValue(index->index());
		file_ << ']' << endl;
    }

    void DumpIR::dumpSetIndex(Instruction *instr)
    {
        SetIndex *set = (SetIndex*)instr;
		file_ << "    ";
		dumpValue(set->table());
		file_ << '[';
		dumpValue(set->index());
		file_ << ']' << " = ";
		dumpValue(set->to());
		file_ << endl;
    }

    void DumpIR::dumpPhi(Instruction *instr)
    {
        Phi *phi = (Phi*)instr;
        file_ << "    " << phi->get_value_name() << " = Phi<";
        bool first = true;
        for (auto use = phi->op_begin(); use != phi->op_end(); ++use)
        {
            if (!first)
                file_ << ", ";
            else
                first = false;
            dumpValue(use->get_value());
        }
        file_ << ">\n";
    }

	void DumpIR::dumpStore(Instruction * instr)
	{
		Store *store = static_cast<Store*>(instr);
		file_ << "    Store ";
		dumpValue(store->get_value());
		file_ << " into [Param]:" << store->get_param_name() << endl;
	}

	void DumpIR::dumpNewClosure(Instruction * instr)
	{
		NewClosure *closure = (NewClosure*)instr;
		file_ << "    " << closure->get_value_name() << " = Closure "
			<< closure->get_func_name();
		file_ << " (";

		size_t op_size = closure->get_num_operands();
		for (auto op = closure->param_begin();
			op != closure->param_end(); ++op)
		{
			dumpValue(op->get_value());
			if (--op_size)
				file_ << ", ";
		}
		file_ << ")\n";
	}
}


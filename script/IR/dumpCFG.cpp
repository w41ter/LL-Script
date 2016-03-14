#include "dumpCFG.h"

using std::endl;

namespace script
{
namespace {
    class IsLabel : public QuadVisitor
    {
        IsLabel() = default;
    public:
        static IsLabel &instance()
        {
            static IsLabel tar;
            return tar;
        }

        virtual ~IsLabel() {}
        virtual bool visit(Constant *v) { return false; };
        virtual bool visit(Temp *v) { return false; };
        virtual bool visit(Identifier *v) { return false; };
        virtual bool visit(Array *v) { return false; };
        virtual bool visit(ArrayIndex *v) { return false; };

        virtual bool visit(If *v) { return false; };
        virtual bool visit(Call *v) { return false; };
        virtual bool visit(Goto *v) { return false; };
        virtual bool visit(Copy *v) { return false; };
        virtual bool visit(Load *v) { return false; };
        virtual bool visit(Store *v) { return false; };
        virtual bool visit(Label *v) { return true; };
        virtual bool visit(Param *v) { return false; };
        virtual bool visit(Invoke *v) { return false; };
        virtual bool visit(Return *v) { return false; }; // 
        virtual bool visit(IfFalse *v) { return false; };
        virtual bool visit(Operation *v) { return false; };
        virtual bool visit(AssignArray *v) { return false; };
        virtual bool visit(ArrayAssign *v) { return false; };
    };
}

DumpCFG::DumpCFG(std::fstream & file)
    : DumpQuad(file) 
{
}

bool DumpCFG::visit(Label * v)
    {
        Quad *tar = (*labelTarget_)[v];
        BasicBlock *block = head_[tar];
        if (block == nullptr) return false;
        dumpName(block->getID()); file_ << "(" << v->name_ << ")";
        return false;
    }

    void DumpCFG::dump(IRModule & module)
    {
        if (!module.isCFGForm())
            return;
        DumpQuad::dump(module);
    }

    void DumpCFG::dumpFunction(IRFunction * func)
    {
        CFG *cfg = func->getCFG();
        file_ << "function $" << func->getName() << " from "
            << func->getName() << "_bb_" << cfg->start_->getID() << " to "
            << func->getName() << "_bb_" << cfg->end_->getID() << ":\n";
        file_ << "\tparams: ";
        for (auto &i : func->getParams())
        {
            file_ << '$' << i << ' ';
        }
        file_ << endl;
        module_ = func->getName();
        dumpCode(func);
        module_ = "anonymous";
    }

    void DumpCFG::dumpCode(IRCode * code)
    {
        CFG *cfg = code->getCFG();
        labelTarget_ = &(cfg->labelTarget_);
        for (auto i : cfg->blocks_)
        {
            head_[i->head_] = i;
        }

        for (auto i : cfg->blocks_)
            dumpBasicBlock(i);
    }

    void DumpCFG::dumpBasicBlock(BasicBlock * block)
    {
        file_ << "begin "; dumpName(block->getID()); file_ << endl;
        file_ << "\tprecursors:" << endl;
        for (auto i : block->precursors_)
        {
            file_ << "\t\t"; dumpName(i->getID()); file_ << endl;
        }
        file_ << "\tsuccessors:" << endl;
        for (auto i : block->successors_)
        {
            file_ << "\t\t"; dumpName(i->getID()); file_ << endl;
        }
        file_ << "{" << endl;
        Quad *begin = block->head_;
        do {
            if (begin == nullptr) break;
            if (!begin->accept(&IsLabel::instance()))
                begin->accept(this), file_ << endl;
            begin = begin->next_;
        } while (begin != block->end_->next_);
        file_ << "} " << endl << endl; //end block@_" << block->getID() << endl << endl;
    }

    void DumpCFG::dumpName(unsigned id)
    {
        file_ << module_ << "_bb_" << id << ' ';
    }
}

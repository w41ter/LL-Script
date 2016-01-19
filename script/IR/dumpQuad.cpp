#include <iostream>
#include <list>
#include "../Parser/lexer.h"
#include "dumpQuad.h"
#include "IRGenerator.h"

using std::endl;

namespace script
{
    void DumpQuad::dump(IRModule &module)
    {
        // std::cout << "begin dump functions" << endl;
        for (auto &i : module.functions_)
            dumpFunction(i.second);
        // std::cout << "begin dump all define" << endl;
        dumpCode(&module);
        // std::cout << "all dump to file" << endl;
    }

    bool DumpQuad::visit(Constant * v)
    {
        switch (v->type_)
        {
        case Constant::T_Character: file_ << '\'' << v->c_ << '\''; break;
        case Constant::T_Float: file_ << v->fnum_; break;
        case Constant::T_Integer: file_ << v->num_; break;
        case Constant::T_String: file_ << '"' << v->str_ << '"'; break;
        }
        file_ << ' ';
        return false;
    }

    bool DumpQuad::visit(Temp * v)
    {
        file_ << v->name_ << ' ';
        return false;
    }

    bool DumpQuad::visit(Identifier * v)
    {
        file_ << "$" << v->name_ << ' ';
        return false;
    }

    bool DumpQuad::visit(Array * v)
    {
        file_ << "new [" << v->total_ << "] ";
        return false;
    }

    bool DumpQuad::visit(ArrayIndex * v)
    {
        v->value_->accept(this);
        file_ << "[ ";
        v->index_->accept(this);
        file_ << "] ";
        return false;
    }

    bool DumpQuad::visit(If * v)
    {
        file_ << '\t' << "if ";
        v->condition_->accept(this);
        v->label_->accept(this);
        return false;
    }

    bool DumpQuad::visit(Call * v)
    {
        // std::cout << "dump call" << std::endl;
        file_ << '\t';
        v->result_->accept(this);
        file_ << " = call ";
        // std::cout << "\tdump call position before:" << v->position_ << std::endl;
        //v->position_->accept(this);
        file_ << "$" << v->name_ << ' ';
        // std::cout << "\tdump call position end" << std::endl;
        file_ << v->num_;
        return false;
    }

    bool DumpQuad::visit(Goto * v)
    {
        file_ << '\t' << "goto ";
        v->label_->accept(this);
        return false;
    }

    bool DumpQuad::visit(Copy * v)
    {
        file_ << '\t';
        v->dest_->accept(this);
        file_ << "= ";
        v->sour_->accept(this);
        return false;
    }

    bool DumpQuad::visit(Load * v)
    {
        file_ << "\tload ";
        v->id_->accept(this);
        v->result_->accept(this);
        return false;
    }

    bool DumpQuad::visit(Store * v)
    {
        file_ << "\tstore ";
        v->id_->accept(this);
        v->result_->accept(this);
        return false;
    }

    bool DumpQuad::visit(Label * v)
    {
        file_ << v->name_ << ": ";
        return false;
    }

    bool DumpQuad::visit(Param * v)
    {
        file_ << "\tparam ";
        v->Value_->accept(this);
        return false;
    }

    bool DumpQuad::visit(Invoke * v)
    {
        file_ << '\t';
        v->result_->accept(this);
        file_ << "= invoke ";
        v->name_->accept(this);
        file_ << v->num_;
        return false;
    }

    bool DumpQuad::visit(Return * v)
    {
        file_ << "\treturn ";
        v->arg_->accept(this);
        return false;
    }

    bool DumpQuad::visit(IfFalse * v)
    {
        file_ << '\t' << "if_false ";
        v->condition_->accept(this);
        v->label_->accept(this);
        return false;
    }

    void DumpQuad::dumpFunction(IRFunction * func)
    {
        file_ << "funtion $" << func->name_ << " from ";
        func->begin_->accept(this);
        file_ << "to ";
        func->end_->accept(this);
        file_ << endl;
        dumpCode(func);
    }

    void DumpQuad::dumpCode(IRCode * code)
    {
        Quad *c = code->context_.begin();
        while (c != nullptr)
        {
            c->accept(this);
            file_ << endl;
            c = c->next_;
        }
        file_ << endl;
    }

    bool DumpQuad::visit(Operation * v)
    {
        file_ << '\t';
        v->result_->accept(this);
        file_ << "= ";
        if (v->left_ != nullptr)
            v->left_->accept(this);

        switch (v->op_)
        {
        case TK_Plus: file_ << "+ "; break;
        case TK_Sub:  file_ << "- "; break;
        case TK_Mul:  file_ << "* "; break;
        case TK_Div:  file_ << "/ "; break;
        case TK_Not:  file_ << "! "; break;
        case TK_Less: file_ << "< "; break;
        case TK_Great: file_ << "> "; break;
        case TK_LessThan: file_ << "<= "; break;
        case TK_NotEqual: file_ << "!= "; break;
        case TK_GreatThan: file_ << ">= "; break;
        case TK_EqualThan: file_ << "== "; break;
        }
        v->right_->accept(this);
        return false;
    }

    bool DumpQuad::visit(AssignArray * v)
    {
        return false;
    }

    bool DumpQuad::visit(ArrayAssign * v)
    {
        return false;
    }

}

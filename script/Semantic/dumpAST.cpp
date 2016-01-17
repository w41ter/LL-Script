#include <iostream>

#include "dumpAST.h"

using std::endl;

namespace script
{
    bool DumpAST::visit(ASTExpressionList *v)
    {
        int index = v->exprs_.size();
        for (auto &i : v->exprs_)
        {
            i->accept(this);
            if (--index)
                file_ << " ";
        }
        return false;
    }

    bool DumpAST::visit(ASTIdentifier *v)
    {
        file_ << v->name_ << " ";
        return false;
    }

    bool DumpAST::visit(ASTNull *v)
    {
        file_ << "null ";
        return false;
    }

    bool DumpAST::visit(ASTConstant *v)
    {
        switch (v->type_)
        {
        case ASTConstant::T_Float: file_ << v->fnum_; break;
        case ASTConstant::T_String: file_ << "\"" << v->str_ << "\""; break;
        case ASTConstant::T_Integer: file_ << v->num_; break;
        case ASTConstant::T_Character:
        {
            switch (v->c_)
            {
            case '\n': file_ << "'\\n'"; break;
            case '\r': file_ << "'\\r'"; break;
            case '\t': file_ << "'\\t'"; break;
            case '\'': file_ << "'\\''"; break;
            default:
                file_ << '\'' << v->c_ << '\''; break;
            }
            break;
        }
        }
        file_ << " ";
        return false;
    }

    bool DumpAST::visit(ASTArray *v)
    {
        file_ << "[ ";
        v->array_->accept(this);
        file_ << "] ";
        return false;
    }

    bool DumpAST::visit(ASTCall *v)
    {
        file_ << "("; 
        v->function_->accept(this);
        file_ << "( ";
        v->arguments_->accept(this);
        file_ << ") ) ";
        return false;
    }

    bool DumpAST::visit(ASTArrayIndex *v)
    {
        v->array_->accept(this);
        file_ << "[ ";
        v->index_->accept(this);
        file_ << "] ";
        return false;
    }

    bool DumpAST::visit(ASTSingleExpression *v)
    {
        file_ << "(" << getOP(v->op_) << " ";
        v->expr_->accept(this);
        file_ << ") ";
        return false;
    }

    bool DumpAST::visit(ASTBinaryExpression *v)
    {
        file_ << "(" << getOP(v->op_) << " ";
        v->left_->accept(this);
        v->right_->accept(this);
        file_ << ") ";
        return false;
    }

    bool DumpAST::visit(ASTRelationalExpression *v)
    {
        file_ << "(" << getOP(v->relation_) << " ";
        v->left_->accept(this);
        v->right_->accept(this);
        file_ << ") ";
        return false;
    }

    bool DumpAST::visit(ASTAndExpression *v)
    {
        file_ << "(and ";
        for (auto &i : v->relations_)
            i->accept(this);
        file_ << ") ";
        return false;
    }

    bool DumpAST::visit(ASTOrExpression *v)
    {
        file_ << "(or ";
        for (auto &i : v->relations_)
            i->accept(this);
        file_ << ") ";
        return false;
    }

    bool DumpAST::visit(ASTAssignExpression *v)
    {
        file_ << "(set ";
        v->left_->accept(this);
        v->right_->accept(this);
        file_ << ") ";
        return false;
    }

    bool DumpAST::visit(ASTVarDeclStatement *v)
    {
        printIndent();
        file_ << "(let " << v->name_ << " "; 
        v->expr_->accept(this);
        file_ << ")" << endl;
        return false;
    }

    bool DumpAST::visit(ASTContinueStatement *v)
    {
        printIndent();
        file_ << "(continue)" << endl;
        return false;
    }

    bool DumpAST::visit(ASTBreakStatement *v)
    {
        printIndent();
        file_ << "(break)" << endl;
        return false;
    }

    bool DumpAST::visit(ASTReturnStatement *v)
    {
        printIndent();
        file_ << "(return ";
        if (v->expr_.get() != nullptr)
            v->expr_->accept(this);
        file_ << ")" << endl;
        return false;
    }

    bool DumpAST::visit(ASTWhileStatement *v)
    {
        printIndent();
        file_ << "(while "; v->condition_->accept(this);
        file_ << endl;
        indent();
        v->statement_->accept(this);
        printIndent(); file_ << ") " << endl;
        decdent();
        return false;
    }

    bool DumpAST::visit(ASTIfStatement *v)
    {
        printIndent();
        file_ << "(if "; v->condition_->accept(this);
        file_ << endl;
        indent();
        v->ifStatement_->accept(this); 
        if (v->hasElse())
        {
            v->elseStatement_->accept(this); 
        }
        printIndent(); file_ << ") " << endl;
        decdent();
        return false;
    }

    bool DumpAST::visit(ASTBlock *v)
    {
        printIndent();
        file_ << "(begin " << endl;
        indent();
        for (auto &i : v->statements_)
            i->accept(this);
        printIndent();
        file_ << ")" << endl;
        decdent();
        return false;
    }

    bool DumpAST::visit(ASTFunction *v)
    {
        file_ << "(function ";
        v->prototype_->accept(this);
        indent();
        v->block_->accept(this);
        printIndent();
        file_ << ")" << endl;
        decdent();
        return false;
    }

    bool DumpAST::visit(ASTProgram *v)
    {
        file_ << "# lambda defination:" << endl;
        for (auto &i : v->function_)
            i->accept(this), file_ << endl;

        file_ << endl << endl;
        file_ << "# variable defination:" << endl;
        for (auto &i : v->defines_)
            i->accept(this), file_ << endl;

        return false;
    }

    bool DumpAST::visit(ASTPrototype *v)
    {
        file_ << v->name_ << " ( ";
        for (auto &i : v->args_)
            file_ << i << " ";
        file_ << ") " << endl;
        return false;
    }

    bool DumpAST::visit(ASTClosure *v)
    {
        file_ << "(closure (" << v->name_;
        for (auto &i : v->params_)
            file_ << " " << i;
        file_ << ") ) ";
        return false;
    }

    bool DumpAST::visit(ASTDefine *v)
    {
        file_ << "(define " << v->name_ << ' ';
        v->expr_->accept(this);
        file_ << ") " << endl;
        return false;
    }

    bool DumpAST::visit(ASTStatement * v)
    {
        printIndent();
        file_ << "(";
        v->tree_->accept(this);
        file_ << ")" << endl;
        return false;
    }

    std::string DumpAST::getOP(unsigned op)
    {
        std::string str;
        switch (op)
        {
        case TK_Less: str = "<"; break;
        case TK_LessThan: str = "<="; break;
        case TK_Great: str = ">"; break;
        case TK_GreatThan: str = ">="; break;
        case TK_EqualThan: str = "=="; break;
        case TK_NotEqual: str = "!="; break;
        case TK_Plus: str = "+"; break;
        case TK_Sub: str = "-"; break;
        case TK_Mul: str = "*"; break;
        case TK_Div: str = "/"; break;
        case TK_Not: str = "not"; break;
        default: str = "<unknow>"; break;
        }
        return str;
    }
}

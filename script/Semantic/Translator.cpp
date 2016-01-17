#include <sstream>

#include "../Parser/lexer.h"


#include "Translator.h"

namespace script
{
    bool Translator::visit(ASTIfStatement * v)
    {
        return false;
    }

    bool Translator::visit(ASTBlock * v)
    {
        return false;
    }

    bool Translator::visit(ASTFunction * v)
    {
        return false;
    }

    bool Translator::visit(ASTProgram * v)
    {
        return false;
    }

    bool Translator::visit(ASTConstant * v)
    {
        return false;
    }

    bool Translator::visit(ASTSingleExpression * v)
    {
        v->expr_->accept(this);
        switch (v->op_)
        {
        case TK_Sub:
            result_ = module_->createSingleSub(result_);
            break;
        case TK_Not:
            result_ = module_->createNot(result_);
            break;
        }
        return false;
    }

    bool Translator::visit(ASTBinaryExpression * v)
    {
        v->left_->accept(this);
        Quad *left = result_;
        v->right_->accept(this);
        Quad *right = result_;

        Quad *value = nullptr;
        switch (v->op_)
        {
        case TK_Plus:
            value = module_->createAdd(left, right);
            break;
        case TK_Sub:
            value = module_->createSub(left, right);
            break;
        case TK_Mul:
            value = module_->createMul(left, right);
            break;
        case TK_Div:
            value = module_->createDiv(left, right);
            break;
        }
        result_ = value;
        return false;
    }

    bool Translator::visit(ASTRelationalExpression * v)
    {
        v->left_->accept(this);
        Quad *left = result_;
        v->right_->accept(this);
        Quad *right = result_;

        Quad *value = nullptr;
        switch (v->relation_)
        {
        case TK_Great:
            value = module_->createAdd(left, right);
            break;
        case TK_GreatThan:
            value = module_->createSub(left, right);
            break;
        case TK_Less:
            value = module_->createMul(left, right);
            break;
        case TK_LessThan:
            value = module_->createDiv(left, right);
            break;
        case TK_EqualThan:
            value = module_->createEqualThan(left, right);
            break;
        case TK_NotEqual:
            value = module_->createNotEqual(left, right);
            break;
        }
        result_ = value;
        return false;
    }
}


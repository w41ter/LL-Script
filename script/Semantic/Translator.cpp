#include <sstream>

#include "Translator.h"
#include "../Parser/lexer.h"

namespace script
{
    bool Translator::visit(ASTExpressionList *v)
    {
        return false;
    }

    bool Translator::visit(ASTIdentifier *v)
    {
        result_ = gen_->CreateValue<Identifier>(v->name_);//(*symbols_)[v->name_];
        return false;
    }

    bool Translator::visit(ASTNull *v)
    {
        return false;
    }

    bool Translator::visit(ASTConstant *v)
    {
        switch (v->type_)
        {
        case ASTConstant::T_Character:
            result_ = gen_->CreateValue<Constant>(v->c_);
            break;
        case ASTConstant::T_Float:
            result_ = gen_->CreateValue<Constant>(v->fnum_);
            break;
        case ASTConstant::T_Integer:
            result_ = gen_->CreateValue<Constant>(v->num_);
            break;
        case ASTConstant::T_String:
            result_ = gen_->CreateValue<Constant>(v->str_);
            break;
        }
        return false;
    }

    bool Translator::visit(ASTArray *v)
    {
        auto *exprList = static_cast<ASTExpressionList*>(&*(v->array_));
        auto *array = gen_->CreateValue<Array>(exprList->exprs_.size());
        Value *temp = gen_->CreateValue<Temp>();
        gen_->insertCopy(array, temp);
        int index = 0;
        for (auto &i : exprList->exprs_)
        {
            i->accept(this);
            Value *t = gen_->CreateValue<ArrayIndex>(
                temp, gen_->CreateValue<Constant>(index++));
            gen_->insertStore(t, loadValue(result_));
        }
        result_ = temp;
        return false;
    }

    bool Translator::visit(ASTCall *v)
    {
        auto *exprList = static_cast<ASTExpressionList*>(&*(v->arguments_));
        for (auto &i : exprList->exprs_)
        {
            i->accept(this);
            gen_->insertParam(loadValue(result_));
        }
        v->function_->accept(this);
        auto *result = gen_->CreateValue<Temp>();
        gen_->insertInvoke(loadValue(result_), result, exprList->exprs_.size());
        result_ = result;
        return false;
    }

    bool Translator::visit(ASTArrayIndex *v)
    {
        v->array_->accept(this);
        Value *array = loadValue(result_);
        v->index_->accept(this);
        Value *index = loadValue(result_);
        result_ = gen_->CreateValue<ArrayIndex>(array, index);
        return false;
    }

    bool Translator::visit(ASTSingleExpression *v)
    {
        unsigned op = v->op_;
        v->expr_->accept(this);
        Value *result = gen_->CreateValue<Temp>();
        gen_->insertSingle(op, loadValue(result_), result);
        result_ = result;
        return false;
    }

    bool Translator::visit(ASTBinaryExpression *v)
    {
        unsigned op = v->op_;
        v->left_->accept(this);
        Value *left = loadValue(result_);
        v->right_->accept(this);
        Value *result = gen_->CreateValue<Temp>();
        gen_->insertOperation(op, left, loadValue(result_), result);
        result_ = result;
        return false;
    }

    bool Translator::visit(ASTRelationalExpression *v)
    {
        unsigned op = v->relation_;
        v->left_->accept(this);
        Value *left = loadValue(result_);
        v->right_->accept(this);
        Value *result = gen_->CreateValue<Temp>();
        gen_->insertOperation(op, left, loadValue(result_), result);
        result_ = result;
        return false;
    }

    bool Translator::visit(ASTAndExpression *v)
    {
        Label *end = gen_->Create<Label>();
        Label *false_ = gen_->Create<Label>();
        Temp *temp = gen_->CreateValue<Temp>();
        for (auto &i : v->relations_)
        {
            i->accept(this);
            gen_->insertIfFalse(loadValue(result_), false_);
        }
        gen_->insertCopy(gen_->CreateValue<Constant>(1.0f), temp);
        gen_->insertGoto(end);
        gen_->insertLabel(false_);
        gen_->insertCopy(gen_->CreateValue<Constant>(0.0f), temp);
        gen_->insertLabel(end);
        result_ = temp;
        return false;
    }

    bool Translator::visit(ASTOrExpression *v)
    {
        Label *end = gen_->Create<Label>();
        Label *true_ = gen_->Create<Label>();
        Temp *temp = gen_->CreateValue<Temp>();
        for (auto &i : v->relations_)
        {
            i->accept(this);
            gen_->insertIf(loadValue(result_), true_);
        }
        gen_->insertCopy(gen_->CreateValue<Constant>(0.0f), temp);
        gen_->insertGoto(end);
        gen_->insertLabel(true_);
        gen_->insertCopy(gen_->CreateValue<Constant>(1.0f), temp);
        gen_->insertLabel(end);
        result_ = temp;
        return false;
    }

    bool Translator::visit(ASTAssignExpression *v)
    {
        v->left_->accept(this);
        Value *value = loadValue(result_);
        v->right_->accept(this);

        // 
        // generate store if left is variable
        // 
        if (value->isVariable())
            gen_->insertStore(value, result_);
        else
            gen_->insertCopy(value, result_);
        // var_ = var_
        return false;
    }

    bool Translator::visit(ASTVarDeclStatement *v)
    {
        v->expr_->accept(this);
        Identifier *name = gen_->CreateValue<Identifier>(v->name_);
        gen_->insertStore(name, loadValue(result_));
        result_ = nullptr;
        return false;
    }

    bool Translator::visit(ASTContinueStatement *v)
    {
        gen_->insertGoto(breakLabels_.top());
        result_ = nullptr;
        return false;
    }

    bool Translator::visit(ASTBreakStatement *v)
    {
        gen_->insertGoto(continueLabels_.top());
        result_ = nullptr;
        return false;
    }

    bool Translator::visit(ASTReturnStatement *v)
    {
        if (v->expr_ != nullptr)
            v->expr_->accept(this);
        else
            result_ = gen_->CreateValue<Temp>();
        gen_->insertReturn(loadValue(result_));
        result_ = nullptr;
        return false;
    }

    bool Translator::visit(ASTWhileStatement *v)
    {
        Label *begin = gen_->Create<Label>();
        Label *end = gen_->Create<Label>();

        gen_->insertLabel(begin);

        v->condition_->accept(this);
        gen_->insertIfFalse(loadValue(result_), end);

        breakLabels_.push(begin);
        continueLabels_.push(end);
        v->statement_->accept(this);
        breakLabels_.pop();
        continueLabels_.pop();

        gen_->insertLabel(end);
        result_ = nullptr;
        return false;
    }

    bool Translator::visit(ASTIfStatement *v)
    {
        Label *end = gen_->Create<Label>();
        Label *else_ = v->hasElse() ? gen_->Create<Label>() : end;

        v->condition_->accept(this);
        gen_->insertIfFalse(loadValue(result_), else_);

        v->ifStatement_->accept(this);
        gen_->insertGoto(end);

        if (v->hasElse())
        {
            gen_->insertLabel(else_);
            v->elseStatement_->accept(this);
        }

        gen_->insertLabel(end);
        result_ = nullptr;
        return false;
    }

    bool Translator::visit(ASTBlock *v)
    {
        for (auto &i : v->statements_)
            i->accept(this);
        return false;
    }

    bool Translator::visit(ASTFunction *v)
    {
        Label *begin = gen_->Create<Label>();
        Label *end = gen_->Create<Label>();
        
        std::map<std::string, Identifier*> *origin = symbols_;
        symbols_ = new std::map<std::string, Identifier*>();

        QuadFunction *function = 
            module_.createFunction(v->prototype_->name_, begin, end);

        function_[v->prototype_->name_] = begin;

        // set generator
        auto *gen = gen_; gen_ = function->getGenerator();

        gen_->insertLabel(begin);
        v->prototype_->accept(this);
        v->block_->accept(this);
        gen_->insertLabel(end);

        // reset generaor
        gen_ = gen;

        delete symbols_;
        result_ = nullptr;
        return false;
    }

    bool Translator::visit(ASTProgram *v)
    {
        for (auto &i : v->function_)
            i->accept(this);

        Label *begin = gen_->Create<Label>();
        Label *end = gen_->Create<Label>();

        gen_->insertLabel(begin);
        for (auto &i : v->defines_)
            i->accept(this);
        gen_->insertLabel(end);

        return false;
    }

    bool Translator::visit(ASTClosure * v)
    {
        for (auto &i : v->params_)
            gen_->insertParam(gen_->CreateValue<Identifier>(v->name_)); //(*symbols_)[i]);

        auto *result = gen_->CreateValue<Temp>();
        gen_->insertCall(function_[v->name_], result, v->params_.size());
        result_ = result;
        return false;
    }

    bool Translator::visit(ASTStatement * v)
    {
        v->tree_->accept(this);
        result_ = nullptr;
        return false;
    }

    bool Translator::visit(ASTPrototype * v)
    {
        for (auto &i : v->args_)
            (*symbols_)[i] = gen_->CreateValue<Identifier>(i);
        return false;
    }

    bool Translator::visit(ASTDefine * v)
    {
        v->expr_->accept(this);
        Identifier *name = gen_->CreateValue<Identifier>(v->name_);
        (*symbols_)[v->name_] = name;
        gen_->insertStore(name, loadValue(result_));
        result_ = nullptr;
        return false;
    }

    Value * Translator::loadValue(Value * value)
    {
        if (value->isVariable())
        {
            Value *temp = gen_->CreateValue<Temp>();
            gen_->insertLoad(value, temp);
            value = temp;
        }
        return value;
    }
}

#include <sstream>

#include "Translator.h"

namespace script
{
    bool Translator::visit(ASTExpressionList & v)
    {
        return false;
    }

    bool Translator::visit(ASTIdentifier & v)
    {
        var_ = MallocQuad<Identifier>(getName(v.name_));
        return false;
    }

    bool Translator::visit(ASTNull & v)
    {
        return false;
    }

    bool Translator::visit(ASTConstant & v)
    {
        switch (v.type_)
        {
        case ASTConstant::T_Charactor:
            var_ = MallocQuad<Character>(v.c_);
            break;
        case ASTConstant::T_Float:
            var_ = MallocQuad<Float>(v.fnum_);
            break;
        case ASTConstant::T_Integer: 
            var_ = MallocQuad<Integer>(v.num_);
            break;
        case ASTConstant::T_String: 
            var_ = MallocQuad<String>(v.str_);
            break;
        }
        return false;
    }

    bool Translator::visit(ASTArray & v)
    {
        auto *exprList = static_cast<ASTExpressionList*>(v.array_);
        auto *array = MallocQuad<Array>(exprList->exprs_.size());
        int index = 0;
        for (auto i : exprList->exprs_)
        {
            i->accept(*this);
            Var *result = MallocQuad<Temp>();
            quads_.push_back(MallocQuad<Copy>(var_, result));
            quads_.push_back(MallocQuad<ArrayAssign>(
                result,
                MallocQuad<Integer>(index++),
                array));
        }
        var_ = array;
        return false;
    }

    bool Translator::visit(ASTCall & v)
    {
        auto *exprList = static_cast<ASTExpressionList*>(v.arguments_);
        for (auto i : exprList->exprs_)
        {
            i->accept(*this);
            quads_.push_back(MallocQuad<Param>(var_));
        }
        v.function_->accept(*this);
        auto *result = MallocQuad<Temp>();
        quads_.push_back(MallocQuad<Call>(
            var_,
            result,
            exprList->exprs_.size()
            ));
        var_ = result;
        return false;
    }

    bool Translator::visit(ASTArrayIndex & v)
    {
        v.array_->accept(*this);
        auto *array = var_;
        v.index_->accept(*this);
        auto *index = var_;
        Var *result = MallocQuad<Temp>();
        quads_.push_back(MallocQuad<AssignArray>(array, index, result));
        var_ = result;
        return false;
    }

    bool Translator::visit(ASTSingleExpression & v)
    {
        SingleAssign::OP op = v.op_ == ASTSingleExpression::OP_Not
            ? SingleAssign::OP::NOT
            : SingleAssign::OP::NAGTIVE;
        v.expr_->accept(*this);
        auto *result = MallocQuad<Temp>();
        quads_.push_back(MallocQuad<SingleAssign>(op, var_, result));
        var_ = result;
        return false;
    }

    bool Translator::visit(ASTBinaryExpression & v)
    {
        unsigned op = 0;
        switch (v.op_)
        {
        case ASTBinaryExpression::OP_Add:
            op = BinaryAssign::OP::ADD;
            break;
        case ASTBinaryExpression::OP_Sub:
            op = BinaryAssign::OP::SUB;
            break;
        case ASTBinaryExpression::OP_Mul:
            op = BinaryAssign::OP::MUL;
            break;
        case ASTBinaryExpression::OP_Div:
            op = BinaryAssign::OP::DIV;
            break;
        }
        v.left_->accept(*this);
        auto *left = var_;
        v.right_->accept(*this);
        auto *result = MallocQuad<Temp>();
        quads_.push_back(MallocQuad<BinaryAssign>(
            op, left, var_, result
            ));
        var_ = result;
        return false;
    }

    bool Translator::visit(ASTRelationalExpression & v)
    {
        unsigned op = 0;
        switch (v.relation_)
        {
        case ASTRelationalExpression::RL_Equal:
            op = RelopAssign::E;
            break;
        case ASTRelationalExpression::RL_Great:
            op = RelopAssign::G;
            break;
        case ASTRelationalExpression::RL_NotEqual:
            op = RelopAssign::NE;
            break;
        case ASTRelationalExpression::RL_GreatThan:
            op = RelopAssign::GT;
            break;
        case ASTRelationalExpression::RL_Less:
            op = RelopAssign::L;
            break;
        case ASTRelationalExpression::RL_LessThan:
            op = RelopAssign::LT;
            break;
        }
        v.left_->accept(*this);
        auto *left = var_;
        v.right_->accept(*this);
        auto *result = MallocQuad<Temp>();
        quads_.push_back(MallocQuad<RelopAssign>(
            op, left, var_, result
            ));
        var_ = result;
        return false;
    }

    bool Translator::visit(ASTAndExpression & v)
    {
        Label *end = MallocQuad<Label>();
        Label *false_ = MallocQuad<Label>();
        Temp *temp = MallocQuad<Temp>();
        for (auto i : v.relations_)
        {
            i->accept(*this);
            quads_.push_back(MallocQuad<IfFalse>(var_, false_));
        }
        quads_.push_back(MallocQuad<Copy>(MallocQuad<Integer>(1), temp));
        quads_.push_back(MallocQuad<Goto>(end));
        quads_.push_back(false_);
        quads_.push_back(MallocQuad<Copy>(MallocQuad<Integer>(0), temp));
        quads_.push_back(end);
        var_ = temp;
        return false;
    }

    bool Translator::visit(ASTOrExpression & v)
    {
        Label *end = MallocQuad<Label>();
        Label *true_ = MallocQuad<Label>();
        Temp *temp = MallocQuad<Temp>();
        for (auto i : v.relations_)
        {
            i->accept(*this);
            quads_.push_back(MallocQuad<If>(var_, true_));
        }
        quads_.push_back(MallocQuad<Copy>(MallocQuad<Integer>(0), temp));
        quads_.push_back(MallocQuad<Goto>(end));
        quads_.push_back(true_);
        quads_.push_back(MallocQuad<Copy>(MallocQuad<Integer>(1), temp));
        quads_.push_back(end);
        var_ = temp;
        return false;
    }

    bool Translator::visit(ASTAssignExpression & v)
    {
        v.left_->accept(*this);
        Var *var = var_;
        v.right_->accept(*this);
        quads_.push_back(MallocQuad<Copy>(var, var_));
        // var_ = var_
        return false;
    }

    bool Translator::visit(ASTVarDeclStatement & v)
    {
        v.expr_->accept(*this);

        auto *i = MallocQuad<Identifier>(getName(v.name_));
        quads_.push_back(MallocQuad<Copy>(var_, i));
        return false;
    }

    bool Translator::visit(ASTContinueStatement & v)
    {
        quads_.push_back(MallocQuad<Goto>(breakLabels_.top()));
        return false;
    }

    bool Translator::visit(ASTBreakStatement & v)
    {
        quads_.push_back(MallocQuad<Goto>(continueLabels_.top()));
        return false;
    }

    bool Translator::visit(ASTReturnStatement & v)
    {
        if (v.expr_ != nullptr)
            v.expr_->accept(*this);
        else
            var_ = MallocMemory<Temp>();
        quads_.push_back(MallocQuad<Return>(var_));
        return false;
    }

    bool Translator::visit(ASTWhileStatement & v)
    {
        Label *begin = MallocQuad<Label>();
        Label *end = MallocQuad<Label>();
        quads_.push_back(begin);
        v.condition_->accept(*this);
        quads_.push_back(MallocQuad<IfFalse>(var_, end));
        breakLabels_.push(begin);
        continueLabels_.push(end);
        v.statement_->accept(*this);
        quads_.push_back(end);
        breakLabels_.pop();
        continueLabels_.pop();
        return false;
    }

    bool Translator::visit(ASTIfStatement & v)
    {
        Label *end = MallocQuad<Label>();
        Label *else_ = v.hasElse() ? MallocQuad<Label>() : end;
        v.condition_->accept(*this);
        quads_.push_back(MallocQuad<IfFalse>(var_, else_));
        v.ifStatement_->accept(*this);
        quads_.push_back(MallocQuad<Goto>(end));
        if (v.hasElse())
        {
            quads_.push_back(else_);
            v.elseStatement_->accept(*this);
        }
        quads_.push_back(end);
        return false;
    }

    bool Translator::visit(ASTBlock & v)
    {
        auto *table = table_;
        table_ = v.table_;
        for (auto i : v.statements_)
            i->accept(*this);
        table_ = table;
        return false;
    }

    bool Translator::visit(ASTFunction & v)
    {
        Label *begin = MallocQuad<Label>();
        Label *end = MallocQuad<Label>();
        Function *func = MallocQuad<Function>(v.name_, begin, end);
        quads_.push_back(func);
        quads_.push_back(begin);
        v.block_->accept(*this);
        quads_.push_back(end);
        return false;
    }

    bool Translator::visit(ASTProgram & v)
    {
        table_ = v.table_;
        for (auto i : v.functions_)
            i->accept(*this);
        return false;
    }

    std::string Translator::getName(std::string & str)
    {
        std::stringstream stream(str);
        stream << "@in_" << table_->find(str);
        std::string name;
        stream >> name;
        return name;
    }
}


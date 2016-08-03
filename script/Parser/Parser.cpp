#include "Parser.h"

#include <iostream>
#include <sstream>

#include "../IR/CFG.h"
#include "../IR/IRContext.h"
#include "../IR/IRModule.h"
#include "../IR/SymbolTable.h"
#include "../IR/Instruction.h"
#include "../Semantic/Diagnosis.h"
#include "../Semantic/DiagnosisConsumer.h"

using std::map;
using std::set;
using std::string;
using std::vector;
using script::ir::Value;

namespace script
{

namespace
{
    std::string getTmpName(std::string name = "Tmp")
    {
        static int count = 0;
        std::stringstream stream;
        stream << name << count;
        std::string str;
        stream >> str;
        return str;
    }

    ir::BinaryOps TranslateRelationalToBinaryOps(unsigned tok)
    {
        using ir::BinaryOps;
        switch (tok)
        {
        case TK_Less:
            return BinaryOps::BO_Less;
        case TK_LessThan:
            return BinaryOps::BO_NotGreat;
        case TK_Great:
            return BinaryOps::BO_Great;
        case TK_GreatThan:
            return BinaryOps::BO_NotLess;
        case TK_NotEqual:
            return BinaryOps::BO_NotEqual;
        case TK_EqualThan:
            return BinaryOps::BO_Equal;
        default:
            break;
        }
    }

    void LoadParamsIntoTable(
        SymbolTable *table, 
        IRContext *context,
        BasicBlock *allocBlock,
        std::vector<std::pair<std::string, Token>> &params)
    {
        for (auto &i : params)
        {
            table->insertVariables(i.first, i.second);
            Value *value = context->create<ir::Alloca>(i.first, allocBlock);
            table->bindValue(i.first, value);
        }
    }
}

    bool Parser::isRelational(unsigned tok)
    {
        return (tok == TK_Less || tok == TK_LessThan ||
            tok == TK_Great || tok == TK_GreatThan ||
            tok == TK_NotEqual || tok == TK_EqualThan);
    }

    std::string Parser::getTempIDName(const char *name)
    {
        static int index = 0;
        std::stringstream stream;
        stream << name << "@" << index++;
        string str;
        stream >> str;
        return str;
    }

    void Parser::initialize()
    {
        map<string, unsigned> keywords = {
            { "if", TK_If },
            { "let", TK_Let },
            { "else", TK_Else },
            { "true", TK_True },
            { "null", TK_Null },
            { "false", TK_False },
            { "while", TK_While },
            { "break", TK_Break },
            { "return", TK_Return },
            { "continue", TK_Continue },
            { "function", TK_Function },
            { "lambda", TK_Lambda },
            { "define", TK_Define }
        };
        for (auto i : keywords)
        {
            lexer_.registerKeyword(i.first, i.second);
        }
    }

    Value *Parser::factor()
    {
        switch (token_.kind_)
        {
        case TK_LitFloat:
        {
            float value = token_.fnum_;
            advance();
            return context_->create<ir::Constant>(value);
        }
        case TK_LitString:
        {
            string str = token_.value_;
            advance();
            return context_->create<ir::Constant>(str);
        }
        case TK_LitInteger:
        {
            int integer = token_.num_;
            advance();
            return context_->create<ir::Constant>(integer);
        }
        case TK_LitCharacter:
        {
            char c = token_.value_[0];
            advance();
            return context_->create<ir::Constant>(c);
        }
        case TK_Identifier:
        {
            string name = token_.value_;
            auto kind = table_->findName(name);
            if (kind == SymbolTable::None)
            {
                Diagnosis diag(DiagType::DT_Error, lexer_.getCoord());
                diag << "Using undefine identifier : " << name;
                diag_.diag(diag);
            }
            advance();
            if (token_.kind_ == TK_Assign)
            {
                advance();
                if (kind == SymbolTable::Let)
                {
                    return context_->create<ir::Store>(
                        expression(), table_->getValue(name), block_);
                }
                else
                {
                    Diagnosis diag(DiagType::DT_Error, lexer_.getCoord());
                    diag << "Try to assign to define constant : " << name;
                    diag_.diag(diag);
                    return nullptr;
                }
            }
            else
            {
                return context_->create<ir::Load>(
                    table_->getValue(name), getTmpName(), block_);
            }
        }
        case TK_LParen:
        {
            advance();
            Value * result = expression();
            match(TK_RParen);
            return result;
        }
        case TK_LSquareBrace:
        {
            return tableDecl();
        }
        case TK_Lambda:
        {
            return lambdaDecl();
        }
        default:
            return keywordConstant();
        }
    }

    Value *Parser::indexExpr()
    {
        Value *result = factor();
        while (token_.kind_ == TK_Period)
        {
            advance();
            string name = exceptIdentifier();
            Value *rhs = context_->create<ir::Constant>(name);
            if (token_.kind_ == TK_Assign)
            {
                result = context_->create<ir::SetIndex>(
                    result, rhs, expression(), block_);
                return result;
            }
            else
            {
                result = context_->create<ir::Index>(
                    result, rhs, getTmpName(), block_);
            }
        }
        return result;
    }

    Value *Parser::factorSuffix()
    {
        Value *result = indexExpr(), *rhs = nullptr;
        do {
            if (token_.kind_ == TK_LSquareBrace)
            {
                advance();
                Value *expr = expression();
                match(TK_RSquareBrace);
                if (token_.kind_ == TK_Assign)
                {
                    return context_->create<ir::SetIndex>(
                        result, expr, expression(), block_);
                }
                result = context_->create<ir::Index>(
                    result, expr, getTmpName(), block_);
            }
            else if (token_.kind_ == TK_LParen)
            {
                advance();
                std::vector<Value*> params;
                if (token_.kind_ != TK_RParen)
                {
                    params.push_back(expression());
                    while (token_.kind_ == TK_Comma)
                    {
                        advance();
                        params.push_back(expression());
                    }
                }
                
                match(TK_RParen);
                result = context_->create<ir::Invoke>(
                    result, params, getTmpName(), block_);
            }
            else
            {
                break;
            }
        } while (true);
        return result;
    }

    Value *Parser::notExpr()
    {
        if (token_.kind_ != TK_Not)
        {
            return factorSuffix();
        }
        advance();
        return context_->create<ir::NotOp>(factorSuffix(), getTmpName(), block_);
    }

    Value *Parser::negativeExpr()
    {
        using ir::BinaryOps;
        if (token_.kind_ != TK_Sub)
        {
            return notExpr();
        }
        advance();
        Value *zero = context_->create<ir::Constant>(0);
        return context_->create<ir::BinaryOperator>(
            BinaryOps::BO_Sub, zero, notExpr(), getTmpName(), block_);
    }

    Value *Parser::mulAndDivExpr()
    {
        using ir::BinaryOps;
        Value *result = negativeExpr();
        while (token_.kind_ == TK_Mul || token_.kind_ == TK_Div)
        {
            auto op = token_.kind_ == TK_Mul 
                ? BinaryOps::BO_Mul : BinaryOps::BO_Div;
            advance();
            result = context_->create<ir::BinaryOperator>(
                BinaryOps(op), result, negativeExpr(), getTmpName(), block_);
        }
        return result;
    }

    Value *Parser::addAndSubExpr()
    {
        using ir::BinaryOps;
        Value *result = mulAndDivExpr();
        while (token_.kind_ == TK_Plus || token_.kind_ == TK_Sub)
        {
            auto op = token_.kind_ == TK_Plus
                ? BinaryOps::BO_Add : BinaryOps::BO_Sub;
            advance();
            result = context_->create<ir::BinaryOperator>(
                BinaryOps(op), result, mulAndDivExpr(), getTmpName(), block_);
        }
        return result;
    }

    Value *Parser::relationalExpr()
    {
        using ir::BinaryOps;
        Value *result = addAndSubExpr();
        while (isRelational(token_.kind_))
        {
            auto op = TranslateRelationalToBinaryOps(token_.kind_);
            advance();
            result = context_->create<ir::BinaryOperator>(
                BinaryOps(op), result, addAndSubExpr(), getTmpName(), block_);
        }
        return result;
    }

    Value *Parser::andExpr()
    {
        Value *result = relationalExpr();
        if (token_.kind_ != TK_And)
            return result;

        BasicBlock *tmpBlock = block_,
            *thenBlock = cfg_->createBasicBlock(getTmpName("Label_")),
            *trueBlock = cfg_->createBasicBlock(getTmpName("true_expr_")),
            *falseBlock = cfg_->createBasicBlock(getTmpName("false_expr_")),
            *endBlock = cfg_->createBasicBlock(getTmpName("end_"));

        context_->create<ir::Branch>(result, falseBlock, trueBlock, tmpBlock);
        while (token_.kind_ == TK_And)
        {
            advance();
            Value *expr = relationalExpr();
            block_ = trueBlock;
            trueBlock = cfg_->createBasicBlock(getTmpName("true_"));
            context_->create<ir::Branch>(result, falseBlock, trueBlock, block_);
        }

        Value *true_ = context_->create<ir::Constant>(true);
        Value *false_ = context_->create<ir::Constant>(false);
        Value *trueVal = context_->create<ir::Assign>(true_, getTmpName(), trueBlock);
        Value *falseVal = context_->create<ir::Assign>(false_, getTmpName(), falseBlock);
        context_->create<ir::Goto>(endBlock, trueBlock);
        context_->create<ir::Goto>(endBlock, falseBlock);
        block_ = endBlock;
        auto params = { trueVal, falseVal };
        return context_->create<ir::Phi>(getTmpName(), block_, params);
    }

    Value *Parser::orExpr()
    {
        Value *result = andExpr();
        if (token_.kind_ != TK_Or)
            return result;

        BasicBlock *tmpBlock = block_,
            *thenBlock = cfg_->createBasicBlock(getTmpName("Label_")),
            *trueBlock = cfg_->createBasicBlock(getTmpName("true_expr_")),
            *falseBlock = cfg_->createBasicBlock(getTmpName("false_expr_")),
            *endBlock = cfg_->createBasicBlock(getTmpName("end_"));

        context_->create<ir::Branch>(result, trueBlock, falseBlock, tmpBlock);
        while (token_.kind_ == TK_Or)
        {
            advance();
            Value *expr = andExpr();
            block_ = falseBlock;
            falseBlock = cfg_->createBasicBlock(getTmpName("false_"));
            context_->create<ir::Branch>(result, trueBlock, falseBlock, block_);
        }

        Value *true_ = context_->create<ir::Constant>(true);
        Value *false_ = context_->create<ir::Constant>(false);
        Value *trueVal = context_->create<ir::Assign>(true_, getTmpName(), trueBlock);
        Value *falseVal = context_->create<ir::Assign>(false_, getTmpName(), falseBlock);
        context_->create<ir::Goto>(endBlock, trueBlock);
        context_->create<ir::Goto>(endBlock, falseBlock);
        block_ = endBlock;
        auto params = { trueVal, falseVal };
        return context_->create<ir::Phi>(getTmpName(), block_, params);
    }

    Value *Parser::expression()
    {
        using ir::Instructions;
        auto coord = lexer_.getCoord();
        Value *result = orExpr();
        if (token_.kind_ == TK_Assign)
        {
            advance();
            //orExpr();
            Value *rhs = expression();
            if (rhs->instance() == Instructions::IR_Load)
            {
                rhs = context_->create<ir::Load>(rhs, getTmpName(), block_);
            }

            if (result->instance() == Instructions::IR_Store)
            {
                context_->create<ir::Store>(rhs, result, block_);
            }
            else
            {
                // rvalue can't be assignned.
                Diagnosis diag(DiagType::DT_Error, coord);
                diag << "不能对右值赋值";
                diag_.diag(diag);
            }

            result = rhs;
        }
        return result;
    }

    void Parser::breakStat()
    {
        match(TK_Break);
        match(TK_Semicolon);

        if (breaks_.size() <= 0)
        {
            Diagnosis diag(DiagType::DT_Error, lexer_.getCoord());
            diag << "Break 外层需要 while statement";
            diag_.diag(diag);
            return;
        }

        context_->create<ir::Goto>(breaks_.top(), block_);

        // 尽管后面部分为落空语句，但是仍然为他建立一个block
        block_ = cfg_->createBasicBlock(getTmpName("full_throught_"));
    }

    void Parser::continueStat()
    {
        match(TK_Continue);
        match(TK_Semicolon);
        
        if (continues_.size() <= 0)
        {
            Diagnosis diag(DiagType::DT_Error, lexer_.getCoord());
            diag << "Continue 外层需要 while statement";
            diag_.diag(diag);
            return;
        }

        context_->create<ir::Goto>(continues_.top(), block_);
        
        // 尽管后面部分为落空语句，但是仍然为他建立一个block
        block_ = cfg_->createBasicBlock(getTmpName("full_throught_"));
    }

    void Parser::returnStat()
    {
        advance();
        if (token_.kind_ != TK_Semicolon)
        {
            Value *expr = expression();
            context_->create<ir::Return>(expr, block_);
        }
        else
        {
            context_->create<ir::ReturnVoid>(block_);
        }

        match(TK_Semicolon);
        BasicBlock *succBlock = cfg_->createBasicBlock("succ_");
        context_->create<ir::Goto>(succBlock, block_);
        block_ = succBlock;
    }

    void Parser::whileStat()
    {
        advance();
        BasicBlock *tmpBlock = block_,
            *condBlock = cfg_->createBasicBlock(getTmpName("while_cond_")),
            *thenBlock = cfg_->createBasicBlock(getTmpName("while_then_")),
            *endBlock = cfg_->createBasicBlock(getTmpName("end_while_"));

        block_ = condBlock;
        match(TK_LParen);
        Value *expr = expression();
        match(TK_RParen);
        context_->create<ir::Branch>(expr, thenBlock, endBlock, /*condBlock==*/block_);

        breaks_.push(endBlock);
        continues_.push(condBlock);

        block_ = thenBlock;
        statement();
        context_->create<ir::Goto>(condBlock, /*thenBlock==*/block_);

        block_ = endBlock;
        breaks_.pop();
        continues_.pop();
    }

    void Parser::ifStat()
    {
        advance();
        match(TK_LParen);
        Value *expr = expression();
        match(TK_RParen);

        BasicBlock *tmpBlock = block_,
            *thenBlock = cfg_->createBasicBlock(getTmpName("if_then_")),
            *endBlock = cfg_->createBasicBlock(getTmpName("if_end_")),
            *elseBlock = nullptr;

        block_ = thenBlock;
        statement();
        context_->create<ir::Goto>(endBlock, /*thenBlock==*/block_);

        if (token_.kind_ == TK_Else)
        {
            advance();

            elseBlock = cfg_->createBasicBlock(getTmpName("if_else_"));
            context_->create<ir::Branch>(expr, thenBlock, elseBlock, tmpBlock);

            block_ = elseBlock;
            statement();
            context_->create<ir::Goto>(endBlock, /*elseBlock==*/block_);
        }
        else
        {
            context_->create<ir::Branch>(expr, thenBlock, endBlock, tmpBlock);
        }

        block_ = endBlock;
    }

    void Parser::statement()
    {
        switch (token_.kind_)
        {
        case TK_LCurlyBrace:
            return block();
        case TK_If:
            return ifStat();
        case TK_While:
            return whileStat();
        case TK_Return:
            return returnStat();
        case TK_Break:
            return breakStat();
        case TK_Continue:
            return continueStat();
        case TK_Let:
            return letDecl();
        case TK_Define:
            return defineDecl();
        //case TK_Function:
            //return functionDecl();
        default:
            expression();
            match(TK_Semicolon);
            return ;
        }
    }

    void Parser::block()
    {
        match(TK_LCurlyBrace);
        while (token_.kind_ != TK_RCurlyBrace)
        {
            statement();
        }
        advance();
    }

    //
    // "("[param_list] ")"
    // 
    std::vector<std::pair<string, Token>> Parser::readParams()
    {
        match(TK_LParen);

        std::vector<std::pair<string, Token>> params;
        if (token_.kind_ == TK_Identifier)
        {
            params.push_back(std::pair<string, Token>(token_.value_, token_));
            advance();
            while (token_.kind_ == TK_Comma)
            {
                advance();
                Token token = token_;
                params.push_back(std::pair<string, Token>(exceptIdentifier(), token));
            }
        }

        match(TK_RParen);

        return std::move(params);
    }

    Value *Parser::keywordConstant()
    {
        Value *result = nullptr;
        if (token_.kind_ == TK_True)
            result = context_->create<ir::Constant>(true);
        else if (token_.kind_ == TK_False)
            result = context_->create<ir::Constant>(false);
        else if (token_.kind_ == TK_Null)
            result = context_->create<ir::Constant>();
        else
        {
            Diagnosis diag(DiagType::DT_Error, lexer_.getCoord());
            diag << "Unrecorded token" << Diagnosis::TokenToStirng(token_.kind_);
            diag_.diag(diag);
        }
        advance();
        return result;
    }

    Value *Parser::lambdaDecl()
    {
        match(TK_Lambda);
        std::string name = getTmpName("lambda_");
        table_->insertDefines(name, token_);
        IRFunction *function = module_.createFunction(name);
        Value *define = context_->create<ir::Alloca>(name, allocaBlock_);
        Value *invoke = context_->create<ir::Invoke>(
            name, std::vector<Value*>(), getTmpName(), block_);
        context_->create<ir::Store>(invoke, define, block_);
        table_->bindValue(name, define);

        auto params = std::move(readParams());
        function->setParams(params);

        // Saving some old containter.
        // Creating new containter.
        SymbolTable *table = table_;
        BasicBlock *oldBlock = block_;
        CFG *cfg = cfg_;
        IRContext *context = context_;

        table_ = function->getTable();
        context_ = function->getContext();
        cfg_ = function;

        BasicBlock *alloca = allocaBlock_;
        allocaBlock_ = cfg_->createBasicBlock(name + "_alloca");
        BasicBlock *save = block_ = cfg_->createBasicBlock(name + "_entry");

        LoadParamsIntoTable(table_, function->getContext(), allocaBlock_, params);
        function->setEntry(block_);
        block();
        function->setEnd(block_);

        context_->create<ir::Goto>(block_, allocaBlock_);
        allocaBlock_ = alloca;
        block_ = oldBlock;
        cfg_ = cfg;
        context_ = context;
        table_ = table;
        return invoke;
    }

    void Parser::tableOthers(Value *table)
    {
        Value *cons = nullptr;
        if (token_.kind_ == TK_LitCharacter)
        {
            cons = context_->create<ir::Constant>(token_.value_[0]);
        }
        else if (token_.kind_ == TK_LitFloat)
        {
            cons = context_->create<ir::Constant>(token_.fnum_);
        }
        else if (token_.kind_ == TK_LitInteger)
        {
            cons = context_->create<ir::Constant>(token_.num_);
        }
        else if (token_.kind_ == TK_LitString)
        {
            cons = context_->create<ir::Constant>(token_.value_);
        }
        else
        {
            Diagnosis diag(DiagType::DT_Error, lexer_.getCoord());
            diag << "Unknow table declare!";
            diag_.diag(diag);
            advance();
            return;
        }

        advance();
        if (token_.kind_ == TK_Assign)
        {
            advance();
            Value *expr = expression();
            if (cons->instance() == ir::Instructions::IR_Value)
            {
                ir::Constant *innerCons =
                    static_cast<ir::Constant*>(cons);
                if (innerCons->type() == ir::Constant::Integer
                    && innerCons->getInteger() < 0)
                {
                    Diagnosis diag(DiagType::DT_Error, lexer_.getCoord());
                    diag << "table index can't be less than zero";
                    diag_.diag(diag);
                }
            }
            context_->create<ir::SetIndex>(table, cons, expr, block_);
        }
        else
        {
            Value *tmp = context_->create<ir::Constant>(-1);
            context_->create<ir::SetIndex>(table, tmp, cons, block_);
        }
    }

    void Parser::tableIdent(Value *table)
    {
        string name = exceptIdentifier();
        if (token_.kind_ == TK_Assign)
        {
            // name = lambda ... == "name" = lambda
            advance();
            Value *expr = expression();
            Value *str = context_->create<ir::Constant>(name);
            context_->create<ir::SetIndex>(table, str, expr, block_);
        }
        else
        {
            if (table_->findName(name) == SymbolTable::None)
            {
                Diagnosis diag(DiagType::DT_Error, lexer_.getCoord());
                diag << "Using undefine id: \"" << name << "\"";
                diag_.diag(diag);
                return;
            }
            Value *cons = context_->create<ir::Constant>(-1);
            Value *tmp = context_->create<ir::Load>(
                table_->getValue(name), getTmpName(), block_);
            context_->create<ir::SetIndex>(table, cons, tmp, block_);
        }
    }

    Value *Parser::tableDecl()
    {
        Value *table = context_->create<ir::Alloca>(getTmpName("table_"), allocaBlock_);
        do {
            advance();
            if (token_.kind_ == TK_Identifier)
                tableIdent(table);
            else if (token_.kind_ == TK_RSquareBrace)
                break;
            else
                tableOthers(table);
            
        } while (token_.kind_ == TK_Comma);
        match(TK_RSquareBrace);
        return nullptr;
    }

    void Parser::functionDecl()
    {
        match(TK_Function);
        string name = exceptIdentifier();

        if (table_->findName(name) != SymbolTable::None)
        {
            Diagnosis diag(DiagType::DT_Error, lexer_.getCoord());
            diag << "redefined \"" << name << "\" as define function";
            diag_.diag(diag);
        }
        table_->insertDefines(name, token_);
        IRFunction *function = module_.createFunction(name);
        Value *define = context_->create<ir::Alloca>(name, allocaBlock_);
        Value *invoke = context_->create<ir::Invoke>(
            name, std::vector<Value*>(), getTmpName(), block_);
        context_->create<ir::Store>(invoke, define, block_);
        table_->bindValue(name, define);

        auto params = std::move(readParams());
        function->setParams(params);

        // Saving some old containter.
        // Creating new containter.
        SymbolTable *table = table_;
        BasicBlock *oldBlock = block_;
        CFG *cfg = cfg_;
        IRContext *context = context_;

        table_ = function->getTable();
        context_ = function->getContext();
        cfg_ = function;

        BasicBlock *alloca = allocaBlock_;
        allocaBlock_ = cfg_->createBasicBlock(name + "_alloca");
        BasicBlock *save = block_ = cfg_->createBasicBlock(name + "_entry");

        LoadParamsIntoTable(table_, function->getContext(), allocaBlock_, params);
        function->setEntry(block_);
        block();
        function->setEnd(block_);
        
        context_->create<ir::Goto>(save, allocaBlock_);
        allocaBlock_ = alloca;
        block_ = oldBlock;
        cfg_ = cfg;
        context_ = context;
        table_ = table;
    }

    void Parser::letDecl()
    {
        match(TK_Let);
        string name = exceptIdentifier();
        if (table_->findName(name) != SymbolTable::None)
        {
            Diagnosis diag(DiagType::DT_Error, lexer_.getCoord());
            diag << "redefined \"" << name << "\" as let binding";
            diag_.diag(diag);
        }
        table_->insertVariables(name, token_);
        Value *let = context_->create<ir::Alloca>(name, allocaBlock_);
        table_->bindValue(name, let);

        match(TK_Assign);
        Value *expr = expression();
        context_->create<ir::Store>(expr, let, block_);
        match(TK_Semicolon);
    }

    void Parser::defineDecl()
    {
        match(TK_Define);
        string name = exceptIdentifier();
        if (table_->findName(name) != SymbolTable::None)
        {
            Diagnosis diag(DiagType::DT_Error, lexer_.getCoord());
            diag << "redefined \"" << name << "\" as define constant";
            diag_.diag(diag);
        }

        table_->insertDefines(name, token_);
        // insert Alloca instr.
        Value *define = context_->create<ir::Alloca>(name, allocaBlock_);
        table_->bindValue(name, define);

        match(TK_Assign);
        Value *expr = expression();
        context_->create<ir::Store>(expr, define, block_);
        match(TK_Semicolon);
    }

    bool Parser::topLevelDecl()
    {
        if (token_.kind_ == TK_Define)
            defineDecl();
        else if (token_.kind_ == TK_Let)
            letDecl();
        else if (token_.kind_ == TK_Function)
            functionDecl();
        else
            return false;
        return true;
    }

    void Parser::parse()
    {
        BasicBlock *entry = module_.createBasicBlock("entry");
        allocaBlock_ = module_.createBasicBlock("global_alloca");
        block_ = entry;
        table_ = module_.getTable();
        context_ = module_.getContext();
        cfg_ = &module_;

        module_.setEntry(entry);

        advance();
        while (token_.kind_ != TK_EOF)
        {
            if (!topLevelDecl())
            {
                expression();
                match(TK_Semicolon);
            }
        }

        module_.setEnd(block_);

        context_->create<ir::Goto>(entry, allocaBlock_);
    }

    Parser::Parser(Lexer & lexer, IRModule &module, DiagnosisConsumer &diag) 
        : lexer_(lexer), module_(module), diag_(diag)
    {
        initialize();
    }

    void Parser::advance()
    {
        token_ = lexer_.getToken();
    }

    void Parser::match(unsigned tok)
    {
        if (token_.kind_ != tok)
        {
            Diagnosis diag(DiagType::DT_Error, token_.coord_);
            diag << Diagnosis::TokenToStirng(tok)
                << " except in file but find "
                << Diagnosis::TokenToStirng(token_.kind_);
            diag_.diag(diag);
        }
        advance();
    }

    std::string Parser::exceptIdentifier()
    {
        if (token_.kind_ != TK_Identifier)
        {
            Diagnosis diag(DiagType::DT_Error, token_.coord_);
            diag << Diagnosis::TokenToStirng(TK_Identifier)
                << " except in file but find "
                << Diagnosis::TokenToStirng(token_.kind_);
            diag_.diag(diag);
        }
        string value = std::move(token_.value_);
        advance();
        return std::move(value);
    }
}
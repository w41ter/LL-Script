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

using namespace script::ir;

using std::map;
using std::set;
using std::string;
using std::vector;

namespace script
{

namespace
{
    std::string getTmpName(std::string name = "Tmp_")
    {
        static std::map<std::string, int> count;
        std::stringstream stream;
        stream << name << count[name]++;
        std::string str;
        stream >> str;
        return str;
    }

    BinaryOps TranslateRelationalToBinaryOps(unsigned tok)
    {
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
        }
    }
}

    bool Parser::tryToCatchID(std::string &name)
    {
        SymbolTable *table = table_;
        if (table_->findName(name) != SymbolTable::None)
            return true;
        table = table->getParent();
        while (table != nullptr)
        {
            if (table->findName(name) != SymbolTable::None)
            {
                table->catchedName(name);
                auto kind = table->findName(name);
                if (kind == SymbolTable::Define)
                    table_->insertDefines(name, token_);
                else
                    table_->insertVariables(name, token_);
                return true;
            }
            table = table->getParent();
        }
        Diagnosis diag(DiagType::DT_Error, lexer_.getCoord());
        diag << "Using undefine identifier : " << name;
        diag_.diag(diag);
        return false;
    }

    bool Parser::isRelational(unsigned tok)
    {
        return (tok == TK_Less || tok == TK_LessThan ||
            tok == TK_Great || tok == TK_GreatThan ||
            tok == TK_NotEqual || tok == TK_EqualThan);
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

    Value *Parser::variable()
    {
        switch (token_.kind_)
        {
        case TK_Identifier:
        {  
            string name = token_.value_;
            advance();

            tryToCatchID(name);
            return cfg_->readVariableDef(name, block_);
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
            break;
        }
    }

    Value *Parser::variableSuffix()
    {
        Value *result = variable();
        while (true)
        {
            switch (token_.kind_)
            {
            case TK_LSquareBrace:
            {
                advance();
                Value *expr = expression();
                match(TK_RSquareBrace);
                // 默认生成 Index, 在 Assign 部分处理成为 SetIndex
                result = context_->create<Index>(
                    result, expr, getTmpName(), block_);
            }
            case TK_LCurlyBrace:
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
                result = context_->create<Invoke>(
                    result, params, getTmpName(), block_);
            }
            case TK_Period:
            {
                advance();
                string name = exceptIdentifier();
                Value *rhs = context_->create<Constant>(name);
                rhs = context_->create<Assign>(rhs, getTmpName(), block_);
                result = context_->create<Index>(
                    result, rhs, getTmpName(), block_);
            }
            default:
                goto END;
            }
        }
    END:
        return result;
    }

    Value *Parser::value()
    {
        switch (token_.kind_)
        {
        case TK_LitFloat:
        {
            float value = token_.fnum_;
            advance();
            Value *val = context_->create<Constant>(value);
            return context_->create<Assign>(val, getTmpName(), block_);
        }
        case TK_LitString:
        {
            string str = token_.value_;
            advance();
            Value *val = context_->create<Constant>(str);
            return context_->create<Assign>(val, getTmpName(), block_);
        }
        case TK_LitInteger:
        {
            int integer = token_.num_;
            advance();
            Value *val = context_->create<Constant>(integer);
            return context_->create<Assign>(val, getTmpName(), block_);
        }
        case TK_LitCharacter:
        {
            char c = token_.value_[0];
            advance();
            Value *val = context_->create<Constant>(c);
            return context_->create<Assign>(val, getTmpName(), block_);
        }
        case TK_True:
        {
            advance();
            Value *val = context_->create<Constant>(true);
            return context_->create<Assign>(val, getTmpName(), block_);
        }
        case TK_False:
        {
            advance();
            Value *val = context_->create<Constant>(false);
            return context_->create<Assign>(val, getTmpName(), block_);
        }
        case TK_Null:
        {
            advance();
            Value *val = context_->create<Constant>();
            return context_->create<Assign>(val, getTmpName(), block_);
        }
        default:
            return variableSuffix();
        }
    }

    Value *Parser::notExpr()
    {
        if (token_.kind_ != TK_Not)
        {
            return value();
        }
        advance();
        return context_->create<NotOp>(value(), getTmpName(), block_);
    }

    Value *Parser::negativeExpr()
    {
        if (token_.kind_ != TK_Sub)
        {
            return notExpr();
        }
        advance();
        Value *zero = context_->create<Constant>(0);
        return context_->create<BinaryOperator>(
            BinaryOps::BO_Sub, zero, notExpr(), getTmpName(), block_);
    }

    Value *Parser::mulAndDivExpr()
    {
        Value *result = negativeExpr();
        while (token_.kind_ == TK_Mul || token_.kind_ == TK_Div)
        {
            auto op = token_.kind_ == TK_Mul 
                ? BinaryOps::BO_Mul : BinaryOps::BO_Div;
            advance();
            result = context_->create<BinaryOperator>(
                BinaryOps(op), result, negativeExpr(), getTmpName(), block_);
        }
        return result;
    }

    Value *Parser::addAndSubExpr()
    {
        Value *result = mulAndDivExpr();
        while (token_.kind_ == TK_Plus || token_.kind_ == TK_Sub)
        {
            auto op = token_.kind_ == TK_Plus
                ? BinaryOps::BO_Add : BinaryOps::BO_Sub;
            advance();
            result = context_->create<BinaryOperator>(
                BinaryOps(op), result, mulAndDivExpr(), getTmpName(), block_);
        }
        return result;
    }

    Value *Parser::relationalExpr()
    {
        Value *result = addAndSubExpr();
        while (isRelational(token_.kind_))
        {
            auto op = TranslateRelationalToBinaryOps(token_.kind_);
            advance();
            result = context_->create<BinaryOperator>(
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
            *trueBlock = cfg_->createBasicBlock(getTmpName("true_expr_")),
            *falseBlock = cfg_->createBasicBlock(getTmpName("false_expr_"));

        context_->create<Branch>(result, falseBlock, trueBlock, tmpBlock);
        while (token_.kind_ == TK_And)
        {
            advance();
            block_ = trueBlock;
            Value *expr = relationalExpr();
            trueBlock = cfg_->createBasicBlock(getTmpName("true_expr_"));
            context_->create<Branch>(result, falseBlock, trueBlock, block_);
        }

        Value *true_ = context_->create<Constant>(true);
        Value *false_ = context_->create<Constant>(false);
        Value *trueVal = context_->create<Assign>(true_, getTmpName(), trueBlock);
        Value *falseVal = context_->create<Assign>(false_, getTmpName(), falseBlock);

        BasicBlock *endBlock = cfg_->createBasicBlock(getTmpName("end_expr_"));
        context_->create<Goto>(endBlock, trueBlock);
        context_->create<Goto>(endBlock, falseBlock);
        block_ = endBlock;
        auto params = { trueVal, falseVal };
        return context_->create<Phi>(getTmpName(), block_, params);
    }

    Value *Parser::orExpr()
    {
        Value *result = andExpr();
        if (token_.kind_ != TK_Or)
            return result;

        BasicBlock *tmpBlock = block_,
            *trueBlock = cfg_->createBasicBlock(getTmpName("true_expr_")),
            *falseBlock = cfg_->createBasicBlock(getTmpName("false_expr_"));

        context_->create<Branch>(result, trueBlock, falseBlock, tmpBlock);
        while (token_.kind_ == TK_Or)
        {
            advance();
            block_ = falseBlock;
            Value *expr = andExpr();
            falseBlock = cfg_->createBasicBlock(getTmpName("false_expr_"));
            context_->create<Branch>(result, trueBlock, falseBlock, block_);
        }

        Value *true_ = context_->create<Constant>(true);
        Value *false_ = context_->create<Constant>(false);
        Value *trueVal = context_->create<Assign>(true_, getTmpName(), trueBlock);
        Value *falseVal = context_->create<Assign>(false_, getTmpName(), falseBlock);

        BasicBlock *endBlock = cfg_->createBasicBlock(getTmpName("end_expr_"));
        context_->create<Goto>(endBlock, trueBlock);
        context_->create<Goto>(endBlock, falseBlock);
        block_ = endBlock;
        auto params = { trueVal, falseVal };
        return context_->create<Phi>(getTmpName(), block_, params);
    }

    Value *Parser::expression()
    {
        auto coord = lexer_.getCoord();
        Value *result = orExpr();
        if (token_.kind_ == TK_Assign)
        {
            advance();
            Value *rhs = expression();

                Diagnosis diag(DiagType::DT_Error, coord);
                diag << "不能对右值赋值";
                diag_.diag(diag);
            

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

        context_->create<Goto>(breaks_.top(), block_);

        cfg_->sealBlock(block_);

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

        context_->create<Goto>(continues_.top(), block_);
        
        cfg_->sealBlock(block_);

        // 尽管后面部分为落空语句，但是仍然为他建立一个block
        block_ = cfg_->createBasicBlock(getTmpName("full_throught_"));
    }

    void Parser::returnStat()
    {
        advance();
        if (token_.kind_ != TK_Semicolon)
        {
            Value *expr = expression();
            context_->create<Return>(expr, block_);
        }
        else
        {
            context_->create<ReturnVoid>(block_);
        }

        match(TK_Semicolon);
        BasicBlock *succBlock = cfg_->createBasicBlock("return_succ_");
        cfg_->sealBlock(block_);
        block_ = succBlock;
    }

    void Parser::whileStat()
    {
        advance();
        BasicBlock *tmpBlock = block_,
            *condBlock = cfg_->createBasicBlock(getTmpName("while_cond_")),
            *thenBlock = cfg_->createBasicBlock(getTmpName("while_then_")),
            *endBlock = cfg_->createBasicBlock(getTmpName("end_while_"));

        context_->create<Goto>(condBlock, tmpBlock);
        cfg_->sealBlock(tmpBlock);

        block_ = condBlock;
        match(TK_LParen);
        Value *expr = expression();
        match(TK_RParen);
        context_->create<Branch>(expr, thenBlock, endBlock, /*condBlock==*/block_);

        breaks_.push(endBlock);
        continues_.push(condBlock);

        block_ = thenBlock;
        statement();
        context_->create<Goto>(condBlock, /*thenBlock==*/block_);
        
        cfg_->sealBlock(thenBlock);
        cfg_->sealBlock(block_);
        cfg_->sealBlock(condBlock);

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

        cfg_->sealBlock(block_);

        BasicBlock *tmpBlock = block_,
            *thenBlock = cfg_->createBasicBlock(getTmpName("if_then_"));

        block_ = thenBlock;
        statement();
        BasicBlock *thenEndBlock = block_, *endBlock = nullptr;

        cfg_->sealBlock(block_);

        if (token_.kind_ == TK_Else)
        {
            advance();

            BasicBlock *elseBlock = cfg_->createBasicBlock(getTmpName("if_else_"));
            context_->create<Branch>(expr, thenBlock, elseBlock, tmpBlock);

            block_ = elseBlock;
            statement();

            cfg_->sealBlock(elseBlock);
            cfg_->sealBlock(block_);

            endBlock = cfg_->createBasicBlock(getTmpName("if_end_"));
            context_->create<Goto>(endBlock, /*elseBlock==*/block_);
        }
        else
        {
            endBlock = cfg_->createBasicBlock(getTmpName("if_end_"));
            context_->create<Branch>(expr, thenBlock, endBlock, tmpBlock);
        }

        cfg_->sealBlock(thenBlock);

        context_->create<Goto>(endBlock, thenEndBlock);
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
            result = context_->create<Constant>(true);
        else if (token_.kind_ == TK_False)
            result = context_->create<Constant>(false);
        else if (token_.kind_ == TK_Null)
            result = context_->create<Constant>();
        else
        {
            Diagnosis diag(DiagType::DT_Error, lexer_.getCoord());
            diag << "Unrecorded token" << Diagnosis::TokenToStirng(token_.kind_);
            diag_.diag(diag);
        }
        advance();
        result = context_->create<Assign>(result, getTmpName(), block_);
        return result;
    }

    Value *Parser::lambdaDecl()
    {
        match(TK_Lambda);
        std::string name = getTmpName("lambda_");
        table_->insertDefines(name, token_);

        IRFunction *function = module_.createFunction(name);
        Value *invoke = context_->create<Invoke>(
            name, std::vector<Value*>(), name, block_);
        cfg_->saveVariableDef(name, block_, invoke);

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
        table_->bindParent(table);

        BasicBlock *alloca = allocaBlock_;
        allocaBlock_ = cfg_->createBasicBlock(name + "_alloca");
        BasicBlock *save = block_ = cfg_->createBasicBlock(name + "_entry");

        LoadParamsIntoTable(table_, function->getContext(), allocaBlock_, params);
        function->setEntry(block_);
        block();
        function->setEnd(block_);

        context_->create<Goto>(save, allocaBlock_);

        cfg_->sealOthersBlock();

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
            cons = context_->create<Constant>(token_.value_[0]);
        }
        else if (token_.kind_ == TK_LitFloat)
        {
            cons = context_->create<Constant>(token_.fnum_);
        }
        else if (token_.kind_ == TK_LitInteger)
        {
            cons = context_->create<Constant>(token_.num_);
        }
        else if (token_.kind_ == TK_LitString)
        {
            cons = context_->create<Constant>(token_.value_);
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
            if (cons->instance() == Instructions::IR_Value)
            {
                Constant *innerCons =
                    static_cast<Constant*>(cons);
                if (innerCons->type() == Constant::Integer
                    && innerCons->getInteger() < 0)
                {
                    Diagnosis diag(DiagType::DT_Error, lexer_.getCoord());
                    diag << "table index can't be less than zero";
                    diag_.diag(diag);
                }
            }
            cons = context_->create<Assign>(cons, getTmpName(), block_);
            context_->create<SetIndex>(table, cons, expr, block_);
        }
        else
        {
            Value *tmp = context_->create<Constant>(-1);
            tmp = context_->create<Assign>(tmp, getTmpName(), block_);
            cons = context_->create<Assign>(cons, getTmpName(), block_);
            context_->create<SetIndex>(table, tmp, cons, block_);
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
            Value *str = context_->create<Constant>(name);
            str = context_->create<Assign>(str, getTmpName(), block_);
            context_->create<SetIndex>(table, str, expr, block_);
        }
        else
        {
            tryToCatchID(name);
            Value *id = cfg_->readVariableDef(name, block_);//findID(name);
            Value *cons = context_->create<Constant>(-1);
            cons = context_->create<Assign>(cons, getTmpName(), block_);
            //Value *tmp = context_->create<Load>(id, getTmpName(), block_);
            context_->create<SetIndex>(table, cons, id, block_);
        }
    }

    Value *Parser::tableDecl()
    {
        Value *table = context_->create<Alloca>(getTmpName("table_"), allocaBlock_);
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
        return table;
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

        // create function and generate parallel invoke.
        IRFunction *function = module_.createFunction(name);
        Value *invoke = context_->create<Invoke>(
            name, std::vector<Value*>(), name, block_);
        cfg_->saveVariableDef(name, block_, invoke);

        // match params
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
        table_->bindParent(table);

        BasicBlock *alloca = allocaBlock_;
        allocaBlock_ = cfg_->createBasicBlock(name + "_alloca");
        BasicBlock *save = block_ = cfg_->createBasicBlock(name + "_entry");

        LoadParamsIntoTable(table_, function->getContext(), allocaBlock_, params);
        function->setEntry(block_);
        block();
        function->setEnd(block_);
        
        context_->create<Goto>(save, allocaBlock_);
        
        cfg_->sealOthersBlock();
        
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
        match(TK_Assign);

        Value *expr = expression();
        Value *let = context_->create<Assign>(expr, name, block_);
        cfg_->saveVariableDef(name, block_, let);
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

        match(TK_Assign);

        // match expression and save variable def.
        Value *expr = expression();
        Value *define = context_->create<Assign>(expr, name, block_);
        cfg_->saveVariableDef(name, block_, define);
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
        allocaBlock_ = module_.createBasicBlock("global_alloca");
        BasicBlock *entry = module_.createBasicBlock("entry");

        block_ = entry;
        table_ = module_.getTable();
        context_ = module_.getContext();
        cfg_ = &module_;

        module_.setEntry(allocaBlock_);

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

        context_->create<Goto>(entry, allocaBlock_);
        cfg_->sealOthersBlock();
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
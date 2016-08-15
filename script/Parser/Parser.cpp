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
            Symbol symbol;
            symbol.type_ = Symbol::Let;
            symbol.tok_ = i.second;
            table->insert(i.first, symbol);
        }
    }
}

    bool Parser::tryToCatchID(std::string &name)
    {
        SymbolTable *table = table_;
        Symbol symbol;
        if (table_->find(name, symbol) != Symbol::None)
            return true;
        table = table->getParent();
        while (table != nullptr)
        {
            unsigned kind = table->find(name, symbol);
            if (kind != Symbol::None)
            {
                table->setCatched(name);
                symbol.param_ = false;
                symbol.beCaught_ = false;
                symbol.caught_ = true;
                symbol.type_ = kind;
                table_->insert(name, symbol);
                return true;
            }
            table = table->getParent();
        }

        diag_.undefineID(name, lexer_.getCoord());
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

    std::string Parser::LHS(std::list<Value*> &lhs)
    {
        string name = exceptIdentifier();
        Value *val = nullptr;
        while (true)
        {
            switch (token_.kind_)
            {
            case TK_LCurlyBrace:
            {
                if (val == nullptr)
                    val = cfg_->readVariableDef(name, block_);
                
                advance();
                std::vector<Value*> params;
                if (token_.kind_ != TK_RParen)
                {
                    params.push_back(orExpr());
                    while (token_.kind_ == TK_Comma)
                    {
                        advance();
                        params.push_back(orExpr());
                    }
                }

                match(TK_RParen);
                val = context_->create<Invoke>(
                    val, params, getTmpName());
                lhs.push_back(val);
                break;
            }
            case TK_LSquareBrace:
            {
                if (val == nullptr)
                    val = cfg_->readVariableDef(name, block_);

                advance();
                Value *expr = orExpr();
                match(TK_RSquareBrace);
                // 默认生成 Index, 在 Assign 部分处理成为 SetIndex
                val = context_->create<Index>(
                    val, expr, getTmpName());
                lhs.push_back(val);
                break;
            }
            case TK_Period:
            {
                if (val == nullptr)
                    val = cfg_->readVariableDef(name, block_);

                advance();
                string name = exceptIdentifier();
                Value *rhs = context_->create<Constant>(name);
                rhs = context_->create<Assign>(rhs, getTmpName());
                val = context_->create<Index>(val, rhs, getTmpName());
                lhs.push_back(val);
                break;
            }
            default:
                return name;
            }
        }
        return name;
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
            Value * result = orExpr();
            match(TK_RParen);
            return result;
        }
        case TK_Lambda:
        {
            return lambdaDecl();
        }
        default:
            diag_.unexceptedToken(token_.kind_, lexer_.getCoord());
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
                Value *expr = orExpr();
                match(TK_RSquareBrace);
                result = context_->createAtEnd<Index>(
                    block_, result, expr, getTmpName());
            }
            case TK_LParen:
            {
                advance();
                std::vector<Value*> params;
                if (token_.kind_ != TK_RParen)
                {
                    params.push_back(orExpr());
                    while (token_.kind_ == TK_Comma)
                    {
                        advance();
                        params.push_back(orExpr());
                    }
                }

                match(TK_RParen);
                result = context_->createAtEnd<Invoke>(
                    block_, result, params, getTmpName());
            }
            case TK_Period:
            {
                advance();
                string name = exceptIdentifier();
                Value *rhs = context_->create<Constant>(name);
                rhs = context_->createAtEnd<Assign>(block_, rhs, getTmpName());
                result = context_->createAtEnd<Index>(
                    block_, result, rhs, getTmpName());
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
            return context_->createAtEnd<Assign>(block_, val, getTmpName());
        }
        case TK_LitString:
        {
            string str = token_.value_;
            advance();
            Value *val = context_->create<Constant>(str);
            return context_->createAtEnd<Assign>(block_, val, getTmpName());
        }
        case TK_LitInteger:
        {
            int integer = token_.num_;
            advance();
            Value *val = context_->create<Constant>(integer);
            return context_->createAtEnd<Assign>(block_, val, getTmpName());
        }
        case TK_LitCharacter:
        {
            char c = token_.value_[0];
            advance();
            Value *val = context_->create<Constant>(c);
            return context_->createAtEnd<Assign>(block_, val, getTmpName());
        }
        case TK_True:
        {
            advance();
            Value *val = context_->create<Constant>(true);
            return context_->createAtEnd<Assign>(block_, val, getTmpName());
        }
        case TK_False:
        {
            advance();
            Value *val = context_->create<Constant>(false);
            return context_->createAtEnd<Assign>(block_, val, getTmpName());
        }
        case TK_Null:
        {
            advance();
            Value *val = context_->create<Constant>();
            return context_->createAtEnd<Assign>(block_, val, getTmpName());
        }
        case TK_LSquareBrace:
        {
            // let b = [a][a] or ['a' = 1].a is wrong.
            return tableDecl();
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
        return context_->createAtEnd<NotOp>(block_, value(), getTmpName());
    }

    Value *Parser::negativeExpr()
    {
        if (token_.kind_ != TK_Sub)
        {
            return notExpr();
        }
        advance();
        Value *zero = context_->create<Constant>(0);
        return context_->createAtEnd<BinaryOperator>(
            block_, BinaryOps::BO_Sub, zero, notExpr(), getTmpName());
    }

    Value *Parser::mulAndDivExpr()
    {
        Value *result = negativeExpr();
        while (token_.kind_ == TK_Mul || token_.kind_ == TK_Div)
        {
            auto op = token_.kind_ == TK_Mul 
                ? BinaryOps::BO_Mul : BinaryOps::BO_Div;
            advance();
            result = context_->createAtEnd<BinaryOperator>(
                block_, BinaryOps(op), result, negativeExpr(), getTmpName());
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
            result = context_->createAtEnd<BinaryOperator>(
                block_, BinaryOps(op), result, mulAndDivExpr(), getTmpName());
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
            result = context_->createAtEnd<BinaryOperator>(
                block_, BinaryOps(op), result, addAndSubExpr(), getTmpName());
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

        context_->createBranchAtEnd(tmpBlock, result, falseBlock, trueBlock);
        while (token_.kind_ == TK_And)
        {
            advance();
            block_ = trueBlock;
            Value *expr = relationalExpr();
            trueBlock = cfg_->createBasicBlock(getTmpName("true_expr_"));
            context_->createBranchAtEnd(block_, result, falseBlock, trueBlock);
        }

        Value *true_ = context_->create<Constant>(true);
        Value *false_ = context_->create<Constant>(false);
        Value *trueVal = context_->createAtEnd<Assign>(trueBlock, true_, getTmpName());
        Value *falseVal = context_->createAtEnd<Assign>(falseBlock, false_, getTmpName());

        BasicBlock *endBlock = cfg_->createBasicBlock(getTmpName("end_expr_"));
        context_->createGotoAtEnd(trueBlock, endBlock);
        context_->createGotoAtEnd(falseBlock, endBlock);
        block_ = endBlock;
        auto params = { trueVal, falseVal };
        return context_->createAtEnd<Phi>(block_, getTmpName(), params);
    }

    Value *Parser::orExpr()
    {
        Value *result = andExpr();
        if (token_.kind_ != TK_Or)
            return result;

        BasicBlock *tmpBlock = block_,
            *trueBlock = cfg_->createBasicBlock(getTmpName("true_expr_")),
            *falseBlock = cfg_->createBasicBlock(getTmpName("false_expr_"));

        context_->createBranchAtEnd(tmpBlock, result, trueBlock, falseBlock);
        while (token_.kind_ == TK_Or)
        {
            advance();
            block_ = falseBlock;
            Value *expr = andExpr();
            falseBlock = cfg_->createBasicBlock(getTmpName("false_expr_"));
            context_->createBranchAtEnd(block_, result, trueBlock, falseBlock);
        }

        Value *true_ = context_->create<Constant>(true);
        Value *false_ = context_->create<Constant>(false);
        Value *trueVal = context_->createAtEnd<Assign>(trueBlock, true_, getTmpName());
        Value *falseVal = context_->createAtEnd<Assign>(falseBlock, false_, getTmpName());

        BasicBlock *endBlock = cfg_->createBasicBlock(getTmpName("end_expr_"));
        context_->createGotoAtEnd(trueBlock, endBlock);
        context_->createGotoAtEnd(falseBlock, endBlock);
        block_ = endBlock;
        auto params = { trueVal, falseVal };
        return context_->createAtEnd<Phi>(block_, getTmpName(), params);
    }

    Value *Parser::expression()
    {
        auto coord = lexer_.getCoord();
        std::list<Value*> lhs;
        
        // get left val.
        std::string name = LHS(lhs);
        if (token_.kind_ == TK_Assign)
        {
            advance();
            Value *rhs = orExpr();
            
            if (lhs.size() != 0)
            {
                Value *lastL = lhs.back();
                if (lastL->instance() == Instructions::IR_Index)
                {
                    Index *si = (Index*)lastL;
                    lhs.pop_back();
                    lhs.push_back(context_->create<SetIndex>(
                        si->table(), si->index(), rhs));
                }
            }
            else 
            {
                Value *result = context_->create<Store>(rhs, cfg_->phiName(name));
                cfg_->saveVariableDef(name, block_, result);
                lhs.push_back(result);
            }
            
            for (auto *i : lhs)
            {
                Instruction *instr = (Instruction*)i;
                block_->push_back(instr);
                instr->setParent(block_);
            }
        }
        if (lhs.size() > 0)
            return lhs.back();
        else
            return cfg_->readVariableDef(name, block_);
    }

    void Parser::breakStat()
    {
        match(TK_Break);
        match(TK_Semicolon);

        if (breaks_.size() <= 0)
        {
            diag_.outOfScopeBreakOrContinue(lexer_.getCoord());
            return;
        }

        context_->createGotoAtEnd(block_, breaks_.top());

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
            diag_.outOfScopeBreakOrContinue(lexer_.getCoord());
            return;
        }

        context_->createGotoAtEnd(block_, continues_.top());
        
        cfg_->sealBlock(block_);

        // 尽管后面部分为落空语句，但是仍然为他建立一个block
        block_ = cfg_->createBasicBlock(getTmpName("full_throught_"));
    }

    void Parser::returnStat()
    {
        advance();
        if (token_.kind_ != TK_Semicolon)
        {
            Value *expr = orExpr();
            context_->createAtEnd<Return>(block_, expr);
        }
        else
        {
            context_->createAtEnd<ReturnVoid>(block_);
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

        context_->createGotoAtEnd(tmpBlock, condBlock);
        cfg_->sealBlock(tmpBlock);

        block_ = condBlock;
        match(TK_LParen);
        Value *expr = orExpr();
        match(TK_RParen);
        context_->createBranchAtEnd(block_, expr, thenBlock, endBlock);

        breaks_.push(endBlock);
        continues_.push(condBlock);

        block_ = thenBlock;
        statement();
        context_->createGotoAtEnd(/*thenBlock==*/block_, condBlock);
        
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
        Value *expr = orExpr();
        match(TK_RParen);

        cfg_->sealBlock(block_);

        BasicBlock *tmpBlock = block_,
            *thenBlock = cfg_->createBasicBlock(getTmpName("if_then_"));

        block_ = thenBlock;
        statement();
        BasicBlock *thenEndBlock = block_, *endBlock = nullptr;

        if (token_.kind_ == TK_Else)
        {
            advance();

            BasicBlock *elseBlock = cfg_->createBasicBlock(getTmpName("if_else_"));
            context_->createBranchAtEnd(tmpBlock, expr, thenBlock, elseBlock);

            block_ = elseBlock;
            statement();

            cfg_->sealBlock(elseBlock);
            cfg_->sealBlock(block_);

            endBlock = cfg_->createBasicBlock(getTmpName("if_end_"));
            context_->createGotoAtEnd(/*elseBlock==*/block_, endBlock);
        }
        else
        {
            endBlock = cfg_->createBasicBlock(getTmpName("if_end_"));
            context_->createBranchAtEnd(tmpBlock, expr, thenBlock, endBlock);
        }

        cfg_->sealBlock(thenBlock);
        cfg_->sealBlock(thenEndBlock);

        context_->createGotoAtEnd(thenEndBlock, endBlock);
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
        case TK_Semicolon:
            return statement();
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

    Value *Parser::lambdaDecl()
    {
        match(TK_Lambda);
        std::string name = getTmpName("lambda_");
        Symbol symbol;
        symbol.type_ = Symbol::Define;
        symbol.beCaught_ = false;
        symbol.caught_ = false;
        symbol.tok_ = token_;
        symbol.param_ = false;
        table_->insert(name, symbol);

        IRFunction *function = module_.createFunction(name);
        Value *invoke = context_->createAtEnd<Invoke>(
            block_, name, std::vector<Value*>(), name);
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

        BasicBlock *functionEntry = block_ = cfg_->createBasicBlock(name + "_entry");

        LoadParamsIntoTable(table_, function->getContext(), block_, params);
        function->setEntry(block_);
        block();
        function->setEnd(block_);

        cfg_->sealOthersBlock();

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
            diag_.unknowTableDecl(lexer_.getCoord());
            advance();
            return;
        }

        advance();
        if (token_.kind_ == TK_Assign)
        {
            advance();
            Value *expr = orExpr();
            if (cons->instance() == Instructions::IR_Constant)
            {
                Constant *innerCons =
                    static_cast<Constant*>(cons);
                if (innerCons->type() == Constant::Integer
                    && innerCons->getInteger() < 0)
                {
                    diag_.indexLessThanZero(lexer_.getCoord());
                }
            }
            cons = context_->createAtEnd<Assign>(block_, cons, getTmpName());
            context_->createAtEnd<SetIndex>(block_, table, cons, expr);
        }
        else
        {
            Value *tmp = context_->create<Constant>(-1);
            tmp = context_->createAtEnd<Assign>(block_, tmp, getTmpName());
            cons = context_->createAtEnd<Assign>(block_, cons, getTmpName());
            context_->createAtEnd<SetIndex>(block_, table, tmp, cons);
        }
    }

    void Parser::tableIdent(Value *table)
    {
        string name = exceptIdentifier();
        if (token_.kind_ == TK_Assign)
        {
            // name = lambda ... == "name" = lambda
            advance();
            Value *expr = orExpr();
            Value *str = context_->create<Constant>(name);
            str = context_->createAtEnd<Assign>(block_, str, getTmpName());
            context_->createAtEnd<SetIndex>(block_, table, str, expr);
        }
        else
        {
            tryToCatchID(name);
            Value *id = cfg_->readVariableDef(name, block_);
            Value *cons = context_->create<Constant>(-1);
            cons = context_->createAtEnd<Assign>(block_, cons, getTmpName());
            context_->createAtEnd<SetIndex>(block_, table, cons, id);
        }
    }

    Value *Parser::tableDecl()
    {
        Value *table = context_->create<Table>();
        table = context_->createAtEnd<Assign>(
            block_, table, getTmpName("Table_"));
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

        Symbol symbol;
        if (table_->find(name, symbol) != Symbol::None)
        {
            diag_.redefineAs(std::string("function"), lexer_.getCoord());
        }
        symbol.beCaught_ = false;
        symbol.caught_ = false;
        symbol.param_ = false;
        symbol.tok_ = token_;
        symbol.type_ = Symbol::Define;
        table_->insert(name, symbol);

        // create function and generate parallel invoke.
        IRFunction *function = module_.createFunction(name);
        Value *invoke = context_->createAtEnd<Invoke>(
            block_, name, std::vector<Value*>(), name);
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

        BasicBlock *save = block_ = cfg_->createBasicBlock(name + "_entry");

        LoadParamsIntoTable(table_, function->getContext(), block_, params);
        function->setEntry(block_);
        block();
        function->setEnd(block_);

        cfg_->sealOthersBlock();
        
        block_ = oldBlock;
        cfg_ = cfg;
        context_ = context;
        table_ = table;
    }

    void Parser::letDecl()
    {
        match(TK_Let);
        string name = exceptIdentifier();
        Symbol symbol;
        if (table_->find(name, symbol) != Symbol::None)
        {
            diag_.redefineAs(std::string("binding"), lexer_.getCoord());
        }
        symbol.beCaught_ = false;
        symbol.caught_ = false;
        symbol.param_ = false;
        symbol.tok_ = token_;
        symbol.type_ = Symbol::Let;
        table_->insert(name, symbol);
        match(TK_Assign);

        Value *expr = orExpr();
        Value *let = context_->createAtEnd<Store>(
            block_, expr, cfg_->phiName(name));
        cfg_->saveVariableDef(name, block_, let);
        match(TK_Semicolon);
    }

    void Parser::defineDecl()
    {
        match(TK_Define);
        string name = exceptIdentifier();
        Symbol symbol;
        if (table_->find(name, symbol) != Symbol::None)
        {
            diag_.redefineAs(std::string("constant"), lexer_.getCoord());
        }
        symbol.beCaught_ = false;
        symbol.caught_ = false;
        symbol.param_ = false;
        symbol.tok_ = token_;
        symbol.type_ = Symbol::Define;
        table_->insert(name, symbol);

        match(TK_Assign);

        // match expression and save variable def.
        Value *expr = orExpr();
        Value *define = context_->createAtEnd<Store>(
            block_, expr, cfg_->phiName(name));
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
        BasicBlock *entry = module_.createBasicBlock("entry");

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
            diag_.except(tok, token_.kind_, lexer_.getCoord());
        }
        advance();
    }

    std::string Parser::exceptIdentifier()
    {
        if (token_.kind_ != TK_Identifier)
        {
            diag_.except(TK_Identifier, token_.kind_, lexer_.getCoord());
        }
        string value = std::move(token_.value_);
        advance();
        return std::move(value);
    }
}
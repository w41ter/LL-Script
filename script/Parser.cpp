#include "Parser.h"

#include <iostream>
#include <sstream>

#include "CFG.h"
#include "Value.h"
#include "IRContext.h"
#include "IRModule.h"
#include "Instruction.h"
#include "DiagnosisConsumer.h"

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

    unsigned TranslateRelationalToBinaryOps(unsigned tok)
    {
        switch (tok)
        {
        case TK_Less:
            return BinaryOperator::Less;
        case TK_LessThan:
            return BinaryOperator::NotGreat;
        case TK_Great:
            return BinaryOperator::Great;
        case TK_GreatThan:
            return BinaryOperator::NotLess;
        case TK_NotEqual:
            return BinaryOperator::NotEqual;
        case TK_EqualThan:
            return BinaryOperator::Equal;
        default:
            assert(0);
            return 0;
        }
    }
}

    bool Parser::tryToCatchID(scope_iterator iter, std::string &name)
    {
        if (functionStack.rend() == iter) {
            return false;
        }
        if (iter->symbolTable.count(name)) {
            return true;
        }
        // insert into symbol table and capture it.
        iter->captures.insert(name);
        iter->symbolTable.insert(std::pair<std::string, 
            unsigned>{name, FunctionScope::Let});
		// save it to SSA form
		Value *param = IRContext::create<Param>(name);
		iter->cfg_->saveVariableDef(
			name, iter->cfg_->getEntryBlock(), param);
        return tryToCatchID(++iter, name);
    }

    bool Parser::tryToCatchID(std::string &name)
    {
		if (isExistsInScope(name))
            return true;
        
        if (!tryToCatchID(functionStack.rbegin(), name)) {
            diag_.undefineID(name, lexer_.getCoord());
            return false;
        }
        return true;
    }

    bool Parser::isRelational(unsigned tok)
    {
        return (tok == TK_Less || tok == TK_LessThan ||
            tok == TK_Great || tok == TK_GreatThan ||
            tok == TK_NotEqual || tok == TK_EqualThan);
    }

    void Parser::initialize()
    {
        std::map<std::string, unsigned> keywords = {
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

    Value *Parser::parseVariable()
    {
        switch (token_.kind_)
        {
        case TK_Identifier:
        {  
            std::string name = token_.value_;
            tryToCatchID(name);
            advance();
			return scope->cfg_->readVariableDef(
                name, scope->block_);
        }
        case TK_LParen:
        {
            advance();
            Value * result = parseRightHandExpr();
            match(TK_RParen);
            return result;
        }
        case TK_Lambda:
        {
            return parseLambdaDecl();
        }
        default:
            diag_.unexceptedToken(token_.kind_, lexer_.getCoord());
            advance();
            return nullptr;
        }
    }

    Value *Parser::parseSuffixCommon(Value *value)
    {
        Value *result = value;
        while (true)
        {
            switch (token_.kind_)
            {
            case TK_LSquareBrace:
            {
                advance();
                Value *expr = parseRightHandExpr();
                match(TK_RSquareBrace);
                result = IRContext::createAtEnd<Index>(
                    scope->block_, result, expr, getTmpName());
                break;
            }
            case TK_LParen:
            {
                advance();
                std::vector<Value*> params;
                if (token_.kind_ != TK_RParen)
                {
                    params.push_back(parseRightHandExpr());
                    while (token_.kind_ == TK_Comma)
                    {
                        advance();
                        params.push_back(parseRightHandExpr());
                    }
                }

                match(TK_RParen);
                result = IRContext::createAtEnd<Invoke>(
                    scope->block_, result, params, getTmpName());
                break;
            }
            case TK_Period:
            {
                advance();
                std::string name = exceptIdentifier();
                Value *rhs = IRContext::create<Constant>(name);
                rhs = IRContext::createAtEnd<Assign>(
                    scope->block_, rhs, getTmpName());
                result = IRContext::createAtEnd<Index>(
                    scope->block_, result, rhs, getTmpName());
                break;
            }
            default:
                return result;
            }
        }
    }

    Value *Parser::parseVariableSuffix()
    {
        Value *result = parseVariable();
        return parseSuffixCommon(result);
    }

    Value *Parser::parseValue()
    {
        switch (token_.kind_)
        {
        case TK_LitFloat:
        {
            float value = token_.fnum_;
            advance();
            Value *val = IRContext::create<Constant>(value);
            return IRContext::createAtEnd<Assign>(
				scope->block_, val, getTmpName());
        }
        case TK_LitString:
        {
            std::string str = token_.value_;
            advance();
            Value *val = IRContext::create<Constant>(str);
            return IRContext::createAtEnd<Assign>(
				scope->block_, val, getTmpName());
        }
        case TK_LitInteger:
        {
            int integer = token_.num_;
            advance();
            Value *val = IRContext::create<Constant>(integer);
            return IRContext::createAtEnd<Assign>(
				scope->block_, val, getTmpName());
        }
        case TK_LitCharacter:
        {
            char c = token_.value_[0];
            advance();
            Value *val = IRContext::create<Constant>(c);
            return IRContext::createAtEnd<Assign>(
				scope->block_, val, getTmpName());
        }
        case TK_True:
        {
            advance();
            Value *val = IRContext::create<Constant>(true);
            return IRContext::createAtEnd<Assign>(
				scope->block_, val, getTmpName());
        }
        case TK_False:
        {
            advance();
            Value *val = IRContext::create<Constant>(false);
            return IRContext::createAtEnd<Assign>(
				scope->block_, val, getTmpName());
        }
        case TK_Null:
        {
            advance();
            Value *val = IRContext::create<Constant>();
            return IRContext::createAtEnd<Assign>(
				scope->block_, val, getTmpName());
        }
        case TK_LSquareBrace:
        {
            // let b = [a][a] or ['a' = 1].a is wrong.
            return parseTableDecl();
        }
        default:
            return parseVariableSuffix();
        }
    }

    Value *Parser::parseNotExpr()
    {
        if (token_.kind_ != TK_Not)
        {
            return parseValue();
        }
        advance();
        return IRContext::createAtEnd<NotOp>(
			scope->block_, parseValue(), getTmpName());
    }

    Value *Parser::parseNegativeExpr()
    {
        if (token_.kind_ != TK_Sub)
        {
            return parseNotExpr();
        }
        advance();
        Value *zero = IRContext::create<Constant>(0);
		zero = IRContext::createAtEnd<Assign>(
			scope->block_, zero, getTmpName());
        return IRContext::createAtEnd<BinaryOperator>(
			scope->block_, BinaryOperator::Sub, zero, 
            parseNotExpr(), getTmpName());
    }

    Value *Parser::parseMulAndDivExpr()
    {
        Value *result = parseNegativeExpr();
        while (token_.kind_ == TK_Mul || token_.kind_ == TK_Div)
        {
            auto op = token_.kind_ == TK_Mul 
                ? BinaryOperator::Mul : BinaryOperator::Div;
            advance();
            result = IRContext::createAtEnd<BinaryOperator>(
				scope->block_, op, result, parseNegativeExpr(),
                getTmpName());
        }
        return result;
    }

    Value *Parser::parseAddAndSubExpr()
    {
        Value *result = parseMulAndDivExpr();
        while (token_.kind_ == TK_Plus || token_.kind_ == TK_Sub)
        {
            auto op = token_.kind_ == TK_Plus
                ? BinaryOperator::Add : BinaryOperator::Sub;
            advance();
            result = IRContext::createAtEnd<BinaryOperator>(
				scope->block_, op, result, parseMulAndDivExpr(),
                getTmpName());
        }
        return result;
    }

    Value *Parser::parseRelationalExpr()
    {
        Value *result = parseAddAndSubExpr();
        while (isRelational(token_.kind_))
        {
            auto op = TranslateRelationalToBinaryOps(token_.kind_);
            advance();
            result = IRContext::createAtEnd<BinaryOperator>(
				scope->block_, op, result, parseAddAndSubExpr(), 
                getTmpName());
        }
        return result;
    }

    Value *Parser::parseAndExpr()
    {
        Value *result = parseRelationalExpr();
        if (token_.kind_ != TK_And)
            return result;

        BasicBlock *tmpBlock = scope->block_;
        BasicBlock *trueBlock = scope->cfg_->createBasicBlock(
            getTmpName("true_expr_"));
        BasicBlock *falseBlock = scope->cfg_->createBasicBlock(
            getTmpName("false_expr_"));

		IRContext::createBranchAtEnd(
            tmpBlock, result, falseBlock, trueBlock);
        while (token_.kind_ == TK_And)
        {
            advance();
			scope->block_ = trueBlock;
            Value *expr = parseRelationalExpr();
            trueBlock = scope->cfg_->createBasicBlock(getTmpName("true_expr_"));
			IRContext::createBranchAtEnd(
				scope->block_, result, falseBlock, trueBlock);
        }

        Value *true_ = IRContext::create<Constant>(true);
        Value *false_ = IRContext::create<Constant>(false);
        Value *trueVal = IRContext::createAtEnd<Assign>(
            trueBlock, true_, getTmpName());
        Value *falseVal = IRContext::createAtEnd<Assign>(
            falseBlock, false_, getTmpName());

        BasicBlock *endBlock = scope->cfg_->createBasicBlock(
            getTmpName("end_expr_"));
		IRContext::createGotoAtEnd(trueBlock, endBlock);
		IRContext::createGotoAtEnd(falseBlock, endBlock);
		scope->block_ = endBlock;
        auto params = { trueVal, falseVal };
        return IRContext::createAtEnd<Phi>(
			scope->block_, getTmpName(), params);
    }

    Value *Parser::parseOrExpr()
    {
        Value *result = parseAndExpr();
        if (token_.kind_ != TK_Or)
            return result;

        BasicBlock *tmpBlock = scope->block_;
        BasicBlock *trueBlock = scope->cfg_->createBasicBlock(
            getTmpName("true_expr_"));
        BasicBlock *falseBlock = scope->cfg_->createBasicBlock(
            getTmpName("false_expr_"));

		IRContext::createBranchAtEnd(
			tmpBlock, result, trueBlock, falseBlock);
        while (token_.kind_ == TK_Or)
        {
            advance();
			scope->block_ = falseBlock;
            Value *expr = parseAndExpr();
            falseBlock = scope->cfg_->createBasicBlock(
                getTmpName("false_expr_"));
			IRContext::createBranchAtEnd(
				scope->block_, result, trueBlock, falseBlock);
        }

        Value *true_ = IRContext::create<Constant>(true);
        Value *false_ = IRContext::create<Constant>(false);
        Value *trueVal = IRContext::createAtEnd<Assign>(
            trueBlock, true_, getTmpName());
        Value *falseVal = IRContext::createAtEnd<Assign>(
            falseBlock, false_, getTmpName());

        BasicBlock *endBlock = scope->cfg_->createBasicBlock(
            getTmpName("end_expr_"));
		IRContext::createGotoAtEnd(trueBlock, endBlock);
		IRContext::createGotoAtEnd(falseBlock, endBlock);
		scope->block_ = endBlock;
        auto params = { trueVal, falseVal };
        return IRContext::createAtEnd<Phi>(
			scope->block_, getTmpName(), params);
    }

    Value *Parser::parseRightHandExpr()
    {
        return parseOrExpr();
    }

    void Parser::parseAssignExpr()
    {
        std::string name = exceptIdentifier();
        tryToCatchID(name);
        Value *def = scope->cfg_->readVariableDef(
            name, scope->block_);
        Value *val = parseSuffixCommon(def);

        // ensure a; is wrong.
        if (val == def)
            match(TK_Assign);
        else if (token_.kind_ != TK_Assign)
            return;
        else
            advance();

        Value *RHS = parseRightHandExpr();

        // if the last inst of left is Index,
        // replace it with SetIndex.
        if (val != def && val->is_instr())
        {
            Instruction *instr = static_cast<Instruction*>(val);
            if (instr->is_index())
            {
                Index *index = static_cast<Index*>(instr);
                SetIndex *SI = IRContext::create<SetIndex>(
                    index->table(), index->index(), RHS);
                index->replace_with(SI);
                return;
            }
        }
        Value *result = IRContext::createAtEnd<Assign>(
            scope->block_, val, getTmpName());
        scope->cfg_->saveVariableDef(
            name, scope->block_, result);
    }

    void Parser::parseExpression()
    {
        parseAssignExpr();
        match(TK_Semicolon);
    }

    void Parser::parseBreakStat()
    {
        match(TK_Break);
        match(TK_Semicolon);

        if (breaks_.size() <= 0)
        {
            diag_.outOfScopeBreakOrContinue(lexer_.getCoord());
            return;
        }

		IRContext::createGotoAtEnd(scope->block_, breaks_.top());

		scope->cfg_->sealBlock(scope->block_);

		scope->block_ = scope->cfg_->createBasicBlock(
			getTmpName("full_throught_"));
    }

    void Parser::parseContinueStat()
    {
        match(TK_Continue);
        match(TK_Semicolon);
        
        if (continues_.size() <= 0)
        {
            diag_.outOfScopeBreakOrContinue(lexer_.getCoord());
            return;
        }

		IRContext::createGotoAtEnd(
			scope->block_, continues_.top());
        
		scope->cfg_->sealBlock(scope->block_);

		scope->block_ = scope->cfg_->createBasicBlock(
            getTmpName("full_throught_"));
    }

    void Parser::parseReturnStat()
    {
        advance();
        if (token_.kind_ != TK_Semicolon)
        {
            Value *expr = parseRightHandExpr();
			IRContext::createAtEnd<Return>(
				scope->block_, expr);
        }
        else
        {
			IRContext::createAtEnd<ReturnVoid>(
				scope->block_);
        }

        match(TK_Semicolon);
        BasicBlock *succBlock = 
            scope->cfg_->createBasicBlock("return_succ_");
		scope->cfg_->sealBlock(scope->block_);
		scope->block_ = succBlock;
    }

    void Parser::parseWhileStat()
    {
        advance();
        BasicBlock *tmpBlock = scope->block_;
        BasicBlock *condBlock = scope->cfg_->createBasicBlock(
            getTmpName("while_cond_"));
        BasicBlock *thenBlock = scope->cfg_->createBasicBlock(
            getTmpName("while_then_"));
        BasicBlock *endBlock = scope->cfg_->createBasicBlock(
            getTmpName("end_while_"));

		IRContext::createGotoAtEnd(tmpBlock, condBlock);
		scope->cfg_->sealBlock(tmpBlock);

		scope->block_ = condBlock;
        match(TK_LParen);
        Value *expr = parseRightHandExpr();
        match(TK_RParen);
		IRContext::createBranchAtEnd(
			scope->block_, expr, thenBlock, endBlock);

        breaks_.push(endBlock);
        continues_.push(condBlock);

		scope->block_ = thenBlock;
        parseStatement();
		IRContext::createGotoAtEnd(
			/*thenBlock==*/scope->block_, condBlock);
        
		scope->cfg_->sealBlock(thenBlock);
		scope->cfg_->sealBlock(scope->block_);
		scope->cfg_->sealBlock(condBlock);

		scope->block_ = endBlock;
        breaks_.pop();
        continues_.pop();
    }

    void Parser::parseIfStat()
    {
        advance();
        match(TK_LParen);
        Value *expr = parseRightHandExpr();
        match(TK_RParen);

		scope->cfg_->sealBlock(scope->block_);

        BasicBlock *tmpBlock = scope->block_;
        BasicBlock *thenBlock = scope->cfg_->createBasicBlock(
                getTmpName("if_then_"));

		scope->block_ = thenBlock;
        parseStatement();
        BasicBlock *thenEndBlock = scope->block_,
            *endBlock = nullptr;

        if (token_.kind_ == TK_Else)
        {
            advance();

            BasicBlock *elseBlock = 
				scope->cfg_->createBasicBlock(getTmpName("if_else_"));
			IRContext::createBranchAtEnd(
				tmpBlock, expr, thenBlock, elseBlock);

			scope->block_ = elseBlock;
            parseStatement();

			scope->cfg_->sealBlock(elseBlock);
			scope->cfg_->sealBlock(scope->block_);

            endBlock = scope->cfg_->createBasicBlock(getTmpName("if_end_"));
			IRContext::createGotoAtEnd(
				/*elseBlock==*/scope->block_, endBlock);
        }
        else
        {
            endBlock = scope->cfg_->createBasicBlock(getTmpName("if_end_"));
			IRContext::createBranchAtEnd(
				tmpBlock, expr, thenBlock, endBlock);
        }

		scope->cfg_->sealBlock(thenBlock);
		scope->cfg_->sealBlock(thenEndBlock);

		IRContext::createGotoAtEnd(thenEndBlock, endBlock);
		scope->block_ = endBlock;
    }

    void Parser::parseStatement()
    {
        switch (token_.kind_)
        {
        case TK_If:
            return parseIfStat();
        case TK_While:
            return parseWhileStat();
        case TK_Return:
            return parseReturnStat();
        case TK_Break:
            return parseBreakStat();
        case TK_Continue:
            return parseContinueStat();
        case TK_Let:
            return parseLetDecl();
        case TK_Define:
            return parseDefineDecl();
        case TK_Semicolon:
            return parseStatement();
        default:
            parseExpression();
            return ;
        }
    }

    void Parser::parseBlock()
    {
        match(TK_LCurlyBrace);
        while (token_.kind_ != TK_RCurlyBrace)
        {
            parseStatement();
        }
        advance();
    }

    void Parser::parseTableOthers(Value *table)
    {
        Constant *cons = nullptr;
        if (token_.kind_ == TK_LitCharacter)
        {
            cons = IRContext::create<Constant>(token_.value_[0]);
        }
        else if (token_.kind_ == TK_LitFloat)
        {
            cons = IRContext::create<Constant>(token_.fnum_);
        }
        else if (token_.kind_ == TK_LitInteger)
        {
            cons = IRContext::create<Constant>(token_.num_);
        }
        else if (token_.kind_ == TK_LitString)
        {
            cons = IRContext::create<Constant>(token_.value_);
        }
        else
        {
            diag_.unknowTableDecl(lexer_.getCoord());
            advance();
            return;
        }

        if (cons->type() == Constant::Integer
            && cons->getInteger() < 0)
        {
            diag_.indexLessThanZero(lexer_.getCoord());
        }

        advance();
        if (token_.kind_ == TK_Assign)
        {
            advance();
            Value *expr = parseRightHandExpr();
            Value *tmp = IRContext::createAtEnd<Assign>(
				scope->block_, cons, getTmpName());
			IRContext::createAtEnd<SetIndex>(
				scope->block_, table, tmp, expr);
        }
        else
        {
            Value *tmp = IRContext::create<Constant>(-1);
            tmp = IRContext::createAtEnd<Assign>(
				scope->block_, tmp, getTmpName());
            Value *tmpCons = IRContext::createAtEnd<Assign>(
				scope->block_, cons, getTmpName());
			IRContext::createAtEnd<SetIndex>(
				scope->block_, table, tmp, tmpCons);
        }
    }

    void Parser::parseTableIdent(Value *table)
    {
        std::string name = exceptIdentifier();
        if (token_.kind_ == TK_Assign)
        {
            // name = lambda ... == "name" = lambda
            advance();
            Value *expr = parseRightHandExpr();
            Value *str = IRContext::create<Constant>(name);
            str = IRContext::createAtEnd<Assign>(
				scope->block_, str, getTmpName());
			IRContext::createAtEnd<SetIndex>(
				scope->block_, table, str, expr);
        }
        else
        {
            tryToCatchID(name);
            Value *id = scope->cfg_->readVariableDef(name, scope->block_);
            Value *cons = IRContext::create<Constant>(-1);
            cons = IRContext::createAtEnd<Assign>(
				scope->block_, cons, getTmpName());
			IRContext::createAtEnd<SetIndex>(
				scope->block_, table, cons, id);
        }
    }

    Value *Parser::parseTableDecl()
    {
        assert(token_.kind_ == TK_LSquareBrace);

        Value *table = IRContext::create<Table>();
        table = IRContext::createAtEnd<Assign>(
			scope->block_, table, getTmpName("Table_"));

        do {
            advance();
            if (token_.kind_ == TK_Identifier)
                parseTableIdent(table);
            else if (token_.kind_ != TK_RSquareBrace)
                parseTableOthers(table);
            else
                break;
        } while (token_.kind_ == TK_Comma);
        match(TK_RSquareBrace);
        return table;
    }

	//
	// "("[param_list] ")"
	// 
	void Parser::parseParams(Strings &params)
	{
		match(TK_LParen);

		if (token_.kind_ == TK_Identifier)
		{
			params.push_back(token_.value_);
			advance();
			while (token_.kind_ == TK_Comma)
			{
				advance();
				params.push_back(exceptIdentifier());
			}
		}

		match(TK_RParen);
	}

	Value *Parser::parseLambdaDecl()
	{
		match(TK_Lambda);
		std::string name = getTmpName("$lambda_");
		defineIntoScope(name, FunctionScope::Define);

		return parseFunctionCommon(name);
	}

    void Parser::getFunctionParamsAndBody(
		Strings &params, IRFunction *function)
    {
		// match params
		parseParams(params);
		for (auto &str : params) {
			defineIntoScope(str, FunctionScope::Let);
			Value *param = IRContext::create<Param>(str);
			scope->cfg_->saveVariableDef(str, scope->block_, param);
		}

        parseBlock();

		scope->cfg_->sealOthersBlock();
    }

    void Parser::getFunctionPrototype(
        Strings &prototype, const Strings &params)
    {
        auto &captures = scope->captures;
        prototype.clear();
        prototype.reserve(captures.size() + params.size());
        for (auto &str : captures) {
            prototype.push_back(str);
        }
        
        for (auto &str : params) 
            prototype.push_back(str);
    }

	// Must be called after pop function scope.
    Value *Parser::createClosureForFunction(
		const std::string &name,
		std::unordered_set<std::string> &captures)
    {
        std::vector<Value*> paramsVals;
        for (auto &str : captures) {
			Value *val = scope->cfg_->readVariableDef(str, scope->block_);
			val = IRContext::createAtEnd<Assign>(
				scope->block_, val, getTmpName());
            paramsVals.push_back(val);
        }
        
        Value *funcval = IRContext::create<Function>(name);
		funcval = IRContext::createAtEnd<Assign>(
			scope->block_, funcval, getTmpName());
        Value *invoke = IRContext::createAtEnd<Invoke>(
			scope->block_, funcval, paramsVals, getTmpName());
		Value *func = IRContext::createAtEnd<Store>(
			scope->block_, invoke, scope->cfg_->phiName(name));
		scope->cfg_->saveVariableDef(name, scope->block_, func);
        return invoke;
    }

	Value *Parser::parseFunctionCommon(const std::string &name)
	{
		// create function and generate parallel invoke.
		IRFunction *function = module_.createFunction(name);
		pushFunctionScopeAndInit(function);

		Strings params;
		getFunctionParamsAndBody(params, function);

		// save current captures.
		std::vector<std::string> prototype;
		getFunctionPrototype(prototype, params);
		function->setParams(std::move(prototype));

		std::unordered_set<std::string> captures;
		std::swap(captures, scope->captures);
		popFunctionScope(function);

		// create closure for function.
		return createClosureForFunction(name, captures);
	}

    void Parser::parseFunctionDecl()
    {
        match(TK_Function);
        std::string name = exceptIdentifier();
        if (isExistsInScope(name)) {
            diag_.redefineAs(std::string("function"), lexer_.getCoord());
        }
        defineIntoScope(name, FunctionScope::Define);

		parseFunctionCommon(name);
    }

	void Parser::parseLetDefineCommon(
		const std::string &name)
	{
		// match expression and save variable def.
		Value *expr = parseRightHandExpr();
		Value *define = IRContext::createAtEnd<Assign>(
			scope->block_, expr, scope->cfg_->phiName(name));
		scope->cfg_->saveVariableDef(name, scope->block_, define);
		match(TK_Semicolon);
	}

    void Parser::parseLetDecl()
    {
        match(TK_Let);
        std::string name = exceptIdentifier();
        if (isExistsInScope(name)) {
            diag_.redefineAs(std::string("binding"), lexer_.getCoord());
        }
        defineIntoScope(name, FunctionScope::Let);
        match(TK_Assign);

		parseLetDefineCommon(name);
    }

    void Parser::parseDefineDecl()
    {
        match(TK_Define);
        std::string name = exceptIdentifier();
        if (isExistsInScope(name)) {
            diag_.redefineAs(std::string("binding"), lexer_.getCoord());
        }
        defineIntoScope(name, FunctionScope::Define);

        match(TK_Assign);
		
		parseLetDefineCommon(name);
    }

    void Parser::parse()
    {
		IRFunction *mainfunc = module_.createFunction("$main");
		pushFunctionScopeAndInit(mainfunc);

        advance();
        while (token_.kind_ != TK_EOF)
        {
			parseStatement();
        }

		scope->cfg_->sealOthersBlock();
		popFunctionScope(mainfunc);
    }

    Parser::Parser(Lexer & lexer, IRModule &module, DiagnosisConsumer &diag) 
        : lexer_(lexer), module_(module), diag_(diag), scope(nullptr)
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
        std::string value = std::move(token_.value_);
        advance();
        return std::move(value);
    }

    void Parser::pushFunctionScope() 
    {
        functionStack.push_back(FunctionScope{});
		scope = &functionStack.back();
    }

	void Parser::pushFunctionScopeAndInit(IRFunction *func)
	{
		pushFunctionScope();
		scope->cfg_ = func;
		scope->block_ = scope->cfg_->createBasicBlock(
			func->getName() + "_entry");
		func->setEntry(scope->block_);
	}

    void Parser::popFunctionScope() 
    {
        functionStack.pop_back();
		if (functionStack.size() == 0)
			scope = nullptr;
		else 
			scope = &functionStack.back();
    }

	void Parser::popFunctionScope(IRFunction *func)
	{
		func->setEnd(scope->block_);
		popFunctionScope();
	}

    void Parser::defineIntoScope(const std::string &str, unsigned type) 
    {
		scope->symbolTable.insert(
			std::pair<std::string, unsigned>{str, type});
    }

    void Parser::insertIntoScope(const std::string &str, unsigned type)
    {
		scope->upperTable.insert(
			std::pair<std::string, unsigned>{str, type});
    }

    bool Parser::isDefineInScope(const std::string &str)
    {
        return scope->symbolTable.count(str);
    }

    bool Parser::isExistsInScope(const std::string &str) 
    {
        return isDefineInScope(str);
    }
}
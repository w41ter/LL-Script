#include "Parser.h"

#include <iostream>
#include <sstream>

#include "CFG.h"
#include "Value.h"
#include "IRContext.h"
#include "IRModule.h"
#include "Instruction.h"
#include "Diagnosis.h"
#include "DiagnosisConsumer.h"

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
		Value *param = iter->context_->create<Param>(name);
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
        tryToCatchID(name);
        Value *val = scope->cfg_->readVariableDef(
			name, scope->block_);
        while (true)
        {
            switch (token_.kind_)
            {
            case TK_LParen:
            {
                advance();
                std::vector<Value*> params;
                if (token_.kind_ != TK_RParen)
                {
                    params.push_back(rightHandExpr());
                    while (token_.kind_ == TK_Comma)
                    {
                        advance();
                        params.push_back(rightHandExpr());
                    }
                }

                match(TK_RParen);
                val = scope->context_->create<Invoke>(
                    val, params, getTmpName());
                lhs.push_back(val);
                break;
            }
            case TK_RParen:
            {
                advance();
                Value *expr = rightHandExpr();
                match(TK_RSquareBrace);
                // merge Index and Assign to SetIndex
                val = scope->context_->create<Index>(
                    val, expr, getTmpName());
                lhs.push_back(val);
                break;
            }
            case TK_Period:
            {
                advance();
                string name = exceptIdentifier();
                Value *rhs = scope->context_->create<Constant>(name);
                rhs = scope->context_->create<Assign>(rhs, getTmpName());
                val = scope->context_->create<Index>(val, rhs, getTmpName());
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
            tryToCatchID(name);
            advance();
            return scope->cfg_->readVariableDef(
				name, scope->block_);
        }
        case TK_LParen:
        {
            advance();
            Value * result = rightHandExpr();
            match(TK_RParen);
            return result;
        }
        case TK_Lambda:
        {
            return lambdaDecl();
        }
        default:
            diag_.unexceptedToken(token_.kind_, lexer_.getCoord());
            advance();
            return nullptr;
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
                Value *expr = rightHandExpr();
                match(TK_RSquareBrace);
                result = scope->context_->createAtEnd<Index>(
					scope->block_, result, expr, getTmpName());
				break;
            }
            case TK_LParen:
            {
                advance();
                std::vector<Value*> params;
                if (token_.kind_ != TK_RParen)
                {
                    params.push_back(rightHandExpr());
                    while (token_.kind_ == TK_Comma)
                    {
                        advance();
                        params.push_back(rightHandExpr());
                    }
                }

                match(TK_RParen);
                result = scope->context_->createAtEnd<Invoke>(
					scope->block_, result, params, getTmpName());
				break;
            }
            case TK_Period:
            {
                advance();
                string name = exceptIdentifier();
                Value *rhs = scope->context_->create<Constant>(name);
                rhs = scope->context_->createAtEnd<Assign>(
					scope->block_, rhs, getTmpName());
                result = scope->context_->createAtEnd<Index>(
					scope->block_, result, rhs, getTmpName());
				break;
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
            Value *val = scope->context_->create<Constant>(value);
            return scope->context_->createAtEnd<Assign>(
				scope->block_, val, getTmpName());
        }
        case TK_LitString:
        {
            string str = token_.value_;
            advance();
            Value *val = scope->context_->create<Constant>(str);
            return scope->context_->createAtEnd<Assign>(
				scope->block_, val, getTmpName());
        }
        case TK_LitInteger:
        {
            int integer = token_.num_;
            advance();
            Value *val = scope->context_->create<Constant>(integer);
            return scope->context_->createAtEnd<Assign>(
				scope->block_, val, getTmpName());
        }
        case TK_LitCharacter:
        {
            char c = token_.value_[0];
            advance();
            Value *val = scope->context_->create<Constant>(c);
            return scope->context_->createAtEnd<Assign>(
				scope->block_, val, getTmpName());
        }
        case TK_True:
        {
            advance();
            Value *val = scope->context_->create<Constant>(true);
            return scope->context_->createAtEnd<Assign>(
				scope->block_, val, getTmpName());
        }
        case TK_False:
        {
            advance();
            Value *val = scope->context_->create<Constant>(false);
            return scope->context_->createAtEnd<Assign>(
				scope->block_, val, getTmpName());
        }
        case TK_Null:
        {
            advance();
            Value *val = scope->context_->create<Constant>();
            return scope->context_->createAtEnd<Assign>(
				scope->block_, val, getTmpName());
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
        return scope->context_->createAtEnd<NotOp>(
			scope->block_, value(), getTmpName());
    }

    Value *Parser::negativeExpr()
    {
        if (token_.kind_ != TK_Sub)
        {
            return notExpr();
        }
        advance();
        Value *zero = scope->context_->create<Constant>(0);
        return scope->context_->createAtEnd<BinaryOperator>(
			scope->block_, BinaryOperator::Sub, zero, notExpr(), getTmpName());
    }

    Value *Parser::mulAndDivExpr()
    {
        Value *result = negativeExpr();
        while (token_.kind_ == TK_Mul || token_.kind_ == TK_Div)
        {
            auto op = token_.kind_ == TK_Mul 
                ? BinaryOperator::Mul : BinaryOperator::Div;
            advance();
            result = scope->context_->createAtEnd<BinaryOperator>(
				scope->block_, op, result, negativeExpr(), getTmpName());
        }
        return result;
    }

    Value *Parser::addAndSubExpr()
    {
        Value *result = mulAndDivExpr();
        while (token_.kind_ == TK_Plus || token_.kind_ == TK_Sub)
        {
            auto op = token_.kind_ == TK_Plus
                ? BinaryOperator::Add : BinaryOperator::Sub;
            advance();
            result = scope->context_->createAtEnd<BinaryOperator>(
				scope->block_, op, result, mulAndDivExpr(), getTmpName());
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
            result = scope->context_->createAtEnd<BinaryOperator>(
				scope->block_, op, result, addAndSubExpr(), getTmpName());
        }
        return result;
    }

    Value *Parser::andExpr()
    {
        Value *result = relationalExpr();
        if (token_.kind_ != TK_And)
            return result;

        BasicBlock *tmpBlock = scope->block_,
            *trueBlock = scope->cfg_->createBasicBlock(getTmpName("true_expr_")),
            *falseBlock = scope->cfg_->createBasicBlock(getTmpName("false_expr_"));

		scope->context_->createBranchAtEnd(tmpBlock, result, falseBlock, trueBlock);
        while (token_.kind_ == TK_And)
        {
            advance();
			scope->block_ = trueBlock;
            Value *expr = relationalExpr();
            trueBlock = scope->cfg_->createBasicBlock(getTmpName("true_expr_"));
			scope->context_->createBranchAtEnd(
				scope->block_, result, falseBlock, trueBlock);
        }

        Value *true_ = scope->context_->create<Constant>(true);
        Value *false_ = scope->context_->create<Constant>(false);
        Value *trueVal = scope->context_->createAtEnd<Assign>(trueBlock, true_, getTmpName());
        Value *falseVal = scope->context_->createAtEnd<Assign>(falseBlock, false_, getTmpName());

        BasicBlock *endBlock = scope->cfg_->createBasicBlock(getTmpName("end_expr_"));
		scope->context_->createGotoAtEnd(trueBlock, endBlock);
		scope->context_->createGotoAtEnd(falseBlock, endBlock);
		scope->block_ = endBlock;
        auto params = { trueVal, falseVal };
        return scope->context_->createAtEnd<Phi>(
			scope->block_, getTmpName(), params);
    }

    Value *Parser::orExpr()
    {
        Value *result = andExpr();
        if (token_.kind_ != TK_Or)
            return result;

        BasicBlock *tmpBlock = scope->block_,
            *trueBlock = scope->cfg_->createBasicBlock(getTmpName("true_expr_")),
            *falseBlock = scope->cfg_->createBasicBlock(getTmpName("false_expr_"));

		scope->context_->createBranchAtEnd(
			tmpBlock, result, trueBlock, falseBlock);
        while (token_.kind_ == TK_Or)
        {
            advance();
			scope->block_ = falseBlock;
            Value *expr = andExpr();
            falseBlock = scope->cfg_->createBasicBlock(getTmpName("false_expr_"));
			scope->context_->createBranchAtEnd(
				scope->block_, result, trueBlock, falseBlock);
        }

        Value *true_ = scope->context_->create<Constant>(true);
        Value *false_ = scope->context_->create<Constant>(false);
        Value *trueVal = scope->context_->createAtEnd<Assign>(trueBlock, true_, getTmpName());
        Value *falseVal = scope->context_->createAtEnd<Assign>(falseBlock, false_, getTmpName());

        BasicBlock *endBlock = scope->cfg_->createBasicBlock(getTmpName("end_expr_"));
		scope->context_->createGotoAtEnd(trueBlock, endBlock);
		scope->context_->createGotoAtEnd(falseBlock, endBlock);
		scope->block_ = endBlock;
        auto params = { trueVal, falseVal };
        return scope->context_->createAtEnd<Phi>(
			scope->block_, getTmpName(), params);
    }

    Value *Parser::rightHandExpr() 
    {
        return orExpr();
    }

    Value *Parser::assignExpr()
    {
        auto coord = lexer_.getCoord();
        std::list<Value*> lhs;
        
        // get left val.
        std::string name = LHS(lhs);
        if (token_.kind_ == TK_Assign)
        {
            advance();
            Value *rhs = rightHandExpr();
            
            if (lhs.size() != 0)
            {
                Value *lastL = lhs.back();
                if (!lastL->is_value())
                {
                    Instruction *instr = (Instruction*)lastL;
                    if (instr->is_index())
                    {
                        Index *si = (Index*)lastL;
                        lhs.pop_back();
                        lhs.push_back(scope->context_->create<SetIndex>(
                            si->table(), si->index(), rhs));
                    }
                }
            }
            else 
            {
                Value *result = scope->context_->create<Store>(
					rhs, scope->cfg_->phiName(name));
				scope->cfg_->saveVariableDef(
					name, scope->block_, result);
                lhs.push_back(result);
            }
            
            for (auto *i : lhs)
            {
                Instruction *instr = (Instruction*)i;
				scope->block_->push_back(instr);
                instr->set_parent(scope->block_);
            }
        }
        if (lhs.size() > 0)
            return lhs.back();
        else
            return scope->cfg_->readVariableDef(name, scope->block_);
    }

    void Parser::expression()
    {
        assignExpr();
        match(TK_Semicolon);
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

		scope->context_->createGotoAtEnd(scope->block_, breaks_.top());

		scope->cfg_->sealBlock(scope->block_);

		scope->block_ = scope->cfg_->createBasicBlock(
			getTmpName("full_throught_"));
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

		scope->context_->createGotoAtEnd(
			scope->block_, continues_.top());
        
		scope->cfg_->sealBlock(scope->block_);

		scope->block_ = scope->cfg_->createBasicBlock(getTmpName("full_throught_"));
    }

    void Parser::returnStat()
    {
        advance();
        if (token_.kind_ != TK_Semicolon)
        {
            Value *expr = rightHandExpr();
			scope->context_->createAtEnd<Return>(
				scope->block_, expr);
        }
        else
        {
			scope->context_->createAtEnd<ReturnVoid>(
				scope->block_);
        }

        match(TK_Semicolon);
        BasicBlock *succBlock = scope->cfg_->createBasicBlock("return_succ_");
		scope->cfg_->sealBlock(scope->block_);
		scope->block_ = succBlock;
    }

    void Parser::whileStat()
    {
        advance();
        BasicBlock *tmpBlock = scope->block_,
            *condBlock = scope->cfg_->createBasicBlock(getTmpName("while_cond_")),
            *thenBlock = scope->cfg_->createBasicBlock(getTmpName("while_then_")),
            *endBlock = scope->cfg_->createBasicBlock(getTmpName("end_while_"));

		scope->context_->createGotoAtEnd(tmpBlock, condBlock);
		scope->cfg_->sealBlock(tmpBlock);

		scope->block_ = condBlock;
        match(TK_LParen);
        Value *expr = rightHandExpr();
        match(TK_RParen);
		scope->context_->createBranchAtEnd(
			scope->block_, expr, thenBlock, endBlock);

        breaks_.push(endBlock);
        continues_.push(condBlock);

		scope->block_ = thenBlock;
        statement();
		scope->context_->createGotoAtEnd(
			/*thenBlock==*/scope->block_, condBlock);
        
		scope->cfg_->sealBlock(thenBlock);
		scope->cfg_->sealBlock(scope->block_);
		scope->cfg_->sealBlock(condBlock);

		scope->block_ = endBlock;
        breaks_.pop();
        continues_.pop();
    }

    void Parser::ifStat()
    {
        advance();
        match(TK_LParen);
        Value *expr = rightHandExpr();
        match(TK_RParen);

		scope->cfg_->sealBlock(scope->block_);

        BasicBlock *tmpBlock = scope->block_,
            *thenBlock = scope->cfg_->createBasicBlock(getTmpName("if_then_"));

		scope->block_ = thenBlock;
        statement();
        BasicBlock *thenEndBlock = scope->block_, *endBlock = nullptr;

        if (token_.kind_ == TK_Else)
        {
            advance();

            BasicBlock *elseBlock = 
				scope->cfg_->createBasicBlock(getTmpName("if_else_"));
			scope->context_->createBranchAtEnd(
				tmpBlock, expr, thenBlock, elseBlock);

			scope->block_ = elseBlock;
            statement();

			scope->cfg_->sealBlock(elseBlock);
			scope->cfg_->sealBlock(scope->block_);

            endBlock = scope->cfg_->createBasicBlock(getTmpName("if_end_"));
			scope->context_->createGotoAtEnd(
				/*elseBlock==*/scope->block_, endBlock);
        }
        else
        {
            endBlock = scope->cfg_->createBasicBlock(getTmpName("if_end_"));
			scope->context_->createBranchAtEnd(
				tmpBlock, expr, thenBlock, endBlock);
        }

		scope->cfg_->sealBlock(thenBlock);
		scope->cfg_->sealBlock(thenEndBlock);

		scope->context_->createGotoAtEnd(thenEndBlock, endBlock);
		scope->block_ = endBlock;
    }

    void Parser::statement()
    {
        switch (token_.kind_)
        {
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

    void Parser::tableOthers(Value *table)
    {
        Value *cons = nullptr;
        if (token_.kind_ == TK_LitCharacter)
        {
            cons = scope->context_->create<Constant>(token_.value_[0]);
        }
        else if (token_.kind_ == TK_LitFloat)
        {
            cons = scope->context_->create<Constant>(token_.fnum_);
        }
        else if (token_.kind_ == TK_LitInteger)
        {
            cons = scope->context_->create<Constant>(token_.num_);
        }
        else if (token_.kind_ == TK_LitString)
        {
            cons = scope->context_->create<Constant>(token_.value_);
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
            Value *expr = rightHandExpr();
            if (cons->get_subclass_id() == Value::ConstantVal)
            {
                Constant *innerCons =
                    static_cast<Constant*>(cons);
                if (innerCons->type() == Constant::Integer
                    && innerCons->getInteger() < 0)
                {
                    diag_.indexLessThanZero(lexer_.getCoord());
                }
            }
            cons = scope->context_->createAtEnd<Assign>(
				scope->block_, cons, getTmpName());
			scope->context_->createAtEnd<SetIndex>(
				scope->block_, table, cons, expr);
        }
        else
        {
            Value *tmp = scope->context_->create<Constant>(-1);
            tmp = scope->context_->createAtEnd<Assign>(
				scope->block_, tmp, getTmpName());
            cons = scope->context_->createAtEnd<Assign>(
				scope->block_, cons, getTmpName());
			scope->context_->createAtEnd<SetIndex>(
				scope->block_, table, tmp, cons);
        }
    }

    void Parser::tableIdent(Value *table)
    {
        string name = exceptIdentifier();
        if (token_.kind_ == TK_Assign)
        {
            // name = lambda ... == "name" = lambda
            advance();
            Value *expr = rightHandExpr();
            Value *str = scope->context_->create<Constant>(name);
            str = scope->context_->createAtEnd<Assign>(
				scope->block_, str, getTmpName());
			scope->context_->createAtEnd<SetIndex>(
				scope->block_, table, str, expr);
        }
        else
        {
            tryToCatchID(name);
            Value *id = scope->cfg_->readVariableDef(name, scope->block_);
            Value *cons = scope->context_->create<Constant>(-1);
            cons = scope->context_->createAtEnd<Assign>(
				scope->block_, cons, getTmpName());
			scope->context_->createAtEnd<SetIndex>(
				scope->block_, table, cons, id);
        }
    }

    Value *Parser::tableDecl()
    {
        Value *table = scope->context_->create<Table>();
        table = scope->context_->createAtEnd<Assign>(
			scope->block_, table, getTmpName("Table_"));
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
	//
	// "("[param_list] ")"
	// 
	void Parser::readParams(Strings &params)
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

	Value *Parser::lambdaDecl()
	{
		match(TK_Lambda);
		std::string name = getTmpName("lambda_");
		defineIntoScope(name, FunctionScope::Define);

		return functionCommon(name);
	}

    void Parser::functionParamsAndBody(
		Strings &params, IRFunction *function)
    {
		// match params
		readParams(params);
		for (auto &str : params) {
			defineIntoScope(str, FunctionScope::Let);
			Value *param = scope->context_->create<Param>(str);
			scope->cfg_->saveVariableDef(str, scope->block_, param);
		}

        block();

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
            paramsVals.push_back(val);
        }
        
        Value *funcval = scope->context_->create<Function>(name);
        Value *invoke = scope->context_->createAtEnd<Invoke>(
			scope->block_, funcval, paramsVals, name);
		scope->cfg_->saveVariableDef(name, scope->block_, invoke);
        return invoke;
    }

	Value *Parser::functionCommon(const std::string &name)
	{
		// create function and generate parallel invoke.
		IRFunction *function = module_.createFunction(name);
		pushFunctionScopeAndInit(function);

		Strings params;
		functionParamsAndBody(params, function);

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

    void Parser::functionDecl()
    {
        match(TK_Function);
        string name = exceptIdentifier();
        if (isExistsInScope(name)) {
            diag_.redefineAs(std::string("function"), lexer_.getCoord());
        }
        defineIntoScope(name, FunctionScope::Define);

		functionCommon(name);
    }

    void Parser::letDecl()
    {
        match(TK_Let);
        string name = exceptIdentifier();
        if (isExistsInScope(name)) {
            diag_.redefineAs(std::string("binding"), lexer_.getCoord());
        }
        defineIntoScope(name, FunctionScope::Let);
        match(TK_Assign);

        Value *expr = rightHandExpr();
        Value *let = scope->context_->createAtEnd<Store>(
			scope->block_, expr, scope->cfg_->phiName(name));
		scope->cfg_->saveVariableDef(name, scope->block_, let);
        match(TK_Semicolon);
    }

    void Parser::defineDecl()
    {
        match(TK_Define);
        string name = exceptIdentifier();
        if (isExistsInScope(name)) {
            diag_.redefineAs(std::string("binding"), lexer_.getCoord());
        }
        defineIntoScope(name, FunctionScope::Define);

        match(TK_Assign);

        // match expression and save variable def.
        Value *expr = rightHandExpr();
        Value *define = scope->context_->createAtEnd<Store>(
			scope->block_, expr, scope->cfg_->phiName(name));
		scope->cfg_->saveVariableDef(name, scope->block_, define);
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
        clear();
        BasicBlock *entry = module_.createBasicBlock("entry");

        scope->block_ = entry;
		scope->context_ = module_.getContext();
		scope->cfg_ = &module_;

        module_.setEntry(entry);

        advance();
        while (token_.kind_ != TK_EOF)
        {
            if (!topLevelDecl())
            {
                expression();
            }
        }

        module_.setEnd(scope->block_);

		scope->cfg_->sealOthersBlock();
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

    void Parser::pushFunctionScope() 
    {
        functionStack.push_back(FunctionScope{});
		scope = &functionStack.back();
    }

	void Parser::pushFunctionScopeAndInit(IRFunction *func)
	{
		pushFunctionScope();
		scope->context_ = func->getContext();
		scope->cfg_ = func;
		scope->block_ = scope->cfg_->createBasicBlock(
			func->getName() + "_entry");
		func->setEntry(scope->block_);
	}

    void Parser::popFunctionScope() 
    {
        functionStack.pop_back();
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

    void Parser::clear() 
    {
#define clear_stack(stack)          \
    do {                            \
        while (!(stack).empty()) {  \
            (stack).pop();          \
        }                           \
    } while (0)
        clear_stack(breaks_);
        clear_stack(continues_);
#undef clear_stack    

        functionStack.clear();

        pushFunctionScope();
    }
}
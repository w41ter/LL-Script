#include <map>
#include <string>
#include <iostream>

#include "Parser.h"

using std::map;
using std::string;

namespace script
{
    bool Parser::isRelational(unsigned tok)
    {
        return (tok == TK_Less || tok == TK_LessThan ||
            tok == TK_Great || tok == TK_GreatThan ||
            tok == TK_NotEqualThan || tok == TK_EqualThan);
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
        };
        for (auto i : keywords)
        {
            lexer_.registerKeyword(i.first, i.second);
        }
    }

    ASTree * Parser::parseKeywordConstant()
    {
        ASTree *expr = nullptr;
        if (token_.kind_ == TK_True)
            expr = MallocMemory<ASTConstant>(1);
        else if (token_.kind_ == TK_False)
            expr = MallocMemory<ASTConstant>(0);
        else if (token_.kind_ == TK_Null)
            expr = MallocMemory<ASTNull>();
        else
        {
            std::cout << "find " << token_.value_ << "(" << token_.coord_.lineNum_ << "," << token_.coord_.linePos_ << std::endl;
            throw std::runtime_error(" i dont konw");
        }
        advance();
        return expr;
    }

    //
    // factor:
    //    INT_CONST
    //    | CHAR_CONST
    //    | STRING_CONST
    //    | keyword_constant
    //    | ID
    //    | "(" expression ")"
    //    | "[" expression_list "]"
    //
    ASTree * Parser::parseFactor()
    {
        ASTree *expr = nullptr;
        switch (token_.kind_)
        {
        case TK_LitFloat: 
        {
            expr = MallocMemory<ASTConstant>(token_.fnum_);
            advance();
            break;
        }
        case TK_LitString:
        {
            expr = MallocMemory<ASTConstant>(token_.value_);
            advance();
            break;
        }
        case TK_LitInteger:
        {
            expr = MallocMemory<ASTConstant>(token_.num_);
            advance();
            break;
        }
        case TK_LitCharacter:
        {
            expr = MallocMemory<ASTConstant>(token_.value_[0]);
            advance();
            break;
        }
        case TK_Identifier:
        {
            expr = MallocMemory<ASTIdentifier>(token_.value_);
            advance();
            break;
        }
        case TK_LParen:
        {
            advance();
            expr = parseExpr();
            match(TK_RParen);
            break;
        }
        case TK_LSquareBrace:
        {
            advance();
            expr = MallocMemory<ASTArray>(parseExprList());
            match(TK_RSquareBrace);
            break;
        }
        default:
            expr = parseKeywordConstant();
            break;
        }
        return expr;
    }

    //
    // positive_factor:
    //  factor{ (["[" expression "]"] | ["(" expression_list ")"]) }
    // 
    ASTree * Parser::parsePositveFactor()
    {
        ASTree *expr = parseFactor();
        while (token_.kind_ == TK_LSquareBrace ||
            token_.kind_ == TK_LParen)
        {
            if (token_.kind_ == TK_LSquareBrace)
            {
                advance();
                expr = MallocMemory<ASTArrayIndex>(expr, parseExpr());
                match(TK_RSquareBrace);
            }
            else
            {
                advance();
                expr = MallocMemory<ASTCall>(expr, parseExprList());
                match(TK_RParen);
            }
        }
        return expr;
    }

    //
    // not_factor:
    //  ["!"] positive_factor
    //
    ASTree * Parser::parseNotFactor()
    {
        if (token_.kind_ == TK_Not)
        {
            advance();
            return MallocMemory<ASTSingleExpression>(
                ASTSingleExpression::OP_Not,
                parsePositveFactor()
                );
        }
        return parsePositveFactor();
    }

    //
    // term:
    //  ["-"] not_factor
    //
    ASTree * Parser::parseTerm()
    {
        if (token_.kind_ == TK_Sub)
        {
            advance();
            return MallocMemory<ASTSingleExpression>(
                ASTSingleExpression::OP_Sub,
                parseNotFactor()
                );
        }
        return parseNotFactor();
    }

    //
    // additive_expression:
    //  term{ ("*" | "/") term }
    //
    ASTree * Parser::parseAdditiveExpr()
    {
        ASTree *expr = parseTerm();
        while (token_.kind_ == TK_Mul || token_.kind_ == TK_Div)
        {
            advance();
            expr = MallocMemory<ASTBinaryExpression>(
                token_.kind_ == TK_Plus
                ? ASTBinaryExpression::OP_Mul
                : ASTBinaryExpression::OP_Div,
                expr,
                parseTerm()
                );
        }
        return expr;
    }

    //
    // relational_expression:
    //  additive_expression{ ("+" | "-") additive_expression }
    // 
    ASTree * Parser::parseRelationalExpr()
    {
        ASTree *expr = parseAdditiveExpr();
        while (token_.kind_ == TK_Plus || token_.kind_ == TK_Sub)
        {
            advance();
            expr = MallocMemory<ASTBinaryExpression>(
                token_.kind_ == TK_Plus 
                ? ASTBinaryExpression::OP_Add
                : ASTBinaryExpression::OP_Sub,
                expr,
                parseAdditiveExpr()
                );
        }
        return expr;
    }

    //
    // and_expression:
    //  relational_expression[("<" | ">" | ">=" | "<=" | "==" | "!=") relational_expression]
    // 
    ASTree * Parser::parseAndExpr()
    {
        ASTree *expr = parseRelationalExpr();
        if (isRelational(token_.kind_))
        {
            unsigned op;
            switch (token_.kind_)
            {
            case TK_Less:
                op = ASTRelationalExpression::RL_Less;
                break;
            case TK_LessThan:
                op = ASTRelationalExpression::RL_LessThan;
                break;
            case TK_Great:
                op = ASTRelationalExpression::RL_Great;
                break;
            case TK_GreatThan:
                op = ASTRelationalExpression::RL_GreatThan;
                break;
            case TK_NotEqualThan:
                op = ASTRelationalExpression::RL_NotEqual;
                break;
            case TK_EqualThan:
                op = ASTRelationalExpression::RL_Equal;
                break;
            }
            advance();
            expr = MallocMemory<ASTRelationalExpression>(
                expr, parseRelationalExpr()
                );
        }
        return expr;
    }

    //
    // or_expression:
    //  and_expression{ "&&" and_expression }
    //
    ASTree * Parser::parseOrExpr()
    {
        ASTree *expr = parseAndExpr();
        if (token_.kind_ != TK_And)
            return expr;
        ASTAndExpression *andExpr = MallocMemory<ASTAndExpression>();
        andExpr->push_back(expr);
        while (token_.kind_ == TK_And)
        {
            advance();
            andExpr->push_back(parseAndExpr());
        }
        return andExpr;
    }

    //
    // expression:
    //  or_expression{ "||" or_expression }
    //
    ASTree * Parser::parseExpr()
    {
        ASTree *expr = parseOrExpr();
        if (token_.kind_ != TK_Or)
            return expr;
        ASTOrExpression *orExpr = MallocMemory<ASTOrExpression>();
        orExpr->push_back(expr);
        while (token_.kind_ == TK_Or)
        {
            advance();
            orExpr->push_back(parseOrExpr());
        }
        return orExpr;
    }

    //
    // assign_expression:
    //  expression { "=" expression }
    //
    ASTree * Parser::parseAssignExpr()
    {
        ASTree *expr = parseExpr();
        while (token_.kind_ == TK_Assign)
        {
            advance();
            expr = MallocMemory<ASTAssignExpression>(expr, parseExpr());
        }
        return expr;
    }

    //
    // expression_list:
    //  expression { "," expression }
    //
    ASTree * Parser::parseExprList()
    {
        ASTExpressionList *list = MallocMemory<ASTExpressionList>();
        list->push_back(parseExpr());
        while (token_.kind_ == TK_Comma)
        {
            advance();
            list->push_back(parseExpr());
        }
        return list;
    }

    //
    // statement:
    //    block
    //    | "if" "(" assign_expression ")" statement["else" statement]
    //    | "while" "(" assign_expression ")" statement
    //    | "return"[assign_expression] ";"
    //    | "break" ";"
    //    | "continue" ";"
    //    | assign_expression ";"
    //    | var_decl
    //
    ASTree * Parser::parseStatement()
    {
        switch (token_.kind_)
        {
        case TK_LCurlyBrace:
            return parseBlock();
            break;
        case TK_If:
        {
            advance();
            match(TK_LParen);
            ASTree *condition = parseAssignExpr();
            match(TK_RParen);
            ASTree *ifState = parseStatement();
            if (token_.kind_ == TK_Else)
            {
                advance();
                ASTree *elseState = parseStatement();
                return MallocMemory<ASTIfStatement>(
                    condition, ifState, elseState
                    );
            }
            return MallocMemory<ASTIfStatement>(condition, ifState);
        }
        case TK_While:
        {
            advance();
            match(TK_LParen);
            ASTree *condition = parseAssignExpr();
            match(TK_RParen);
            ASTree *state = parseStatement();
            return MallocMemory<ASTWhileStatement>(condition, state);
        }
        case TK_Return:
        {    
            advance();
            ASTree *expr = nullptr;
            if (token_.kind_ != TK_Semicolon)
                expr = parseAssignExpr();
            
            match(TK_Semicolon);
            return MallocMemory<ASTReturnStatement>(expr);
        }
        case TK_Break:
        {
            advance();
            match(TK_Semicolon);
            return MallocMemory<ASTBreakStatement>();
        }
        case TK_Continue:
        {
            advance();
            match(TK_Semicolon);
            return MallocMemory<ASTContinueStatement>();
        }
        case TK_Let:
        {
            advance();
            string &name = exceptIdentifier();
            match(TK_Assign);
            ASTree *expr = parseAssignExpr();
            match(TK_Semicolon);
            return MallocMemory<ASTVarDeclStatement>(name, expr);
        }
        default:
            ASTree *expr = parseAssignExpr();
            match(TK_Semicolon);
            return expr;
        }
    }

    // 
    // block:
    //  "{" { statement } "}"
    //
    ASTBlock * Parser::parseBlock()
    {
        ASTBlock *block = MallocMemory<ASTBlock>();
        match(TK_LCurlyBrace);
        while (token_.kind_ != TK_RCurlyBrace)
        {
            block->push_back(parseStatement());
        }
        advance();
        return block;
    }

    //
    // param_list:
    //  ID{ ","  ID }
    //
    // function_decl:
    //  "function" ID "("[param_list] ")" block
    //
    ASTFunction * Parser::parseFunctionDecl()
    {
        advance();
        ASTFunction *function = MallocMemory<ASTFunction>(
            exceptIdentifier());
        match(TK_LParen);
        if (token_.kind_ == TK_Identifier)
        {
            function->push_param(token_.value_);
            advance();
            while (token_.kind_ == TK_Comma)
            {
                advance();
                function->push_param(exceptIdentifier());
            }
        }
        match(TK_RParen);
        function->setBlock(parseBlock());
        return function;
    }

    //
    // program: 
    //  { function_decl }
    //
    ASTProgram * Parser::parse()
    {
        ASTProgram *program = MallocMemory<ASTProgram>();
        advance();
        while (token_.kind_ == TK_Function)
        {
            program->push_back(parseFunctionDecl());
        }
        return program;
    }

    void Parser::advance()
    {
        token_ = lexer_.getToken();
    }

    void Parser::match(unsigned tok)
    {
        if (token_.kind_ != tok)
        {
            std::cout << tok << " need in file but find " << token_.kind_ << " "<< token_.coord_.lineNum_ << " " << token_.coord_.linePos_ << std::endl;
            throw std::runtime_error("match false");
        }
        advance();
    }

    std::string & Parser::exceptIdentifier()
    {
        if (token_.kind_ != TK_Identifier)
        {
            std::cout << " Identifier need in file but find " << token_.kind_ << " " << token_.coord_.lineNum_ << " " << token_.coord_.linePos_ << std::endl;
            throw std::runtime_error("match false");
        }
        advance();
        return token_.value_;
    }
}
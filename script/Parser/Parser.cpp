#include <map>
#include <string>
#include <iostream>

#include "Parser.h"

using std::map;
using std::string;
using std::vector;
using std::unique_ptr;
using std::make_unique;

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

    unique_ptr<ASTree> Parser::parseKeywordConstant()
    {
        unique_ptr<ASTree> expr = nullptr;
        if (token_.kind_ == TK_True)
            expr = make_unique<ASTConstant>(1);
        else if (token_.kind_ == TK_False)
            expr = make_unique<ASTConstant>(0);
        else if (token_.kind_ == TK_Null)
            expr = make_unique<ASTNull>();
        else
        {
            std::cout << "find " << token_.value_ << "(" << token_.coord_.lineNum_ << "," << token_.coord_.linePos_ << std::endl;
            throw std::runtime_error(" i dont konw");
        }
        advance();
        return std::move(expr);
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
    unique_ptr<ASTree> Parser::parseFactor()
    {
        unique_ptr<ASTree> expr = nullptr;
        switch (token_.kind_)
        {
        case TK_LitFloat: 
        {
            expr = make_unique<ASTConstant>(token_.fnum_);
            advance();
            break;
        }
        case TK_LitString:
        {
            expr = make_unique<ASTConstant>(token_.value_);
            advance();
            break;
        }
        case TK_LitInteger:
        {
            expr = make_unique<ASTConstant>(token_.num_);
            advance();
            break;
        }
        case TK_LitCharacter:
        {
            expr = make_unique<ASTConstant>(token_.value_[0]);
            advance();
            break;
        }
        case TK_Identifier:
        {
            expr = make_unique<ASTIdentifier>(token_.value_);
            advance();
            break;
        }
        case TK_LParen:
        {
            advance();
            expr = std::move(parseExpr());
            match(TK_RParen);
            break;
        }
        case TK_LSquareBrace:
        {
            advance();
            expr = make_unique<ASTArray>(parseExprList());
            match(TK_RSquareBrace);
            break;
        }
        default:
            expr = std::move(parseKeywordConstant());
            break;
        }
        return std::move(expr);
    }

    //
    // positive_factor:
    //  factor{ (["[" expression "]"] | ["(" expression_list ")"]) }
    // 
    unique_ptr<ASTree> Parser::parsePositveFactor()
    {
        unique_ptr<ASTree> expr = std::move(parseFactor());
        while (token_.kind_ == TK_LSquareBrace ||
            token_.kind_ == TK_LParen)
        {
            if (token_.kind_ == TK_LSquareBrace)
            {
                advance();
                expr = make_unique<ASTArrayIndex>(
                    std::move(expr), std::move(parseExpr()));
                match(TK_RSquareBrace);
            }
            else
            {
                advance();
                expr = make_unique<ASTCall>(
                    std::move(expr), std::move(parseExprList()));
                match(TK_RParen);
            }
        }
        return std::move(expr);
    }

    //
    // not_factor:
    //  ["!"] positive_factor
    //
    unique_ptr<ASTree> Parser::parseNotFactor()
    {
        if (token_.kind_ == TK_Not)
        {
            advance();
            return make_unique<ASTSingleExpression>(
                ASTSingleExpression::OP_Not,
                std::move(parsePositveFactor())
                );
        }
        return std::move(parsePositveFactor());
    }

    //
    // term:
    //  ["-"] not_factor
    //
    unique_ptr<ASTree> Parser::parseTerm()
    {
        if (token_.kind_ == TK_Sub)
        {
            advance();
            return make_unique<ASTSingleExpression>(
                ASTSingleExpression::OP_Sub,
                std::move(parseNotFactor())
                );
        }
        return std::move(parseNotFactor());
    }

    //
    // additive_expression:
    //  term{ ("*" | "/") term }
    //
    unique_ptr<ASTree> Parser::parseAdditiveExpr()
    {
        unique_ptr<ASTree> expr = std::move(parseTerm());
        while (token_.kind_ == TK_Mul || token_.kind_ == TK_Div)
        {
            advance();
            expr = make_unique<ASTBinaryExpression>(
                token_.kind_ == TK_Plus
                ? ASTBinaryExpression::OP_Mul
                : ASTBinaryExpression::OP_Div,
                std::move(expr),
                std::move(parseTerm())
                );
        }
        return std::move(expr);
    }

    //
    // relational_expression:
    //  additive_expression{ ("+" | "-") additive_expression }
    // 
    unique_ptr<ASTree> Parser::parseRelationalExpr()
    {
        unique_ptr<ASTree> expr = std::move(parseAdditiveExpr());
        while (token_.kind_ == TK_Plus || token_.kind_ == TK_Sub)
        {
            advance();
            expr = make_unique<ASTBinaryExpression>(
                token_.kind_ == TK_Plus 
                ? ASTBinaryExpression::OP_Add
                : ASTBinaryExpression::OP_Sub,
                std::move(expr),
                std::move(parseAdditiveExpr())
                );
        }
        return std::move(expr);
    }

    //
    // and_expression:
    //  relational_expression[("<" | ">" | ">=" | "<=" | "==" | "!=") relational_expression]
    // 
    unique_ptr<ASTree> Parser::parseAndExpr()
    {
        unique_ptr<ASTree> expr = std::move(parseRelationalExpr());
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
            expr = make_unique<ASTRelationalExpression>(
                op, std::move(expr), std::move(parseRelationalExpr())
                );
        }
        return std::move(expr);
    }

    //
    // or_expression:
    //  and_expression{ "&&" and_expression }
    //
    unique_ptr<ASTree> Parser::parseOrExpr()
    {
        unique_ptr<ASTree> expr = parseAndExpr();
        if (token_.kind_ != TK_And)
            return std::move(expr);
        unique_ptr<ASTAndExpression> andExpr = make_unique<ASTAndExpression>();
        andExpr->push_back(std::move(expr));
        while (token_.kind_ == TK_And)
        {
            advance();
            andExpr->push_back(std::move(parseAndExpr()));
        }
        return std::move(andExpr);
    }

    //
    // expression:
    //  or_expression{ "||" or_expression }
    //
    unique_ptr<ASTree> Parser::parseExpr()
    {
        unique_ptr<ASTree> expr = std::move(parseOrExpr());
        if (token_.kind_ != TK_Or)
            return std::move(expr);
        unique_ptr<ASTOrExpression> orExpr = make_unique<ASTOrExpression>();
        orExpr->push_back(std::move(expr));
        while (token_.kind_ == TK_Or)
        {
            advance();
            orExpr->push_back(std::move(parseOrExpr()));
        }
        return std::move(orExpr);
    }

    //
    // assign_expression:
    //  expression { "=" expression }
    //
    unique_ptr<ASTree> Parser::parseAssignExpr()
    {
        unique_ptr<ASTree> expr = std::move(parseExpr());
        while (token_.kind_ == TK_Assign)
        {
            advance();
            expr = make_unique<ASTAssignExpression>(
                std::move(expr), std::move(parseExpr()));
        }
        return expr;
    }

    //
    // expression_list:
    //  expression { "," expression }
    //
    unique_ptr<ASTree> Parser::parseExprList()
    {
        unique_ptr<ASTExpressionList> list = make_unique<ASTExpressionList>();
        list->push_back(std::move(parseExpr()));
        while (token_.kind_ == TK_Comma)
        {
            advance();
            list->push_back(std::move(parseExpr()));
        }
        return std::move(list);
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
    unique_ptr<ASTree> Parser::parseStatement()
    {
        switch (token_.kind_)
        {
        case TK_LCurlyBrace:
            return std::move(parseBlock());
            break;
        case TK_If:
        {
            advance();
            match(TK_LParen);
            unique_ptr<ASTree> condition = std::move(parseAssignExpr());
            match(TK_RParen);
            unique_ptr<ASTree> ifState = std::move(parseStatement());
            if (token_.kind_ == TK_Else)
            {
                advance();
                unique_ptr<ASTree> elseState = std::move(parseStatement());
                return make_unique<ASTIfStatement>(
                    std::move(condition), 
                    std::move(ifState), 
                    std::move(elseState));
            }
            return make_unique<ASTIfStatement>(
                std::move(condition), std::move(ifState));
        }
        case TK_While:
        {
            advance();
            match(TK_LParen);
            unique_ptr<ASTree> condition = std::move(parseAssignExpr());
            match(TK_RParen);
            unique_ptr<ASTree> state = std::move(parseStatement());
            return make_unique<ASTWhileStatement>(
                std::move(condition), std::move(state));
        }
        case TK_Return:
        {    
            advance();
            unique_ptr<ASTree> expr = nullptr;
            if (token_.kind_ != TK_Semicolon)
                expr = std::move(parseAssignExpr());
            
            match(TK_Semicolon);
            return make_unique<ASTReturnStatement>(std::move(expr));
        }
        case TK_Break:
        {
            advance();
            match(TK_Semicolon);
            return make_unique<ASTBreakStatement>();
        }
        case TK_Continue:
        {
            advance();
            match(TK_Semicolon);
            return make_unique<ASTContinueStatement>();
        }
        case TK_Let:
        {
            advance();
            string &name = exceptIdentifier();
            match(TK_Assign);
            unique_ptr<ASTree> expr = std::move(parseAssignExpr());
            match(TK_Semicolon);
            return make_unique<ASTVarDeclStatement>(name, std::move(expr));
        }
        default:
            unique_ptr<ASTree> expr = std::move(parseAssignExpr());
            match(TK_Semicolon);
            return std::move(expr);
        }
    }

    // 
    // block:
    //  "{" { statement } "}"
    //
    unique_ptr<ASTBlock> Parser::parseBlock()
    {
        unique_ptr<ASTBlock> block = make_unique<ASTBlock>(table_);
        table_ = block->table_;
        match(TK_LCurlyBrace);
        while (token_.kind_ != TK_RCurlyBrace)
        {
            block->push_back(std::move(parseStatement()));
        }
        advance();
        table_ = block->table_->parent_;
        return std::move(block);
    }

    //
    // param_list:
    //  ID{ ","  ID }
    //
    // function_decl:
    //  "function" ID "("[param_list] ")" block
    //
    unique_ptr<ASTFunction> Parser::parseFunctionDecl()
    {
        advance();
        string &name = exceptIdentifier();
        
        match(TK_LParen);

        vector<string> params;
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
        unique_ptr<ASTPrototype> proto = 
            std::make_unique<ASTPrototype>(name, std::move(params));

        match(TK_RParen);
        unique_ptr<ASTBlock> block = std::move(parseBlock());

        return make_unique<ASTFunction>(std::move(proto), std::move(block));
    }

    //
    // program: 
    //  { function_decl }
    //
    unique_ptr<ASTProgram> Parser::parse()
    {
        unique_ptr<ASTProgram> program = make_unique<ASTProgram>();
        this->table_ = program->table_;
        advance();
        while (token_.kind_ == TK_Function)
        {
            program->push_back(std::move(parseFunctionDecl()));
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
            std::cout << tok << " need in file but find " 
                << token_.kind_ << " "<< token_.coord_.lineNum_ 
                << " " << token_.coord_.linePos_ << std::endl;
            throw std::runtime_error("match false");
        }
        advance();
    }

    std::string & Parser::exceptIdentifier()
    {
        if (token_.kind_ != TK_Identifier)
        {
            std::cout << " Identifier need in file but find " 
                << token_.kind_ << " " << token_.coord_.lineNum_ 
                << " " << token_.coord_.linePos_ << std::endl;
            throw std::runtime_error("match false");
        }
        advance();
        return token_.value_;
    }
}
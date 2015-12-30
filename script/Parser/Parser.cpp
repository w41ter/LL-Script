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

    void Parser::parseKeywordConstant()
    {
        if (token_.kind_ == TK_True)
            ;
        else if (token_.kind_ == TK_False)
            ;
        else if (token_.kind_ == TK_Null)
            ;
        else
        {
            std::cout << "find " << token_.value_ << "(" << token_.coord_.lineNum_ << "," << token_.coord_.linePos_ << std::endl;
            throw std::runtime_error(" i dont konw");
        }
        advance();
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
    void Parser::parseFactor()
    {
        switch (token_.kind_)
        {
        case TK_LitFloat: 
        {
            advance();
            break;
        }
        case TK_LitString:
        {
            advance();
            break;
        }
        case TK_LitInteger:
        {
            advance();
            break;
        }
        case TK_LitCharacter:
        {
            advance();
            break;
        }
        case TK_Identifier:
        {
            advance();
            break;
        }
        case TK_LParen:
        {
            advance();
            parseExpr();
            match(TK_RParen);
            break;
        }
        case TK_LSquareBrace:
        {
            advance();
            parseExprList();
            match(TK_RSquareBrace);
            break;
        }
        default:
            parseKeywordConstant();
            break;
        }
    }

    //
    // positive_factor:
    //  factor{ (["[" expression "]"] | ["(" expression_list ")"]) }
    // 
    void Parser::parsePositveFactor()
    {
        parseFactor();
        while (token_.kind_ == TK_LSquareBrace ||
            token_.kind_ == TK_LParen)
        {
            if (token_.kind_ == TK_LSquareBrace)
            {
                advance();
                parseExpr();
                match(TK_RSquareBrace);
            }
            else
            {
                advance();
                parseExprList();
                match(TK_RParen);
            }
        }
    }

    //
    // not_factor:
    //  ["!"] positive_factor
    //
    void Parser::parseNotFactor()
    {
        if (token_.kind_ == TK_Not)
        {
            advance();
        }
        parsePositveFactor();
    }

    //
    // term:
    //  ["-"] not_factor
    //
    void Parser::parseTerm()
    {
        if (token_.kind_ == TK_Sub)
        {
            advance();
        }
        parseNotFactor();
    }

    //
    // additive_expression:
    //  term{ ("*" | "/") term }
    //
    void Parser::parseAdditiveExpr()
    {
        parseTerm();
        while (token_.kind_ == TK_Mul || token_.kind_ == TK_Div)
        {
            advance();
            parseTerm();
        }
    }

    //
    // relational_expression:
    //  additive_expression{ ("+" | "-") additive_expression }
    // 
    void Parser::parseRelationalExpr()
    {
        parseAdditiveExpr();
        while (token_.kind_ == TK_Plus || token_.kind_ == TK_Sub)
        {
            advance();
            parseAdditiveExpr();
        }
    }

    //
    // and_expression:
    //  relational_expression[("<" | ">" | ">=" | "<=" | "==" | "!=") relational_expression]
    // 
    void Parser::parseAndExpr()
    {
        parseRelationalExpr();
        if (isRelational(token_.kind_))
        {
            advance();
            parseRelationalExpr();
        }
    }

    //
    // or_expression:
    //  and_expression{ "&&" and_expression }
    //
    void Parser::parseOrExpr()
    {
        parseAndExpr();
        while (token_.kind_ == TK_And)
        {
            advance();
            parseAndExpr();
        }
    }

    //
    // expression:
    //  or_expression{ "||" or_expression }
    //
    void Parser::parseExpr()
    {
        parseOrExpr();
        while (token_.kind_ == TK_Or)
        {
            advance();
            parseOrExpr();
        }
    }

    //
    // assign_expression:
    //  expression { "=" expression }
    //
    void Parser::parseAssignExpr()
    {
        parseExpr();
        while (token_.kind_ == TK_Assign)
        {
            advance();
            parseExpr();
        }
    }

    //
    // expression_list:
    //  expression { "," expression }
    //
    void Parser::parseExprList()
    {
        parseExpr();
        while (token_.kind_ == TK_Comma)
        {
            advance();
            parseExpr();
        }
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
    void Parser::parseStatement()
    {
        switch (token_.kind_)
        {
        case TK_LCurlyBrace:
            parseBlock();
            break;
        case TK_If:
        {
            advance();
            match(TK_LParen);
            parseAssignExpr();
            match(TK_RParen);
            parseStatement();
            if (token_.kind_ == TK_Else)
            {
                advance();
                parseStatement();
            }
            break;
        }
        case TK_While:
        {
            advance();
            match(TK_LParen);
            parseAssignExpr();
            match(TK_RParen);
            parseStatement();
            break;
        }
        case TK_Return:
        {    
            advance();
            if (token_.kind_ != TK_Semicolon)
                parseAssignExpr();
            
            match(TK_Semicolon);
            break;
        }
        case TK_Break:
        {
            advance();
            match(TK_Semicolon);
            break;
        }
        case TK_Continue:
        {
            advance();
            match(TK_Semicolon);
            break;
        }
        case TK_Let:
        {
            advance();
            advance();
            match(TK_Assign);
            parseAssignExpr();
            match(TK_Semicolon);
            break;
        }
        default:
            parseAssignExpr();
            match(TK_Semicolon);
            break;
        }
    }

    // 
    // block:
    //  "{" { statement } "}"
    //
    void Parser::parseBlock()
    {
        match(TK_LCurlyBrace);
        while (token_.kind_ != TK_RCurlyBrace)
        {
            parseStatement();
        }
        advance();
    }

    //
    // param_list:
    //  ID{ ","  ID }
    //
    // function_decl:
    //  "function" ID "("[param_list] ")" block
    //
    void Parser::parseFunctionDecl()
    {
        advance();
        advance();
        match(TK_LParen);
        if (token_.kind_ == TK_Identifier)
        {
            advance();
            while (token_.kind_ == TK_Comma)
            {
                advance();
                advance();
            }
        }
        match(TK_RParen);
        parseBlock();
    }

    //
    // program: 
    //  { function_decl }
    //
    void Parser::parse()
    {
        advance();
        while (token_.kind_ == TK_Function)
        {
            parseFunctionDecl();
        }
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
}
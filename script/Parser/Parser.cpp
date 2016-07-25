#include "Parser.h"

#include <iostream>
#include <sstream>

#include "../Semantic/AST.h"
#include "../Semantic/ASTContext.h"
#include "../BuildIn/BuildIn.h"

using std::map;
using std::set;
using std::string;
using std::vector;

namespace script
{
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

    void Parser::factor()
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
            expression();
            match(TK_RParen);
            break;
        }
        case TK_LSquareBrace:
        {
            tableDecl();
            break;
        }
        case TK_Lambda:
        {
            lambdaDecl();
            break;
        }
        default:
            keywordConstant();
            break;
        }
    }

    void Parser::indexExpr()
    {
        factor();
        while (token_.kind_ == TK_Period)
        {
            advance();
            factor();
        }
    }

    void Parser::factorSuffix()
    {
        indexExpr();
        do {
            if (token_.kind_ == TK_LSquareBrace)
            {
                advance();
                expression();
                match(TK_RSquareBrace);
            }
            else if (token_.kind_ == TK_LParen)
            {
                advance();
                if (token_.kind_ != TK_RParen)
                {
                    expression();
                    while (token_.kind_ == TK_Comma)
                    {
                        advance();
                        expression();
                    }
                }
                
                match(TK_RParen);
            }
            else
            {
                break;
            }
        } while (true);
    }

    void Parser::notExpr()
    {
        if (token_.kind_ == TK_Not)
        {
            advance();
        }
        factorSuffix();
    }

    void Parser::negativeExpr()
    {
        if (token_.kind_ == TK_Sub)
        {
            advance();
        }
        notExpr();
    }

    void Parser::mulAndDivExpr()
    {
        negativeExpr();
        while (token_.kind_ == TK_Mul || token_.kind_ == TK_Div)
        {
            unsigned op = token_.kind_;
            advance();
            negativeExpr();
        }
    }

    void Parser::addAndSubExpr()
    {
        mulAndDivExpr();
        while (token_.kind_ == TK_Plus || token_.kind_ == TK_Sub)
        {
            unsigned op = token_.kind_;
            advance();
            mulAndDivExpr();
        }
    }

    void Parser::relationalExpr()
    {
        addAndSubExpr();
        while (isRelational(token_.kind_))
        {
            unsigned op = token_.kind_;
            advance();
            addAndSubExpr();
        }
    }

    void Parser::andExpr()
    {
        relationalExpr();
        if (token_.kind_ == TK_And)
        {
            advance();
            relationalExpr();
        }
    }

    void Parser::orExpr()
    {
        andExpr();
        while (token_.kind_ == TK_Or)
        {
            advance();
            andExpr();
        }
    }

    void Parser::expression()
    {
        orExpr();
        while (token_.kind_ == TK_Assign)
        {
            advance();
            orExpr();
        }
    }

    void Parser::breakStat()
    {
        match(TK_Break);
        match(TK_Semicolon);
    }

    void Parser::continueStat()
    {
        match(TK_Continue);
        match(TK_Semicolon);
    }

    void Parser::returnStat()
    {
        advance();
        if (token_.kind_ != TK_Semicolon)
            expression();

        match(TK_Semicolon);
    }

    void Parser::whileStat()
    {
        advance();
        match(TK_LParen);
        expression();
        match(TK_RParen);
        statement();
    }

    void Parser::ifStat()
    {
        advance();
        match(TK_LParen);
        expression();
        match(TK_RParen);
        statement();
        if (token_.kind_ == TK_Else)
        {
            advance();
            statement();
        }
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
        case TK_Function:
            return functionDecl();
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
    map<string, Token> Parser::readParams()
    {
        match(TK_LParen);

        map<string, Token> params;
        if (token_.kind_ == TK_Identifier)
        {
            params.insert(std::pair<string, Token>(token_.value_, token_));
            advance();
            while (token_.kind_ == TK_Comma)
            {
                advance();
                Token token = token_;
                params.insert(std::pair<string, Token>(exceptIdentifier(), token));
            }
        }

        match(TK_RParen);

        return std::move(params);
    }

    void Parser::keywordConstant()
    {
        if (token_.kind_ == TK_True)
            ;
        else if (token_.kind_ == TK_False)
            ;
        else if (token_.kind_ == TK_Null)
            ;
        else
            errorUnrecordToken();
        advance();
    }

    void Parser::lambdaDecl()
    {
        match(TK_Lambda);
        map<string, Token> params = std::move(readParams());

        block();
    }

    void Parser::tableDecl()
    {
        //match(TK_LSquareBrace);
        do {
            advance();
            if (token_.kind_ == TK_Identifier)
            {
                string name = exceptIdentifier();
                if (token_.kind_ == TK_Assign)
                {
                    advance();
                    expression();
                }
            }
            else if (token_.kind_ == TK_LSquareBrace)
            {
                advance();
                if (token_.kind_ == TK_LitCharacter)
                {
                    advance();
                }
                else if (token_.kind_ == TK_LitFloat)
                {
                    advance();
                }
                else if (token_.kind_ == TK_LitInteger)
                {
                    advance();
                }
                else if (token_.kind_ == TK_LitString)
                {
                    advance();
                }
                else
                {
                    //error
                }
                match(TK_Assign);
                expression();
                match(TK_RSquareBrace);
            }
            else if (token_.kind_ == TK_LitCharacter)
            {
                advance();
            }
            else if (token_.kind_ == TK_LitFloat)
            {
                advance();
            }
            else if (token_.kind_ == TK_LitInteger)
            {
                advance();
            }
            else if (token_.kind_ == TK_LitString)
            {
                advance();
            }
            else
            {
                break;
            }
        } while (token_.kind_ == TK_Comma);
        match(TK_RSquareBrace);
    }

    void Parser::functionDecl()
    {
        match(TK_Function);
        string name = exceptIdentifier();

        map<string, Token> params = std::move(readParams());

        block();
    }

    void Parser::letDecl()
    {
        match(TK_Let);
        string name = exceptIdentifier();
        match(TK_Assign);
        expression();
        match(TK_Semicolon);
    }

    void Parser::defineDecl()
    {
        match(TK_Define);
        string name = exceptIdentifier();
        match(TK_Assign);
        expression();
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
        advance();
        while (token_.kind_ != TK_EOF)
        {
            if (!topLevelDecl())
            {
                expression();
                match(TK_Semicolon);
            }
        }
    }

    Parser::Parser(Lexer & lexer, IRModule &module, DiagnosisConsumer &diag) 
        : lexer_(lexer), module_(module), diag_(diag)
    {
        initialize();
    }

    // 
    // 输出公共错误信息
    // 
    void Parser::commonError()
    {
        std::cout << token_.coord_.fileName_ << "(" <<
            token_.coord_.lineNum_ << "," << token_.coord_.linePos_ << "): ";
    }

    void Parser::errorUnrecordToken()
    {
        commonError();
        std::cout << "find unexcepted character " << token_.value_ << std::endl;
        throw std::runtime_error(" i dont konw");
    }

    void Parser::errorUndefined(const std::string & name)
    {
        commonError();
        std::cout << "identifier \"" << name << "\" are undefined!" << std::endl;
        throw std::runtime_error("identifier undefined!");
    }

    void Parser::errorRedefined(const std::string & name)
    {
        commonError();
        std::cout << "identifier \"" << name << "\" are redefined!" << std::endl;
        throw std::runtime_error("identifier redefined!");
    }

    void Parser::advance()
    {
        token_ = lexer_.getToken();
    }

    void Parser::match(unsigned tok)
    {
        if (token_.kind_ != tok)
        {
            commonError();
            std::cout << tok << " except in file but find " 
                << token_.kind_ << std::endl;
            throw std::runtime_error("match false");
        }
        advance();
    }

    std::string Parser::exceptIdentifier()
    {
        if (token_.kind_ != TK_Identifier)
        {
            commonError();
            std::cout << "identifier need but find " << token_.kind_ << std::endl;
            throw std::runtime_error("match false");
        }
        string value = std::move(token_.value_);
        advance();
        return std::move(value);
    }
}
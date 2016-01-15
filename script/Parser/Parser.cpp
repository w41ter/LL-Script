#include <map>
#include <string>
#include <iostream>
#include <sstream>

#include "Parser.h"

using std::map;
using std::set;
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
            { "define", TK_Define }
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
            std::cout << "find " << token_.value_ << "(" 
                << token_.coord_.lineNum_ << "," 
                << token_.coord_.linePos_ << ")" << std::endl;
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
    //    | function_decl
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
        case TK_Function:
        {
            return std::move(parseFunctionDecl());
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
                unique_ptr<ASTree> exprList = 
                    make_unique<ASTExpressionList>();
                if (token_.kind_ != TK_RParen)
                    exprList = std::move(parseExprList());
                expr = make_unique<ASTCall>(
                    std::move(expr), std::move(exprList));
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
                TK_Not,
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
                TK_Sub, std::move(parseNotFactor()));
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
                token_.kind_,
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
                token_.kind_,
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
            advance();
            expr = make_unique<ASTRelationalExpression>(
                token_.kind_, std::move(expr),
                std::move(parseRelationalExpr())
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
            string name = exceptIdentifier();
            match(TK_Assign);
            unique_ptr<ASTree> expr = std::move(parseAssignExpr());
            match(TK_Semicolon);
            return make_unique<ASTVarDeclStatement>(name, std::move(expr));
        }
        default:
            unique_ptr<ASTree> expr = 
                make_unique<ASTStatement>(std::move(parseAssignExpr()));
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
        unique_ptr<ASTBlock> block = make_unique<ASTBlock>();
        match(TK_LCurlyBrace);
        while (token_.kind_ != TK_RCurlyBrace)
        {
            block->push_back(std::move(parseStatement()));
        }
        advance();
        return std::move(block);
    }

    //
    // param_list:
    //  ID{ ","  ID }
    //
    // function_decl:
    //  "function" "("[param_list] ")" block
    //
    unique_ptr<ASTClosure> Parser::parseFunctionDecl()
    {
        string name = getTempIDName("lambda");

        advance();
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
        
        match(TK_RParen);

        // init symbol table
        Symbols table(symbolTable_.back());
        for (auto &i : params)
        {
            if (!table.Insert(i, 0))
            {
                // error redefined
            }
        }
        symbolTable_.push_back(&table);

        // parse block
        set<string> catchTable;
        catch_.push_back(&catchTable);
        unique_ptr<ASTBlock> block = std::move(parseBlock());
        catch_.pop_back();
        symbolTable_.pop_back();

        // insert catch table
        Symbols *parent = symbolTable_.back();

        vector<string> argument;
        for (auto &i : catchTable)
        {
            argument.push_back(i);
            // insert catch table into parent
            if (parent->find(i) == parent->end())
                catch_.back()->insert(i);
        }

        for (auto &i : params)
        {
            argument.push_back(i);
        }
        
        unique_ptr<ASTPrototype> proto =
            make_unique<ASTPrototype>(name, std::move(argument));
        functions_->push_back(
            make_unique<ASTFunction>(std::move(proto), std::move(block)));

        // Create closure
        argument.clear();
        for (auto &i : catchTable)
        {
            argument.push_back(i);
        }
        return make_unique<ASTClosure>(name, argument);
    }

    //
    // define_decl:
    //  "define" ID "=" expression ";"
    //
    unique_ptr<ASTDefine> Parser::parseDefine()
    {
        match(TK_Define);
        string name = exceptIdentifier();
        match(TK_Assign);
        unique_ptr<ASTree> tree = std::move(parseExpr());
        match(TK_Semicolon);
        return make_unique<ASTDefine>(name, std::move(tree));
    }

    //
    // program: 
    //  { define_decl }
    //
    unique_ptr<ASTProgram> Parser::parse()
    {
        vector<unique_ptr<ASTDefine>> defines;
        vector<unique_ptr<ASTFunction>> functions;
        this->functions_ = &functions;

        Symbols symbol(nullptr);
        set<string> catchTable;
        symbolTable_.push_back(&symbol);
        catch_.push_back(&catchTable);

        advance();
        while (token_.kind_ == TK_Define)
        {
            defines.push_back(std::move(parseDefine()));
        }

        this->functions_ = nullptr;
        return make_unique<ASTProgram>(std::move(defines), std::move(functions));
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

    std::string Parser::exceptIdentifier()
    {
        if (token_.kind_ != TK_Identifier)
        {
            std::cout << " Identifier need in file but find " 
                << token_.kind_ << " " << token_.coord_.lineNum_ 
                << " " << token_.coord_.linePos_ << std::endl;
            throw std::runtime_error("match false");
        }
        string value = std::move(token_.value_);
        advance();
        return std::move(value);
    }
}
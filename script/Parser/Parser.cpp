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
            { "define", TK_Define }
        };
        for (auto i : keywords)
        {
            lexer_.registerKeyword(i.first, i.second);
        }
    }

    ASTree *Parser::parseKeywordConstant()
    {
        ASTree *expr = nullptr;
        if (token_.kind_ == TK_True)
            expr = context_.allocate<ASTConstant>(1, token_);
        else if (token_.kind_ == TK_False)
            expr = context_.allocate<ASTConstant>(0, token_);
        else if (token_.kind_ == TK_Null)
            expr = context_.allocate<ASTNull>();
        else
            errorUnrecordToken();
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
    //    | function_decl
    //
    ASTree *Parser::parseFactor()
    {
        ASTree *expr = nullptr;
        switch (token_.kind_)
        {
        case TK_LitFloat: 
        {
            expr = context_.allocate<ASTConstant>(token_.fnum_, token_);
            advance();
            break;
        }
        case TK_LitString:
        {
            expr = context_.allocate<ASTConstant>(token_.value_, token_);
            advance();
            break;
        }
        case TK_LitInteger:
        {
            expr = context_.allocate<ASTConstant>(token_.num_, token_);
            advance();
            break;
        }
        case TK_LitCharacter:
        {
            expr = context_.allocate<ASTConstant>(token_.value_[0], token_);
            advance();
            break;
        }
        case TK_Identifier:
        {
            SymbolTable *symbol = symbolTable_.back();
            if (symbol->find(token_.value_) == symbol->end())
            {
                if (symbol->findInTree(token_.value_) == symbol->end())
                    errorUndefined(token_.value_);
                catch_.back()->insert(token_.value_);
            }
            expr = context_.allocate<ASTIdentifier>(token_.value_);
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
            expr = context_.allocate<ASTArray>(parseExprList());
            match(TK_RSquareBrace);
            break;
        }
        case TK_Function:
        {
            return parseFunctionDecl();
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
    ASTree *Parser::parsePositveFactor()
    {
        ASTree *expr = parseFactor();
        while (token_.kind_ == TK_LSquareBrace || token_.kind_ == TK_LParen)
        {
            if (token_.kind_ == TK_LSquareBrace)
            {
                advance();
                expr = context_.allocate<ASTArrayIndex>(expr, parseExpr());
                match(TK_RSquareBrace);
            }
            else
            {
                advance();
                ASTree *exprList = context_.allocate<ASTExpressionList>();
                if (token_.kind_ != TK_RParen)
                    exprList = parseExprList();
                match(TK_RParen);
                expr = context_.allocate<ASTCall>(expr, exprList);
            }
        }
        return expr;
    }

    //
    // not_factor:
    //  ["!"] positive_factor
    //
    ASTree *Parser::parseNotFactor()
    {
        if (token_.kind_ == TK_Not)
        {
            advance();
            return context_.allocate<ASTSingleExpression>(
                TK_Not, parsePositveFactor());
        }
        return parsePositveFactor();
    }

    //
    // term:
    //  ["-"] not_factor
    //
    ASTree *Parser::parseTerm()
    {
        if (token_.kind_ == TK_Sub)
        {
            advance();
            return context_.allocate<ASTSingleExpression>(
                TK_Sub, parseNotFactor());
        }
        return parseNotFactor();
    }

    //
    // additive_expression:
    //  term{ ("*" | "/") term }
    //
    ASTree *Parser::parseAdditiveExpr()
    {
        ASTree *expr = parseTerm();
        while (token_.kind_ == TK_Mul || token_.kind_ == TK_Div)
        {
            unsigned op = token_.kind_;
            advance();
            expr = context_.allocate<ASTBinaryExpression>(
                op, expr, parseTerm());
        }
        return expr;
    }

    //
    // relational_expression:
    //  additive_expression{ ("+" | "-") additive_expression }
    // 
    ASTree *Parser::parseRelationalExpr()
    {
        ASTree *expr = parseAdditiveExpr();
        while (token_.kind_ == TK_Plus || token_.kind_ == TK_Sub)
        {
            unsigned op = token_.kind_;
            advance();
            expr = context_.allocate<ASTBinaryExpression>(
                op,expr, parseAdditiveExpr());
        }
        return expr;
    }

    //
    // and_expression:
    //  relational_expression[("<" | ">" | ">=" | "<=" | "==" | "!=") relational_expression]
    // 
    ASTree *Parser::parseAndExpr()
    {
        ASTree *expr = parseRelationalExpr();
        if (isRelational(token_.kind_))
        {
            unsigned op = token_.kind_;
            advance();
            expr = context_.allocate<ASTRelationalExpression>(
                op, expr, parseRelationalExpr());
        }
        return expr;
    }

    //
    // or_expression:
    //  and_expression{ "&&" and_expression }
    //
    ASTree *Parser::parseOrExpr()
    {
        ASTree *expr = parseAndExpr();
        if (token_.kind_ != TK_And)
            return expr;
        ASTAndExpression *andExpr = context_.allocate<ASTAndExpression>();
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
    ASTree *Parser::parseExpr()
    {
        ASTree *expr = parseOrExpr();
        if (token_.kind_ != TK_Or)
            return expr;
        ASTOrExpression *orExpr = context_.allocate<ASTOrExpression>();
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
    ASTree *Parser::parseAssignExpr()
    {
        ASTree *expr = parseExpr();
        while (token_.kind_ == TK_Assign)
        {
            advance();
            expr = context_.allocate<ASTAssignExpression>(expr, parseExpr());
        }
        return expr;
    }

    //
    // expression_list:
    //  expression { "," expression }
    //
    ASTree *Parser::parseExprList()
    {
        ASTExpressionList *list = context_.allocate<ASTExpressionList>();
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
    ASTree *Parser::parseStatement()
    {
        switch (token_.kind_)
        {
        case TK_LCurlyBrace:
            return parseBlock();
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
                return context_.allocate<ASTIfStatement>(
                    condition, ifState, elseState);
            }
            return context_.allocate<ASTIfStatement>(condition, ifState);
        }
        case TK_While:
        {
            advance();
            match(TK_LParen);
            ASTree *condition = parseAssignExpr();
            match(TK_RParen);
            ASTree *state = parseStatement();
            return context_.allocate<ASTWhileStatement>(condition, state);
        }
        case TK_Return:
        {    
            advance();
            ASTree *expr = nullptr;
            if (token_.kind_ != TK_Semicolon)
                expr = parseAssignExpr();
            
            match(TK_Semicolon);
            return context_.allocate<ASTReturnStatement>(expr);
        }
        case TK_Break:
        {
            advance();
            Token token = token_;
            match(TK_Semicolon);
            return context_.allocate<ASTBreakStatement>(token);
        }
        case TK_Continue:
        {
            advance();
            Token token = token_;
            match(TK_Semicolon);
            return context_.allocate<ASTContinueStatement>(token);
        }
        case TK_Let:
        {
            advance();
            Token token = token_;
            string name = exceptIdentifier();

            match(TK_Assign);
            ASTree *expr = parseAssignExpr();
            match(TK_Semicolon);

            // insert symbol to symbol table only after it done.
            SymbolTable *symbol = symbolTable_.back();
            if (!symbol->insert(name, SymbolType::ST_Variable, token))
                errorRedefined(name);

            return context_.allocate<ASTVarDeclStatement>(name, expr);
        }
        default:
            ASTree *expr = context_.allocate<ASTStatement>(parseAssignExpr());
            match(TK_Semicolon);
            return expr;
        }
    }

    // 
    // block:
    //  "{" { statement } "}"
    //
    ASTBlock *Parser::parseBlock()
    {
        ASTBlock *block = context_.allocate<ASTBlock>();
        match(TK_LCurlyBrace);
        while (token_.kind_ != TK_RCurlyBrace)
        {
            block->push_back(parseStatement());
        }
        advance();
        return block;
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

    //
    // param_list:
    //  ID { ","  ID }
    //
    // function_decl:
    //  "function" "("[param_list] ")" block
    //
    ASTClosure *Parser::parseFunctionDecl()
    {
        advance();

        string name = getTempIDName("lambda");
        map<string, Token> params = std::move(readParams());

        // init symbol table
        SymbolTable *table = new SymbolTable(symbolTable_.back());
        for (auto &i : params)
        {
            // all params is constant.
            if (!table->insert(i.first, SymbolType::ST_Constant, i.second))
                errorRedefined(i.first);
        }
        symbolTable_.push_back(table);

        set<string> catchTable;
        catch_.push_back(&catchTable);
        ASTBlock *block = parseBlock();
        catch_.pop_back();
        symbolTable_.pop_back();

        vector<string> argument;
        for (const auto &cat : catchTable)
            argument.push_back(cat);
        for (auto symbol : params)
            argument.push_back(symbol.first);

        int closureArgsSize = argument.size();

        ASTPrototype *proto = context_.allocate<ASTPrototype>(name, std::move(argument));
        functions_->push_back(context_.allocate<ASTFunction>(table, proto, block));

        // Create closure
        argument.clear();
        for (auto &i : catchTable)
            argument.push_back(i);
        return context_.allocate<ASTClosure>(name, closureArgsSize, std::move(argument));
    }

    //
    // define_decl:
    //  "define" ID "=" expression ";"
    //
    ASTDefine *Parser::parseDefine()
    {
        match(TK_Define);
        string name = exceptIdentifier();
        
        match(TK_Assign);
        ASTree *tree = parseExpr();
        match(TK_Semicolon);

        // like let, symbol be insert into after it done.
        SymbolTable *symbol = symbolTable_.back();
        if (!symbol->insert(name, SymbolType::ST_Constant, token_))
        {
            errorRedefined(name);
        }

        return context_.allocate<ASTDefine>(name, tree);
    }

    //
    // program: 
    //  { ( define_decl | statement ) }
    //
    void Parser::parse()
    {
        SymbolTable *symbol = new SymbolTable(nullptr);
        vector<ASTree*> statements;
        vector<ASTFunction*> functions;
        this->functions_ = &functions;

        set<string> catchTable;
        symbolTable_.push_back(symbol);
        catch_.push_back(&catchTable);

        buildin::BuildIn::getInstance().map(
            [this, &statements] (const string &name, int size) -> string {
            symbolTable_.back()->insert(name, SymbolType::ST_Constant, Token());
            string tempName = getTempIDName(name.c_str());
            ASTree *tree = context_.allocate<ASTClosure>(tempName, size, vector<string>());
            statements.push_back(context_.allocate<ASTDefine>(name, tree));
            return std::move(tempName);
        });
        // TODO:

        advance();
        while (token_.kind_ != TK_EOF)
        {
            if (token_.kind_ == TK_Define)
                statements.push_back(parseDefine());
            else
                statements.push_back(parseStatement());
        }

        this->functions_ = nullptr;
        context_.setProgram(context_.allocate<ASTProgram>(
            symbol, std::move(statements), std::move(functions)));
    }

    Parser::Parser(Lexer & lexer, ASTContext &context) 
        : lexer_(lexer), context_(context)
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
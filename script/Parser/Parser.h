#ifndef __PARSER_H__
#define __PARSER_H__
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <memory>

#include "lexer.h"
#include "../Semantic/AST.h"
#include "../Semantic/ASTContext.h"

namespace script
{
    enum KeywordsIDs 
    {
        TK_If = TK_BeginKeywordIDs,
        TK_Let,
        TK_Else,
        TK_True,
        TK_Null,
        TK_False,
        TK_While,
        TK_Break,
        TK_Return,
        TK_Continue,
        TK_Function,
        TK_Define,

        TK_EndKeywordsIDs
    };

    class Parser
    {
    public:
        Parser(Lexer &lexer, ASTContext &context);

        void parse();

    private:
        void initialize();

        ASTree *parseKeywordConstant();
        ASTree *parseFactor();
        ASTree *parsePositveFactor();
        ASTree *parseNotFactor();
        ASTree *parseTerm();
        ASTree *parseAdditiveExpr();
        ASTree *parseRelationalExpr();
        ASTree *parseAndExpr();
        ASTree *parseOrExpr();
        ASTree *parseExpr();
        ASTree *parseExprList();
        ASTree *parseAssignExpr();
        ASTree *parseStatement();
        ASTBlock *parseBlock();
        ASTClosure *parseFunctionDecl();
        ASTDefine *parseDefine();

        SymbolTable *readParams();

        void commonError();
        void errorUnrecordToken();
        void errorUndefined(const std::string &name);
        void errorRedefined(const std::string &name);
        void advance();
        void match(unsigned tok);
        std::string exceptIdentifier();

        bool isRelational(unsigned tok);

        std::string getTempIDName(const char *name);

    private:
        Lexer &lexer_;
        Token token_;

        std::vector<SymbolTable*> symbolTable_;
        std::vector<std::set<std::string>*> catch_;
        std::vector<ASTFunction*> *functions_;

        // ASTContext 用于管理 AST 上下文
        ASTContext &context_;
    };
}

#endif // !__PARSER_H__

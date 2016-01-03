#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexer.h"
#include "../Semantic/AST.h"

namespace script
{
    enum KeywordsIDs {
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

        TK_EndKeywordsIDs
    };

    class Parser
    {
    public:
        Parser(Lexer &lexer) : lexer_(lexer), table_(nullptr) { initialize(); }

        ASTProgram *parse();

    private:
        void initialize();

        ASTree * parseKeywordConstant();
        ASTree * parseFactor();
        ASTree * parsePositveFactor();
        ASTree * parseNotFactor();
        ASTree * parseTerm();
        ASTree * parseAdditiveExpr();
        ASTree * parseRelationalExpr();
        ASTree * parseAndExpr();
        ASTree * parseOrExpr();
        ASTree * parseExpr();
        ASTree * parseExprList();
        ASTree * parseAssignExpr();
        ASTree * parseStatement();
        ASTBlock * parseBlock();
        ASTFunction * parseFunctionDecl();

        void advance();
        void match(unsigned tok);
        std::string &exceptIdentifier();

        bool isRelational(unsigned tok);
    private:
        Lexer &lexer_;
        Token token_;
        SymbolTable *table_;
    };
}

#endif // !__PARSER_H__

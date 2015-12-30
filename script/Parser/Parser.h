#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexer.h"

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
        Parser(Lexer &lexer) : lexer_(lexer) { initialize(); }

        void parse();

    private:
        void initialize();

        void parseKeywordConstant();
        void parseFactor();
        void parsePositveFactor();
        void parseNotFactor();
        void parseTerm();
        void parseAdditiveExpr();
        void parseRelationalExpr();
        void parseAndExpr();
        void parseOrExpr();
        void parseExpr();
        void parseExprList();
        void parseAssignExpr();
        void parseStatement();
        void parseBlock();
        void parseFunctionDecl();

        void advance();
        void match(unsigned tok);

        bool isRelational(unsigned tok);
    private:
        Lexer &lexer_;
        Token token_;
    };
}

#endif // !__PARSER_H__

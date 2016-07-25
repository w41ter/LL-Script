#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <vector>
#include <set>
#include <map>

#include "lexer.h"

namespace script
{
    class IRModule;

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
        TK_Lambda,
        TK_Define,

        TK_EndKeywordsIDs
    };

    class Parser
    {
    public:
        Parser(Lexer &lexer, IRModule &context, DiagnosisConsumer &diag);

        void parse();

    private:
        void initialize();

        bool topLevelDecl();
        void defineDecl();
        void letDecl();
        void functionDecl();
        void tableDecl();
        void lambdaDecl();
        void keywordConstant();

        void expression();
        void orExpr();
        void andExpr();
        void relationalExpr();
        void addAndSubExpr();
        void mulAndDivExpr();
        void negativeExpr();
        void notExpr();
        void factorSuffix();
        void indexExpr();
        void factor();

        void block();
        void statement();
        void ifStat();
        void whileStat();
        void breakStat();
        void continueStat();
        void returnStat();

        std::map<std::string, Token> readParams();

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

        IRModule &module_;
        DiagnosisConsumer &diag_;
    };
}

#endif // !__PARSER_H__

#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexer.h"

namespace script
{
    enum KeywordsIDs {
        TK_If = TK_BeginKeywordIDs,
        TK_Let,
        TK_Else,
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
        Parser(Lexer &lexer) : lexer_(lexer) {}

        void parse();
    private:
        Lexer &lexer_;
    };
}

#endif // !__PARSER_H__

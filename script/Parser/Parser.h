#ifndef __PARSER_H__
#define __PARSER_H__

#include <memory>

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

        std::unique_ptr<ASTProgram> parse();

    private:
        void initialize();

        std::unique_ptr<ASTree> parseKeywordConstant();
        std::unique_ptr<ASTree> parseFactor();
        std::unique_ptr<ASTree> parsePositveFactor();
        std::unique_ptr<ASTree> parseNotFactor();
        std::unique_ptr<ASTree> parseTerm();
        std::unique_ptr<ASTree> parseAdditiveExpr();
        std::unique_ptr<ASTree> parseRelationalExpr();
        std::unique_ptr<ASTree> parseAndExpr();
        std::unique_ptr<ASTree> parseOrExpr();
        std::unique_ptr<ASTree> parseExpr();
        std::unique_ptr<ASTree> parseExprList();
        std::unique_ptr<ASTree> parseAssignExpr();
        std::unique_ptr<ASTree> parseStatement();
        std::unique_ptr<ASTBlock> parseBlock();
        std::unique_ptr<ASTFunction> parseFunctionDecl();

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

#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <vector>
#include <set>
#include <map>
#include <stack>

#include "lexer.h"
#include "../IR/Instruction.h"

namespace script
{
    class CFG;
    class IRContext;
    class SymbolTable;
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
        using Value = ir::Value;
    public:
        Parser(Lexer &lexer, IRModule &context, DiagnosisConsumer &diag);

        void parse();

    private:
        void initialize();

        bool topLevelDecl();
        void defineDecl();
        void letDecl();
        void functionDecl();

        void tableIdent(Value *table);
        void tableOthers(Value *table);
        Value *tableDecl();
        Value *lambdaDecl();
        Value *keywordConstant();

        Value *expression();
        Value *orExpr();
        Value *andExpr();
        Value *relationalExpr();
        Value *addAndSubExpr();
        Value *mulAndDivExpr();
        Value *negativeExpr();
        Value *notExpr();
        Value *factorSuffix();
        Value *indexExpr();
        Value *factor();

        void block();
        void statement();
        void ifStat();
        void whileStat();
        void breakStat();
        void continueStat();
        void returnStat();

        std::vector<std::pair<std::string, Token>> readParams();

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

        // Parse 时需要用的全局变量
        BasicBlock *allocaBlock_ = nullptr;
        BasicBlock *block_ = nullptr;
        SymbolTable *table_ = nullptr;
        IRContext *context_ = nullptr;
        CFG *cfg_ = nullptr;

        // 保存 Stack for break / continue.
        std::stack<BasicBlock*> breaks_;
        std::stack<BasicBlock*> continues_;
    };
}

#endif // !__PARSER_H__

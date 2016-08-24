#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include "lexer.h"
#include "Instruction.h"

namespace script
{
    class CFG;
    class IRContext;
    class SymbolTable;
    class IRFunction;
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

        void tableIdent(Value *table);
        void tableOthers(Value *table);
        Value *tableDecl();
        Value *lambdaDecl();

        Value *assignExpr();
        Value *rightHandExpr();
        Value *orExpr();
        Value *andExpr();
        Value *relationalExpr();
        Value *addAndSubExpr();
        Value *mulAndDivExpr();
        Value *negativeExpr();
        Value *notExpr();
        Value *value();
        Value *variableSuffix();
        Value *variable();

        std::string LHS(std::list<Value*> &lhs);

        void expression();
        void block();
        void statement();
        void ifStat();
        void whileStat();
        void breakStat();
        void continueStat();
        void returnStat();

        typedef std::vector<std::string> Strings;
        Value *createClosureForFunction(
			const std::string &name, 
			std::unordered_set<std::string> &captures);
        void readParams(Strings &params);
        void getFunctionPrototype(Strings &prototype, const Strings &params);
		void functionParamsAndBody(Strings &params, IRFunction *function);
		Value *functionCommon(const std::string &name);

        void advance();
        void match(unsigned tok);
        std::string exceptIdentifier();

        bool isRelational(unsigned tok);

        struct FunctionScope {
            enum { None, Define, Let };
			typedef std::unordered_map<std::string, unsigned> Symbols;
			BasicBlock *block_ = nullptr;
			IRContext *context_ = nullptr;
			CFG *cfg_ = nullptr;
            Symbols symbolTable;
            Symbols upperTable;
            std::unordered_set<std::string> captures;
        };

        typedef std::list<FunctionScope>::reverse_iterator scope_iterator;
        bool tryToCatchID(std::string &name);
        bool tryToCatchID(scope_iterator iter, std::string &name);

        void pushFunctionScope();
        void popFunctionScope();
		void pushFunctionScopeAndInit(IRFunction *func);
		void popFunctionScope(IRFunction *func);
        void defineIntoScope(const std::string &str, unsigned type);
        void insertIntoScope(const std::string &str, unsigned type);
        bool isDefineInScope(const std::string &str);
        bool isExistsInScope(const std::string &str);

        void clear();

    private:
        Lexer &lexer_;
        Token token_;

        IRModule &module_;
        DiagnosisConsumer &diag_;

        // Stack for break / continue.
        std::stack<BasicBlock*> breaks_;
        std::stack<BasicBlock*> continues_;

		FunctionScope *scope = nullptr;
        std::list<FunctionScope> functionStack;
    };
}

#endif // !__PARSER_H__

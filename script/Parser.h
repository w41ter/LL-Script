#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <set>
#include <list>
#include <vector>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <functional>

#include "lexer.h"

namespace script
{
    class CFG;
	class Value;
	class BasicBlock;
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
        Parser(Lexer &lexer, IRModule &context, 
			DiagnosisConsumer &diag);

        void parse();
		void registerUserClosure(const std::string &name);

    private:
        struct FunctionScope;
 
        typedef std::vector<std::string> Strings;
        typedef std::list<FunctionScope>::reverse_iterator scope_iterator;
        
        struct FunctionScope {
            typedef std::unordered_map<std::string, unsigned> Symbols;

            enum { None, Define, Let };

            FunctionScope()
                : cfg_(nullptr)
                , block_(nullptr)
            {}

            CFG *cfg_;
            Symbols symbolTable;
            Symbols upperTable; 
            BasicBlock *block_;
            std::unordered_set<std::string> captures;
        };

        void pushFunctionScope();
        void popFunctionScope();
        void pushFunctionScopeAndInit(IRFunction *func);
        void popFunctionScope(IRFunction *func);
        void defineIntoScope(const std::string &str, unsigned type);
        void insertIntoScope(const std::string &str, unsigned type);
        bool isDefineInScope(const std::string &str);
        bool isExistsInScope(const std::string &str);

        void advance();
        void match(unsigned tok);
        std::string exceptIdentifier();
        bool isRelational(unsigned tok);
        bool tryToCatchID(std::string &name);
        bool tryToCatchID(scope_iterator iter, std::string &name);

        void initialize();

        void parseDefineDecl();
        void parseLetDecl();
		void parseLetDefineCommon(const std::string &name);

		// createClosureForFunction - all callable is function.
		Value *createClosureForFunction(
			const std::string &name,
			std::unordered_set<std::string> &captures);
        Value *parseFunctionCommon(const std::string &name);
		void parseParams(Strings &params);
		void getFunctionPrototype(
			const std::string &name,
			Strings &prototype, 
			const Strings &params
        );
		void getFunctionParamsAndBody(
            Strings &params, 
            IRFunction *function
        );
        void parseFunctionDecl();
		void dealRecursiveDecl(const std::string &name);

        void parseTableIdent(Value *table);
        void parseTableOthers(Value *table);
        Value *parseTableDecl();
        Value *parseLambdaDecl();

        Value *parseRightHandExpr();
        Value *parseOrExpr();
        Value *parseAndExpr();
        Value *parseRelationalExpr();
        Value *parseAddAndSubExpr();
        Value *parseMulAndDivExpr();
        Value *parseNegativeExpr();
        Value *parseNotExpr();
        Value *parseValue();
        Value *parseVariableSuffix();
        Value *parseVariable();
        Value *parseSuffixCommon(Value *value);

		bool isSuffixCommonFisrtFollowSet();

        void parseAssignExpr();
        void parseExpression();
        void parseBlock();
        void parseStatement();
        void parseIfStat();
        void parseWhileStat();
        void parseBreakStat();
        void parseContinueStat();
        void parseReturnStat();

		void registerUserClosures();
    private:
        Lexer &lexer_;
        Token token_;

        IRModule &module_;
        DiagnosisConsumer &diag_;

        // Stack for break / continue.
        std::stack<BasicBlock*> breaks_;
        std::stack<BasicBlock*> continues_;

		FunctionScope *scope;
        std::list<FunctionScope> functionStack;
		std::unordered_set<std::string> userClosures;
    };
}

#endif // !__PARSER_H__

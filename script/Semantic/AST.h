#pragma once

#include <string>
#include <vector>
#include <map>

#include "../Parser/lexer.h"

namespace script
{
    enum class SymbolType
    {
        ST_NONE,
        ST_Constant,
        ST_Variable,
    };

    struct Symbol
    {
        Token token_;
        SymbolType type_;
    };

    class SymbolTable
    {
    public:
        SymbolTable(SymbolTable *parent);

        SymbolTable *getParent();
        bool insert(const std::string &name, SymbolType type, Token token);
        Symbol &find(const std::string &name);
        Symbol &findInTree(const std::string &name);
        Symbol &end() const;
        std::map<std::string, Symbol> &getTables();

    private:
        SymbolTable *parent_;
        std::map<std::string, Symbol> table_;
    };

    bool operator == (const Symbol &lhs, const Symbol &rhs);

    class ASTVisitor;

    class ASTree
    {
    public:
        virtual ~ASTree() = default;
        virtual bool accept(ASTVisitor *v) = 0;
    };

    class ASTClosure : public ASTree
    {
    public:
        ASTClosure(std::string name, int total,
            std::vector<std::string> params);

        virtual ~ASTClosure() = default;
        virtual bool accept(ASTVisitor *v) override;

        int total_;
        std::string name_;
        std::vector<std::string> params_;
    };

    class ASTIdentifier : public ASTree
    {
    public:
        virtual ~ASTIdentifier() = default;
        ASTIdentifier(std::string name);

        virtual bool accept(ASTVisitor *v) override;

        std::string name_;
    };

    class ASTNull : public ASTree
    {
    public:
        virtual ~ASTNull() = default;
        virtual bool accept(ASTVisitor *v) override;
    };

    class ASTConstant : public ASTree
    {
    public:
        enum {
            T_Character,
            T_Integer,
            T_Float,
            T_String,
        };
        virtual ~ASTConstant() = default;
        ASTConstant(char c, Token token);
        ASTConstant(int num, Token token);
        ASTConstant(float fnum, Token token);
        ASTConstant(std::string &str, Token token);

        virtual bool accept(ASTVisitor *v) override;

        unsigned type_;
        char c_;
        int num_;
        float fnum_;
        std::string str_;
        Token token_;
    };

    class ASTExpressionList : public ASTree
    {
    public:
        virtual ~ASTExpressionList() = default;
        void push_back(ASTree *tree);
        virtual bool accept(ASTVisitor *v) override;

        std::vector<ASTree*> exprs_;
    };

    class ASTArray : public ASTree
    {
    public:
        ASTArray(ASTree *tree);
        virtual ~ASTArray() = default;

        virtual bool accept(ASTVisitor *v) override;

        ASTree *array_;
    };

    class ASTCall : public ASTree
    {
    public:
        ASTCall(ASTree *function, ASTree *arguments);
        virtual ~ASTCall() = default;
        virtual bool accept(ASTVisitor *v) override;

        ASTree *function_;
        ASTree *arguments_;
    };

    class ASTArrayIndex : public ASTree
    {
    public:
        ASTArrayIndex(ASTree *array, ASTree *index);

        virtual ~ASTArrayIndex() = default;
        virtual bool accept(ASTVisitor *v) override;

        ASTree *array_;
        ASTree *index_;
    };

    class ASTSingleExpression : public ASTree
    {
    public:
        //enum {
        //    OP_Not,
        //    OP_Sub,
        //};

        ASTSingleExpression(unsigned op, ASTree *expr);
        virtual ~ASTSingleExpression() = default;
        virtual bool accept(ASTVisitor *v) override;

        unsigned op_;
        ASTree *expr_;
    };

    class ASTBinaryExpression : public ASTree
    {
    public:
        //enum {
        //    OP_Add,
        //    OP_Sub,
        //    OP_Mul,
        //    OP_Div,
        //};

        ASTBinaryExpression(unsigned op, ASTree *left, ASTree *right);
        virtual ~ASTBinaryExpression() = default;
        virtual bool accept(ASTVisitor *v) override;

        unsigned op_;
        ASTree *left_;
        ASTree *right_;
    };

    class ASTRelationalExpression : public ASTree
    {
    public:
        //enum {
        //    RL_GreatThan,
        //    RL_Great,
        //    RL_LessThan,
        //    RL_Less,
        //    RL_Equal,
        //    RL_NotEqual,
        //};

        ASTRelationalExpression(unsigned relation, ASTree *left, ASTree *right);
        virtual ~ASTRelationalExpression() = default;
        virtual bool accept(ASTVisitor *v) override;

        unsigned relation_;
        ASTree *left_;
        ASTree *right_;
    };

    class ASTAndExpression : public ASTree
    {
    public:
        virtual ~ASTAndExpression() = default;
        void push_back(ASTree *relation);

        virtual bool accept(ASTVisitor *v) override;

        std::vector<ASTree*> relations_;
    };

    class ASTOrExpression : public ASTree
    {
    public:
        virtual ~ASTOrExpression() = default;
        void push_back(ASTree *relation);

        virtual bool accept(ASTVisitor *v) override;

        std::vector<ASTree*> relations_;
    };

    class ASTAssignExpression : public ASTree
    {
    public:
        ASTAssignExpression(ASTree *left, ASTree *right);
        virtual ~ASTAssignExpression() = default;
        virtual bool accept(ASTVisitor *v) override;

        ASTree *left_;
        ASTree *right_;
    };

    class ASTVarDeclStatement : public ASTree
    {
    public:
        ASTVarDeclStatement(std::string &str, ASTree *expr);
        virtual ~ASTVarDeclStatement() = default;
        virtual bool accept(ASTVisitor *v) override;

        std::string name_;
        ASTree *expr_;
    };

    class ASTContinueStatement : public ASTree
    {
    public:
        ASTContinueStatement(Token token);
        virtual ~ASTContinueStatement() = default;
        virtual bool accept(ASTVisitor *v) override;

        Token token_;
    };

    class ASTBreakStatement : public ASTree
    {
    public:
        ASTBreakStatement(Token token);
        virtual ~ASTBreakStatement() = default;
        virtual bool accept(ASTVisitor *v) override;

        Token token_;
    };

    class ASTReturnStatement : public ASTree
    {
    public:
        ASTReturnStatement(ASTree *expr = nullptr);
        virtual ~ASTReturnStatement() = default;
        virtual bool accept(ASTVisitor *v) override;

        ASTree *expr_;
    };

    class ASTWhileStatement : public ASTree
    {
    public:
        ASTWhileStatement(ASTree *cond, ASTree *statement);
        virtual ~ASTWhileStatement() = default;
        virtual bool accept(ASTVisitor *v) override;

        ASTree *condition_;
        ASTree *statement_;
    };

    class ASTIfStatement : public ASTree
    {
    public:
        ASTIfStatement(ASTree *cond, ASTree *ifState);
        ASTIfStatement(ASTree *cond, ASTree *ifState, ASTree *elseState);

        virtual ~ASTIfStatement() = default;
        bool hasElse() const;

        virtual bool accept(ASTVisitor *v) override;

        ASTree *condition_;
        ASTree *ifStatement_;
        ASTree *elseStatement_;
        bool hasElse_;
    };

    class ASTStatement : public ASTree
    {
    public:
        ASTStatement(ASTree *tree);
        virtual ~ASTStatement() = default;
        virtual bool accept(ASTVisitor *v) override;

        ASTree *tree_;
    };

    class ASTBlock : public ASTree
    {
    public:
        ASTBlock() = default;
        virtual ~ASTBlock() = default;

        void push_back(ASTree *tree);

        virtual bool accept(ASTVisitor *v) override;

        std::vector<ASTree*> statements_;
    };

    class ASTPrototype : public ASTree
    {
    public:
        ASTPrototype(const std::string &name,
            std::vector<std::string> args);
        virtual ~ASTPrototype() = default;

        virtual bool accept(ASTVisitor *v) override;

        std::string name_;
        std::vector<std::string> args_;
    };

    class ASTFunction : public ASTree
    {
    public:
        ASTFunction(SymbolTable *table, ASTPrototype *proto, ASTBlock *block);
        virtual ~ASTFunction();

        virtual bool accept(ASTVisitor *v) override;

        SymbolTable *table_;
        ASTPrototype *prototype_;
        ASTBlock *block_;
    };

    class ASTDefine : public ASTree
    {
    public:
        ASTDefine(std::string &name, ASTree *expr);
        virtual ~ASTDefine() = default;
        virtual bool accept(ASTVisitor *v) override;

        std::string name_;
        ASTree *expr_;
    };

    class ASTProgram : public ASTree
    {
    public:
        ASTProgram(SymbolTable *table,
            std::vector<ASTree*> statements,
            std::vector<ASTFunction*> function);

        virtual ~ASTProgram();
        virtual bool accept(ASTVisitor *v) override;

        SymbolTable *table_;
        std::vector<ASTree*> statements_;
        std::vector<ASTFunction*> function_;
    };

    class ASTVisitor
    {
    public:
        virtual ~ASTVisitor() {};
        virtual bool visit(ASTExpressionList *v) = 0;
        virtual bool visit(ASTIdentifier *v) = 0;
        virtual bool visit(ASTNull *v) = 0;
        virtual bool visit(ASTConstant *v) = 0;
        virtual bool visit(ASTArray *v) = 0;
        virtual bool visit(ASTCall *v) = 0;
        virtual bool visit(ASTArrayIndex *v) = 0;
        virtual bool visit(ASTSingleExpression *v) = 0;
        virtual bool visit(ASTBinaryExpression *v) = 0;
        virtual bool visit(ASTRelationalExpression *v) = 0;
        virtual bool visit(ASTAndExpression *v) = 0;
        virtual bool visit(ASTOrExpression *v) = 0;
        virtual bool visit(ASTAssignExpression *v) = 0;
        virtual bool visit(ASTVarDeclStatement *v) = 0;
        virtual bool visit(ASTContinueStatement *v) = 0;
        virtual bool visit(ASTBreakStatement *v) = 0;
        virtual bool visit(ASTReturnStatement *v) = 0;
        virtual bool visit(ASTWhileStatement *v) = 0;
        virtual bool visit(ASTIfStatement *v) = 0;
        virtual bool visit(ASTStatement *v) = 0;
        virtual bool visit(ASTBlock *v) = 0;
        virtual bool visit(ASTFunction *v) = 0;
        virtual bool visit(ASTProgram *v) = 0;
        virtual bool visit(ASTPrototype *v) = 0;
        virtual bool visit(ASTDefine *v) = 0;
        virtual bool visit(ASTClosure *v) = 0;
    };
}
#pragma once

#include <string>
#include <vector>
#include <map>

namespace script
{
    class ASTree;
    class ASTIdentifier;
    class ASTNull;
    class ASTConstant;
    class ASTArray;
    class ASTCall;
    class ASTArrayIndex;
    class ASTExpressionList;
    class ASTSingleExpression;
    class ASTBinaryExpression;
    class ASTAndExpression;
    class ASTOrExpression;
    class ASTAssignExpression;
    class ASTVarDeclStatement;
    class ASTContinueStatement;
    class ASTBreakStatement;
    class ASTReturnStatement;
    class ASTWhileStatement;
    class ASTIfStatement;
    class ASTBlock;
    class ASTFunction;
    class ASTProgram;

    class Visitor
    {
    public:
        virtual bool visit(ASTExpressionList &v) = 0;
        virtual bool visit(ASTIdentifier &v) = 0;
        virtual bool visit(ASTNull &v) = 0;
        virtual bool visit(ASTConstant &v) = 0;
        virtual bool visit(ASTArray &v) = 0;
        virtual bool visit(ASTCall &v) = 0;
        virtual bool visit(ASTArrayIndex &v) = 0;
        virtual bool visit(ASTSingleExpression &v) = 0;
        virtual bool visit(ASTBinaryExpression &v) = 0;
        virtual bool visit(ASTRelationalExpression &v) = 0;
        virtual bool visit(ASTAndExpression &v) = 0;
        virtual bool visit(ASTOrExpression &v) = 0;
        virtual bool visit(ASTAssignExpression &v) = 0;
        virtual bool visit(ASTVarDeclStatement &v) = 0;
        virtual bool visit(ASTContinueStatement &v) = 0;
        virtual bool visit(ASTBreakStatement &v) = 0;
        virtual bool visit(ASTReturnStatement &v) = 0;
        virtual bool visit(ASTWhileStatement &v) = 0;
        virtual bool visit(ASTIfStatement &v) = 0;
        virtual bool visit(ASTBlock &v) = 0;
        virtual bool visit(ASTFunction &v) = 0;
        virtual bool visit(ASTProgram &v) = 0;
    };

    class ASTree
    {
    public:
        virtual ~ASTree() = default;
        virtual bool accept(Visitor &v) = 0;
    };

    class ASTIdentifier : public ASTree
    {
        friend class Visitor;
    public:
        virtual ~ASTIdentifier() = default;
        ASTIdentifier(std::string name) : name_(name) {}

        virtual bool accept(Visitor &v) override { return v.visit(*this); }

    private:
        std::string name_;
    };

    class ASTNull : public ASTree
    {
        friend class Visitor;
    public:
        virtual ~ASTNull() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    };

    class ASTConstant : public ASTree
    {
        friend class Visitor;
        enum {
            T_Charactor,
            T_Integer,
            T_Float,
            T_String,
        };
    public:
        virtual ~ASTConstant() = default;
        ASTConstant(char c) : type_(T_Charactor), c_(c) {}
        ASTConstant(int num) : type_(T_Integer), num_(num) {}
        ASTConstant(float fnum) : type_(T_Float), fnum_(fnum) {}
        ASTConstant(std::string &str) : type_(T_String), str_(str) {}

        virtual bool accept(Visitor &v) override { return v.visit(*this); }

    private:
        unsigned type_;
        char c_;
        int num_;
        float fnum_;
        std::string str_;
    };

    class ASTExpressionList : public ASTree
    {
    public:
        virtual ~ASTExpressionList() = default;
        void push_back(ASTree *tree) { exprs_.push_back(tree); }
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    private:
        std::vector<ASTree*> exprs_;
    };

    class ASTArray : public ASTree
    {
        friend class Visitor;
    public:
        virtual ~ASTArray() = default;
        void push_back(ASTree *tree) { array_.push_back(tree); }

        virtual bool accept(Visitor &v) override { v.visit(*this); }

    private:
        std::vector<ASTree *> array_;
    };

    class ASTCall : public ASTree
    {
        friend class Visitor;
    public:
        ASTCall(ASTree *function, ASTree *arguments)
            : function_(function)
            , arguments_(arguments)
        {}
        virtual ~ASTCall() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    private:
        ASTree *function_;
        ASTree *arguments_;
    };

    class ASTArrayIndex : public ASTree
    {
        friend class Visitor;
    public:
        ASTArrayIndex(ASTree *array, ASTree *index)
            : array_(array), index_(index)
        {}

        virtual ~ASTArrayIndex() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    private:
        ASTree *array_;
        ASTree *index_;
    };

    class ASTSingleExpression : public ASTree
    {
        friend class Visitor;
        
    public:
        enum {
            OP_Not,
            OP_Sub,
        };

        ASTSingleExpression(unsigned op, ASTree *expr)
            : op_(op), expr_(expr)
        {}
        virtual ~ASTSingleExpression() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

    private:
        unsigned op_;
        ASTree *expr_;
    };

    class ASTBinaryExpression : public ASTree
    {
        friend class Visitor;
        
    public:
        enum {
            OP_Add,
            OP_Sub,
            OP_Mul,
            OP_Div,
        };

        ASTBinaryExpression(unsigned op, ASTree *left, ASTree *right)
            : op_(op), left_(left), right_(right)
        {}
        virtual ~ASTBinaryExpression() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    private:
        unsigned op_;
        ASTree *left_;
        ASTree *right_;
    };

    class ASTRelationalExpression : public ASTree
    {
        friend class Visitor;

    public:
        enum {
            RL_GreatThan,
            RL_Great,
            RL_LessThan,
            RL_Less,
            RL_Equal,
            RL_NotEqual,
        };

        ASTRelationalExpression(unsigned relation, ASTree *left, ASTree *right)
            : relation_(relation), left_(left), right_(right)
        {}
        virtual ~ASTRelationalExpression() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    private:
        unsigned relation_;
        ASTree *left_;
        ASTree *right_;
    };

    class ASTAndExpression : public ASTree
    {
        friend class Visitor;
    public:
        virtual ~ASTAndExpression() = default;
        void push_back(ASTree *relation) { relations_.push_back(relation); }

        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    private:
        std::vector<ASTree*> relations_;
    };

    class ASTOrExpression : public ASTree
    {
        friend class Visitor;
    public:
        virtual ~ASTOrExpression() = default;
        void push_back(ASTree *relation) { relations_.push_back(relation); }

        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    private:
        std::vector<ASTree*> relations_;
    };

    class ASTAssignExpression : public ASTree
    {
        friend class Visitor;
    public:
        ASTAssignExpression(ASTree *left, ASTree *right)
            : left_(left), right_(right) {}
        virtual ~ASTAssignExpression() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    private:
        ASTree *left_;
        ASTree *right_;
    };

    class ASTVarDeclStatement : public ASTree
    {
        friend class Vistor;
    public:
        ASTVarDeclStatement(std::string &str, ASTree *expr)
            : name_(str), expr_(expr) {}
        virtual ~ASTVarDeclStatement() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    private:
        std::string name_;
        ASTree *expr_;
    };

    class ASTContinueStatement : public ASTree
    {
        friend class Visitor;
    public:
        virtual ~ASTContinueStatement() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    };

    class ASTBreakStatement : public ASTree
    {
        friend class Visitor;
    public:
        virtual ~ASTBreakStatement() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    };

    class ASTReturnStatement : public ASTree
    {
        friend class Visitor;
    public:
        ASTReturnStatement(ASTree *expr = nullptr) : expr_(expr) {}
        virtual ~ASTReturnStatement() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    private:
        ASTree *expr_;
    };

    class ASTWhileStatement : public ASTree
    {
        friend class Visitor;
    public:
        ASTWhileStatement(ASTree *cond, ASTree *statement)
            : condition_(cond)
            , statement_(statement)
        {}
        virtual ~ASTWhileStatement() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    private:
        ASTree *condition_;
        ASTree *statement_;
    };

    class ASTIfStatement : public ASTree
    {
        friend class Visitor;
    public:
        ASTIfStatement(ASTree *cond, ASTree *ifState)
            : condition_(cond)
            , ifStatement_(ifState)
            , elseStatement_(nullptr)
            , hasElse_(false)
        {}
        ASTIfStatement(ASTree *cond, ASTree *ifState, ASTree *elseState)
            : condition_(cond)
            , ifStatement_(ifState)
            , elseStatement_(elseStatement_)
            , hasElse_(true)
        {}
        virtual ~ASTIfStatement() = default;
        bool hasElse() const { return hasElse_; }

        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    private:
        ASTree *condition_;
        ASTree *ifStatement_;
        ASTree *elseStatement_;
        bool hasElse_;
    };

    class ASTBlock : public ASTree
    {
        friend class Visitor;
    public:
        virtual ~ASTBlock() = default;
        void push_back(ASTree *tree) { statements_.push_back(tree); }
        
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    private:
        std::vector<ASTree*> statements_;
    };

    class ASTFunction : public ASTree
    {
        friend class Visitor;
    public:
        ASTFunction(std::string &name) : name_(name) {}
        virtual ~ASTFunction() = default;
        void push_param(std::string &name) { params_.push_back(name); }
        void setBlock(ASTBlock *block) { block_ = block; }

        virtual bool accept(Visitor &v) override { return v.visit(*this); }

    private:
        std::string name_;
        std::vector<std::string> params_;
        ASTBlock *block_;
    };

    class ASTProgram : public ASTree
    {
        friend class Visitor;
    public:
        virtual ~ASTProgram() = default;
        void push_back(ASTFunction *function) { functions_.push_back(function); }
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    private:
        std::vector<ASTFunction*> functions_;
    };

    class ASTManager
    {
        ASTManager() = default;
        ~ASTManager()
        {

        }

        std::vector<ASTree*> manager_;
    public:
        static ASTManager &instance()
        {
            static ASTManager manager;
            return manager;
        }

        void push_back(ASTree *tree) { manager_.push_back(tree); }

        void destroy()
        {
            for (auto &i : manager_)
            {
                delete i;
                i = nullptr;
            }
        }
    };

    template<typename T, typename ...Args>
    T *MallocMemory(Args ...args)
    {
        auto &ins = ASTManager::instance();
        auto *buffer =  new T(args);
        ins.push_back(buffer);
        return buffer;
    }
}
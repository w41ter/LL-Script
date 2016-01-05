#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

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

    class SymbolTable
    {
    public:
        SymbolTable(SymbolTable *parent = nullptr) 
            : parent_(parent) 
        { index_ = getIndex(); }

        static int getIndex()
        {
            static int index = 1;
            return index++;
        }

        int find(std::string &str)
        {
            if (table_.count(str) == 0)
            {
                if (parent_ == nullptr)
                    return 0;
                return parent_->find(str);
            }
            return index_;
        }

        bool findInCurrent(std::string &str)
        {
            return table_.count(str) != 0;
        }

        void insert(std::string &str)
        {
            if (table_.count(str) == 0)
                table_.insert(str);
            throw std::runtime_error(str + " has been defined!");
        }

        SymbolTable *parent_;
        std::set<std::string> table_;
        int index_;
    };

    class Visitor
    {
    public:
        virtual ~Visitor() = 0 {}
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
    public:
        virtual ~ASTIdentifier() = default;
        ASTIdentifier(std::string name) : name_(name) {}

        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::string name_;
    };

    class ASTNull : public ASTree
    {
    public:
        virtual ~ASTNull() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    };

    class ASTConstant : public ASTree
    {
       
    public:
        enum {
            T_Charactor,
            T_Integer,
            T_Float,
            T_String,
        };
        virtual ~ASTConstant() = default;
        ASTConstant(char c) : type_(T_Charactor), c_(c) {}
        ASTConstant(int num) : type_(T_Integer), num_(num) {}
        ASTConstant(float fnum) : type_(T_Float), fnum_(fnum) {}
        ASTConstant(std::string &str) : type_(T_String), str_(str) {}

        virtual bool accept(Visitor &v) override { return v.visit(*this); }

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

        std::vector<ASTree*> exprs_;
    };

    class ASTArray : public ASTree
    {
    public:
        ASTArray(ASTree *tree) : array_(tree) {}
        virtual ~ASTArray() = default;
        //void push_back(ASTree *tree) { array_.push_back(tree); }

        virtual bool accept(Visitor &v) override { v.visit(*this); }

        ASTree * array_;
    };

    class ASTCall : public ASTree
    {
    public:
        ASTCall(ASTree *function, ASTree *arguments)
            : function_(function)
            , arguments_(arguments)
        {}
        virtual ~ASTCall() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        ASTree *function_;
        ASTree *arguments_;
    };

    class ASTArrayIndex : public ASTree
    {
    public:
        ASTArrayIndex(ASTree *array, ASTree *index)
            : array_(array), index_(index)
        {}

        virtual ~ASTArrayIndex() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        ASTree *array_;
        ASTree *index_;
    };

    class ASTSingleExpression : public ASTree
    {
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

        unsigned op_;
        ASTree *expr_;
    };

    class ASTBinaryExpression : public ASTree
    {
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

        unsigned op_;
        ASTree *left_;
        ASTree *right_;
    };

    class ASTRelationalExpression : public ASTree
    {
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

        unsigned relation_;
        ASTree *left_;
        ASTree *right_;
    };

    class ASTAndExpression : public ASTree
    {
    public:
        virtual ~ASTAndExpression() = default;
        void push_back(ASTree *relation) { relations_.push_back(relation); }

        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::vector<ASTree*> relations_;
    };

    class ASTOrExpression : public ASTree
    {
    public:
        virtual ~ASTOrExpression() = default;
        void push_back(ASTree *relation) { relations_.push_back(relation); }

        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::vector<ASTree*> relations_;
    };

    class ASTAssignExpression : public ASTree
    {
    public:
        ASTAssignExpression(ASTree *left, ASTree *right)
            : left_(left), right_(right) {}
        virtual ~ASTAssignExpression() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        ASTree *left_;
        ASTree *right_;
    };

    class ASTVarDeclStatement : public ASTree
    {
    public:
        ASTVarDeclStatement(std::string &str, ASTree *expr)
            : name_(str), expr_(expr) {}
        virtual ~ASTVarDeclStatement() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::string name_;
        ASTree *expr_;
    };

    class ASTContinueStatement : public ASTree
    {
    public:
        virtual ~ASTContinueStatement() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    };

    class ASTBreakStatement : public ASTree
    {
    public:
        virtual ~ASTBreakStatement() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }
    };

    class ASTReturnStatement : public ASTree
    {
    public:
        ASTReturnStatement(ASTree *expr = nullptr) : expr_(expr) {}
        virtual ~ASTReturnStatement() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        ASTree *expr_;
    };

    class ASTWhileStatement : public ASTree
    {
    public:
        ASTWhileStatement(ASTree *cond, ASTree *statement)
            : condition_(cond)
            , statement_(statement)
        {}
        virtual ~ASTWhileStatement() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        ASTree *condition_;
        ASTree *statement_;
    };

    class ASTIfStatement : public ASTree
    {
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

        ASTree *condition_;
        ASTree *ifStatement_;
        ASTree *elseStatement_;
        bool hasElse_;
    };

    class ASTBlock : public ASTree
    {
    public:
        ASTBlock(SymbolTable *parent) 
            : table_(new SymbolTable(parent)) 
        {}
        virtual ~ASTBlock()
        {
            delete table_;
        }
        void push_back(ASTree *tree) { statements_.push_back(tree); }
        
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::vector<ASTree*> statements_;
        SymbolTable *table_;
    };

    class ASTFunction : public ASTree
    {
    public:
        ASTFunction(std::string &name) : name_(name) {}
        virtual ~ASTFunction() = default;
        void push_param(std::string &name) { params_.push_back(name); }
        void setBlock(ASTBlock *block) { block_ = block; }

        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::string name_;
        std::vector<std::string> params_;
        ASTBlock *block_;
    };

    class ASTProgram : public ASTree
    {
    public:
        ASTProgram() : table_(new SymbolTable(nullptr)) {}
        virtual ~ASTProgram()
        {
            delete table_;
        }
        void push_back(ASTFunction *function) { functions_.push_back(function); }
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::vector<ASTFunction*> functions_;
        SymbolTable *table_;
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
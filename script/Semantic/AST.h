#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

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
    class ASTRelationalExpression;
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
    class ASTPrototype;

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
        virtual bool visit(ASTPrototype &v) = 0;
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
        void push_back(std::unique_ptr<ASTree> tree) 
        { 
            exprs_.push_back(std::move(tree));
        }
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::vector<std::unique_ptr<ASTree>> exprs_;
    };

    class ASTArray : public ASTree
    {
    public:
        ASTArray(std::unique_ptr<ASTree> tree) 
            : array_(std::move(tree)) 
        {}
        virtual ~ASTArray() = default;

        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::unique_ptr<ASTree> array_;
    };

    class ASTCall : public ASTree
    {
    public:
        ASTCall(std::unique_ptr<ASTree> function, 
            std::unique_ptr<ASTree> arguments)
            : function_(std::move(function))
            , arguments_(std::move(arguments))
        {}
        virtual ~ASTCall() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::unique_ptr<ASTree> function_;
        std::unique_ptr<ASTree> arguments_;
    };

    class ASTArrayIndex : public ASTree
    {
    public:
        ASTArrayIndex(std::unique_ptr<ASTree> array,
            std::unique_ptr<ASTree> index)
            : array_(std::move(array))
            , index_(std::move(index))
        {}

        virtual ~ASTArrayIndex() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::unique_ptr<ASTree> array_;
        std::unique_ptr<ASTree> index_;
    };

    class ASTSingleExpression : public ASTree
    {
    public:
        enum {
            OP_Not,
            OP_Sub,
        };

        ASTSingleExpression(unsigned op, 
            std::unique_ptr<ASTree> expr)
            : op_(op), expr_(std::move(expr))
        {}
        virtual ~ASTSingleExpression() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        unsigned op_;
        std::unique_ptr<ASTree> expr_;
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

        ASTBinaryExpression(unsigned op,
            std::unique_ptr<ASTree> left,
            std::unique_ptr<ASTree> right)
            : op_(op)
            , left_(std::move(left))
            , right_(std::move(right))
        {}
        virtual ~ASTBinaryExpression() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        unsigned op_;
        std::unique_ptr<ASTree> left_;
        std::unique_ptr<ASTree> right_;
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

        ASTRelationalExpression(unsigned relation, 
            std::unique_ptr<ASTree> left, 
            std::unique_ptr<ASTree> right)
            : relation_(relation)
            , left_(std::move(left))
            , right_(std::move(right))
        {}
        virtual ~ASTRelationalExpression() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        unsigned relation_;
        std::unique_ptr<ASTree> left_;
        std::unique_ptr<ASTree> right_;
    };

    class ASTAndExpression : public ASTree
    {
    public:
        virtual ~ASTAndExpression() = default;
        void push_back(std::unique_ptr<ASTree> relation) 
        { 
            relations_.push_back(std::move(relation)); 
        }

        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::vector<std::unique_ptr<ASTree>> relations_;
    };

    class ASTOrExpression : public ASTree
    {
    public:
        virtual ~ASTOrExpression() = default;
        void push_back(std::unique_ptr<ASTree> relation) 
        { 
            relations_.push_back(std::move(relation)); 
        }

        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::vector<std::unique_ptr<ASTree>> relations_;
    };

    class ASTAssignExpression : public ASTree
    {
    public:
        ASTAssignExpression(std::unique_ptr<ASTree> left, 
            std::unique_ptr<ASTree> right)
            : left_(std::move(left))
            , right_(std::move(right))
        {}
        virtual ~ASTAssignExpression() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::unique_ptr<ASTree> left_;
        std::unique_ptr<ASTree> right_;
    };

    class ASTVarDeclStatement : public ASTree
    {
    public:
        ASTVarDeclStatement(std::string &str, 
            std::unique_ptr<ASTree> expr)
            : name_(str), expr_(std::move(expr)) 
        {}
        virtual ~ASTVarDeclStatement() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::string name_;
        std::unique_ptr<ASTree> expr_;
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
        ASTReturnStatement(std::unique_ptr<ASTree> expr = nullptr) 
            : expr_(std::move(expr)) 
        {}
        virtual ~ASTReturnStatement() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::unique_ptr<ASTree> expr_;
    };

    class ASTWhileStatement : public ASTree
    {
    public:
        ASTWhileStatement(std::unique_ptr<ASTree> cond, 
            std::unique_ptr<ASTree> statement)
            : condition_(std::move(cond))
            , statement_(std::move(statement))
        {}
        virtual ~ASTWhileStatement() = default;
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::unique_ptr<ASTree> condition_;
        std::unique_ptr<ASTree> statement_;
    };

    class ASTIfStatement : public ASTree
    {
    public:
        ASTIfStatement(std::unique_ptr<ASTree> cond, 
            std::unique_ptr<ASTree> ifState)
            : condition_(std::move(cond))
            , ifStatement_(std::move(ifState))
            , elseStatement_(nullptr)
            , hasElse_(false)
        {}
        ASTIfStatement(std::unique_ptr<ASTree> cond,
            std::unique_ptr<ASTree> ifState, 
            std::unique_ptr<ASTree> elseState)
            : condition_(std::move(cond))
            , ifStatement_(std::move(ifState))
            , elseStatement_(std::move(elseStatement_))
            , hasElse_(true)
        {}
        virtual ~ASTIfStatement() = default;
        bool hasElse() const { return hasElse_; }

        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::unique_ptr<ASTree> condition_;
        std::unique_ptr<ASTree> ifStatement_;
        std::unique_ptr<ASTree> elseStatement_;
        bool hasElse_;
    };

    class ASTBlock : public ASTree
    {
    public:
        ASTBlock(SymbolTable *parent) 
            : table_(new SymbolTable(parent)) 
        {}

        virtual ~ASTBlock() { delete table_; }

        void push_back(std::unique_ptr<ASTree> tree)
        { 
            statements_.push_back(std::move(tree)); 
        }
        
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::vector<std::unique_ptr<ASTree>> statements_;
        SymbolTable *table_;
    };

    class ASTPrototype : public ASTree
    {
    public:
        ASTPrototype(const std::string &name,
            std::vector<std::string> args)
            : name_(name), args_(std::move(args)) 
        {}
        virtual ~ASTPrototype() = default;

        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::string name_;
        std::vector<std::string> args_;
    };

    class ASTFunction : public ASTree
    {
    public:
        ASTFunction(std::unique_ptr<ASTPrototype> proto,
            std::unique_ptr<ASTBlock> block)
            : prototype_(std::move(proto))
            , block_(std::move(block))
        {}
        virtual ~ASTFunction() = default;

        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::unique_ptr<ASTPrototype> prototype_;
        std::unique_ptr<ASTBlock> block_;
    };

    class ASTProgram : public ASTree
    {
    public:
        ASTProgram() : table_(new SymbolTable(nullptr)) {}
        virtual ~ASTProgram() { delete table_; }
        void push_back(std::unique_ptr<ASTFunction> function) 
        { 
            functions_.push_back(std::move(function)); 
        }
        virtual bool accept(Visitor &v) override { return v.visit(*this); }

        std::vector<std::unique_ptr<ASTFunction>> functions_;
        SymbolTable *table_;
    };
}
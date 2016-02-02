#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

namespace script
{
    class ASTExpressionList;
    class ASTIdentifier;
    class ASTNull;
    class ASTConstant;
    class ASTArray;
    class ASTCall;
    class ASTArrayIndex;
    class ASTSingleExpression;
    class ASTBinaryExpression;
    class ASTRelationalExpression;
    class ASTAndExpression;
    class ASTOrExpression;
    class ASTAssignExpression;
    class ASTVarDeclStatement;
    class ASTContinueStatement;
    class ASTBreakStatement;
    class ASTReturnStatement;
    class ASTWhileStatement;
    class ASTIfStatement;
    class ASTStatement;
    class ASTBlock;
    class ASTFunction;
    class ASTProgram;
    class ASTPrototype;
    class ASTDefine;
    class ASTClosure;

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

    template<typename T, T val>
    class SymbolTable
    {
    public:
        SymbolTable(SymbolTable<T, val> *parent) : parent_(parent) {}

        bool insert(const std::string &name, T t)
        {
            if (table_.count(name) == 0)
            {
                table_.insert(std::pair<std::string, T>(name, t));
                return true;
            }
            return false;
        }

        T &find(const std::string &name)
        {
            if (table_.count(name) == 0)
                return end();
            return table_[name];
        }

        T &findInTree(const std::string &name)
        {
            if (table_.count(name) == 0)
            {
                if (parent_ != nullptr)
                    return parent_->findInTree(name);
                return end();
            }
            return table_[name];
        }

        T &end() const 
        {
            static T t = val;
            return t; 
        }

    private:
        SymbolTable<T, val> *parent_;
        std::map<std::string, T> table_;
    };

    enum class SymbolType { ST_NONE, ST_Constant, ST_Variable, };
    typedef SymbolTable<SymbolType, SymbolType::ST_NONE> Symbols;

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
            std::vector<std::string> params)
            : total_(total)
            , name_(std::move(name))
            , params_(std::move(params))
        {}

        virtual ~ASTClosure() = default;
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

        int total_;
        std::string name_;
        std::vector<std::string> params_;
    };

    class ASTIdentifier : public ASTree
    {
    public:
        virtual ~ASTIdentifier() = default;
        ASTIdentifier(std::string name) : name_(name) {}

        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

        std::string name_;
    };

    class ASTNull : public ASTree
    {
    public:
        virtual ~ASTNull() = default;
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }
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
        ASTConstant(char c) : type_(T_Character), c_(c) {}
        ASTConstant(int num) : type_(T_Integer), num_(num) {}
        ASTConstant(float fnum) : type_(T_Float), fnum_(fnum) {}
        ASTConstant(std::string &str) : type_(T_String), str_(str) {}

        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

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
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

        std::vector<std::unique_ptr<ASTree>> exprs_;
    };

    class ASTArray : public ASTree
    {
    public:
        ASTArray(std::unique_ptr<ASTree> tree) 
            : array_(std::move(tree)) 
        {}
        virtual ~ASTArray() = default;

        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

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
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

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
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

        std::unique_ptr<ASTree> array_;
        std::unique_ptr<ASTree> index_;
    };

    class ASTSingleExpression : public ASTree
    {
    public:
        //enum {
        //    OP_Not,
        //    OP_Sub,
        //};

        ASTSingleExpression(unsigned op, 
            std::unique_ptr<ASTree> expr)
            : op_(op), expr_(std::move(expr))
        {}
        virtual ~ASTSingleExpression() = default;
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

        unsigned op_;
        std::unique_ptr<ASTree> expr_;
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

        ASTBinaryExpression(unsigned op,
            std::unique_ptr<ASTree> left,
            std::unique_ptr<ASTree> right)
            : op_(op)
            , left_(std::move(left))
            , right_(std::move(right))
        {}
        virtual ~ASTBinaryExpression() = default;
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

        unsigned op_;
        std::unique_ptr<ASTree> left_;
        std::unique_ptr<ASTree> right_;
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

        ASTRelationalExpression(unsigned relation, 
            std::unique_ptr<ASTree> left, 
            std::unique_ptr<ASTree> right)
            : relation_(relation)
            , left_(std::move(left))
            , right_(std::move(right))
        {}
        virtual ~ASTRelationalExpression() = default;
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

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

        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

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

        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

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
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

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
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

        std::string name_;
        std::unique_ptr<ASTree> expr_;
    };

    class ASTContinueStatement : public ASTree
    {
    public:
        virtual ~ASTContinueStatement() = default;
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }
    };

    class ASTBreakStatement : public ASTree
    {
    public:
        virtual ~ASTBreakStatement() = default;
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }
    };

    class ASTReturnStatement : public ASTree
    {
    public:
        ASTReturnStatement(std::unique_ptr<ASTree> expr = nullptr) 
            : expr_(std::move(expr)) 
        {}
        virtual ~ASTReturnStatement() = default;
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

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
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

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
            , elseStatement_(std::move(elseState))
            , hasElse_(true)
        {}
        virtual ~ASTIfStatement() = default;
        bool hasElse() const { return hasElse_; }

        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

        std::unique_ptr<ASTree> condition_;
        std::unique_ptr<ASTree> ifStatement_;
        std::unique_ptr<ASTree> elseStatement_;
        bool hasElse_;
    };

    class ASTStatement : public ASTree
    {
    public:
        ASTStatement(std::unique_ptr<ASTree> tree)
            : tree_(std::move(tree))
        {}
        virtual ~ASTStatement() = default;
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

        std::unique_ptr<ASTree> tree_;
    };

    class ASTBlock : public ASTree
    {
    public:
        ASTBlock() {}

        virtual ~ASTBlock() { }

        void push_back(std::unique_ptr<ASTree> tree)
        { 
            statements_.push_back(std::move(tree)); 
        }
        
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

        std::vector<std::unique_ptr<ASTree>> statements_;
    };

    class ASTPrototype : public ASTree
    {
    public:
        ASTPrototype(const std::string &name,
            std::vector<std::string> args)
            : name_(name), args_(std::move(args)) 
        {}
        virtual ~ASTPrototype() = default;

        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

        std::string name_;
        std::vector<std::string> args_;
    };

    class ASTFunction : public ASTree
    {
    public:
        ASTFunction(Symbols *table,
            std::unique_ptr<ASTPrototype> proto,
            std::unique_ptr<ASTBlock> block)
            : table_(table)
            , prototype_(std::move(proto))
            , block_(std::move(block))
        {}
        virtual ~ASTFunction() { delete table_; }

        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

        Symbols *table_;
        std::unique_ptr<ASTPrototype> prototype_;
        std::unique_ptr<ASTBlock> block_;
    };

    class ASTDefine : public ASTree
    {
    public:
        ASTDefine(std::string &name, std::unique_ptr<ASTree> expr)
            : name_(name), expr_(std::move(expr))
        {}
        virtual ~ASTDefine() = default;
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

        std::string name_;
        std::unique_ptr<ASTree> expr_;
    };

    class ASTProgram : public ASTree
    {
    public:
        ASTProgram(Symbols *table,
            std::vector<std::unique_ptr<ASTDefine>> defines,
            std::vector<std::unique_ptr<ASTFunction>> function)
            : table_(table)
            , defines_(std::move(defines))
            , function_(std::move(function))
        {}
        virtual ~ASTProgram() { delete table_; }
        virtual bool accept(ASTVisitor *v) override { return v->visit(this); }

        Symbols *table_;
        std::vector<std::unique_ptr<ASTDefine>> defines_;
        std::vector<std::unique_ptr<ASTFunction>> function_;
    };
}
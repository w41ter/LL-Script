#include "AST.h"

namespace script
{
    bool operator == (const Symbol &lhs, const Symbol &rhs)
    {
        return (lhs.token_ == rhs.token_ && lhs.type_ == rhs.type_);
    }

    SymbolTable::SymbolTable(SymbolTable * parent) : parent_(parent)
    {
    }

    SymbolTable * SymbolTable::getParent()
    {
        return parent_;
    }

    bool SymbolTable::insert(const std::string & name, SymbolType type, Token token)
    {
        if (table_.count(name) == 0)
        {
             table_.insert(std::pair<std::string, Symbol>(name, { token, type }));
            return true;
        }
        return false;
    }

    Symbol & SymbolTable::find(const std::string & name)
    {
        if (table_.count(name) == 0)
            return end();
        return table_[name];
    }

    Symbol & SymbolTable::findInTree(const std::string &name)
    {
        if (table_.count(name) == 0)
        {
            if (parent_ != nullptr)
                return parent_->findInTree(name);
            return end();
        }
        return table_[name];
    }

    Symbol & SymbolTable::end() const
    {
        static Symbol t = { Token(), SymbolType::ST_NONE };
        return t;
    }

    std::map<std::string, Symbol> &SymbolTable::getTables()
    {
        return table_;
    }

    ASTProgram::ASTProgram(SymbolTable * table, 
        std::vector<ASTDefine*> defines, 
        std::vector<ASTFunction*> function)
        : table_(table)
        , defines_(std::move(defines))
        , function_(std::move(function))
    {}

    ASTProgram::~ASTProgram()
    {
        delete table_;
    }

    bool ASTProgram::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTDefine::ASTDefine(std::string & name, ASTree * expr)
        : name_(name), expr_(expr)
    {
    }

    bool ASTDefine::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTClosure::ASTClosure(std::string name, int total, std::vector<std::string> params)
        : total_(total)
        , name_(std::move(name))
        , params_(std::move(params))
    {
    }

    bool ASTClosure::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTIdentifier::ASTIdentifier(std::string name) : name_(name) 
    {
    }

    bool ASTIdentifier::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    bool ASTNull::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTConstant::ASTConstant(char c, Token token) 
        : type_(T_Character), c_(c), token_(token)
    {
    }

    ASTConstant::ASTConstant(int num, Token token)
        : type_(T_Integer), num_(num), token_(token)
    {
    }

    ASTConstant::ASTConstant(float fnum, Token token)
        : type_(T_Float), fnum_(fnum), token_(token)
    {
    }

    ASTConstant::ASTConstant(std::string & str, Token token)
        : type_(T_String), str_(str), token_(token)
    {
    }

    bool ASTConstant::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    void ASTExpressionList::push_back(ASTree * tree)
    {
        exprs_.push_back(std::move(tree));
    }

    bool ASTExpressionList::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTArray::ASTArray(ASTree * tree)
        : array_(std::move(tree))
    {
    }

    bool ASTArray::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTCall::ASTCall(ASTree * function, ASTree * arguments)
        : function_(std::move(function))
        , arguments_(std::move(arguments))
    {
    }

    bool ASTCall::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTArrayIndex::ASTArrayIndex(ASTree * array, ASTree * index)
        : array_(std::move(array))
        , index_(std::move(index))
    {
    }

    bool ASTArrayIndex::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTSingleExpression::ASTSingleExpression(unsigned op, ASTree * expr)
        : op_(op), expr_(std::move(expr))
    {
    }

    bool ASTSingleExpression::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTBinaryExpression::ASTBinaryExpression(unsigned op, ASTree * left, ASTree * right)
        : op_(op)
        , left_(std::move(left))
        , right_(std::move(right))
    {
    }

    bool ASTBinaryExpression::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTRelationalExpression::ASTRelationalExpression(unsigned relation, ASTree * left, ASTree * right)
        : relation_(relation)
        , left_(std::move(left))
        , right_(std::move(right))
    {
    }

    bool ASTRelationalExpression::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    void ASTAndExpression::push_back(ASTree * relation)
    {
        relations_.push_back(std::move(relation));
    }

    bool ASTAndExpression::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    void ASTOrExpression::push_back(ASTree * relation)
    {
        relations_.push_back(std::move(relation));
    }

    bool ASTOrExpression::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTAssignExpression::ASTAssignExpression(ASTree * left, ASTree * right)
        : left_(std::move(left))
        , right_(std::move(right))
    {
    }

    bool ASTAssignExpression::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTVarDeclStatement::ASTVarDeclStatement(std::string & str, ASTree * expr)
        : name_(str), expr_(std::move(expr))
    {
    }

    bool ASTVarDeclStatement::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTContinueStatement::ASTContinueStatement(Token token)
    {
        token_ = std::move(token);
    }

    bool ASTContinueStatement::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTBreakStatement::ASTBreakStatement(Token token)
    {
        token_ = token;
    }

    bool ASTBreakStatement::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTReturnStatement::ASTReturnStatement(ASTree * expr)
        : expr_(std::move(expr))
    {
    }

    bool ASTReturnStatement::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTWhileStatement::ASTWhileStatement(ASTree * cond, ASTree * statement)
        : condition_(std::move(cond))
        , statement_(std::move(statement))
    {
    }

    bool ASTWhileStatement::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTIfStatement::ASTIfStatement(ASTree * cond, ASTree * ifState)
        : condition_(std::move(cond))
        , ifStatement_(std::move(ifState))
        , elseStatement_(nullptr)
        , hasElse_(false)
    {
    }

    ASTIfStatement::ASTIfStatement(ASTree * cond, ASTree * ifState, ASTree * elseState)
        : condition_(std::move(cond))
        , ifStatement_(std::move(ifState))
        , elseStatement_(std::move(elseState))
        , hasElse_(true)
    {
    }

    bool ASTIfStatement::hasElse() const
    {
        return hasElse_;
    }

    bool ASTIfStatement::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTStatement::ASTStatement(ASTree * tree)
        : tree_(std::move(tree))
    {
    }

    bool ASTStatement::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    void ASTBlock::push_back(ASTree * tree)
    {
        statements_.push_back(std::move(tree));
    }

    bool ASTBlock::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTPrototype::ASTPrototype(const std::string & name, std::vector<std::string> args)
        : name_(name), args_(std::move(args))
    {
    }

    bool ASTPrototype::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

    ASTFunction::ASTFunction(SymbolTable * table, ASTPrototype * proto, ASTBlock * block)
        : table_(table)
        , prototype_(std::move(proto))
        , block_(std::move(block))
    {
    }

    ASTFunction::~ASTFunction() 
    { 
        delete table_; 
    }

    bool ASTFunction::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }

}
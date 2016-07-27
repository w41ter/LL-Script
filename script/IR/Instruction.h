#pragma once

#include <list>
#include <string>
#include <vector>

namespace script
{
namespace ir 
{
    class Value;
    class User;
    class BasicBlock;

    class Use
    {
    public:
        Use(Value *value, User *user);
        ~Use();

        Value *getValue() const { return value_; }
        User *getUser() const { return user_; }

    private:
        Value *value_;
        User *user_;
        User *prev_, *next_;
    };

    class Value
    {
    public:
        // addUse/killUse - These two methods should only used by the Use class.
        void addUse(Use &u) { uses_.push_back(u); }
        void killUse(Use &u) { uses_.remove(u); }
        User *use_back() { return uses_.back().getUser(); }
    
    protected:
        std::list<Use> uses_;
        std::string name_;
    };

    class User : public Value 
    {
    protected:
        // 相对应的，User使用vector来组织Use Object
        std::vector<Use> operands_;
    public:
        typedef std::vector<Use>::iterator op_iterator;
        unsigned getNumOperands() const { return operands_.size(); }
        void op_reserve(unsigned NumElements) { operands_.reserve(NumElements); }
        op_iterator op_begin() { return operands_.begin(); }
        op_iterator op_end() { return operands_.end(); }
    };

    enum class Instructions
    {
        IR_Alloca,
        IR_Load,
        IR_Store,
        IR_Invoke,
        IR_Branch,
        IR_Assign,
        IR_NotOp,
        IR_Return,
        IR_ReturnVoid,
        IR_BinaryOps,
        IR_Index,
        IR_SetIndex,
    };

    enum class BinaryOps
    {
        BO_Add,
        BO_Sub,
        BO_Mul,
        BO_Div,
        BO_And,
        BO_Or,
        BO_Great,
        BO_NotLess,
        BO_Less,
        BO_NotGreat,
        BO_Equal,
        BO_NotEqual,
    };

    class Instruction : public User 
    {
    public:
        virtual Instructions instance() const = 0;

        Instruction(const std::string &name, Instruction *before);
        Instruction(const std::string &name, BasicBlock *end);

    protected:    
        BasicBlock *parent_;
        Instruction *prev_, *next_;
        // ...

        std::string name_;
    };

    class Alloca : public Instruction
    {
    public:
        Alloca(const std::string &name, Instruction *insertBefore)
            : Instruction(name, insertBefore)
        { }

        Alloca(const std::string &name, BasicBlock *insertAtEnd)
            : Instruction(name, insertAtEnd)
        { }

        virtual Instructions instance() const { return Instructions::IR_Alloca; }
    };

    class Load : public Instruction
    {
    public:
        Load(Value *from, const std::string &name, Instruction *insertBefore)
            : Instruction(name, insertBefore)
        {

        }

        Load(Value *from, const std::string &name, Instruction *insertAtEnd)
            : Instruction(name, insertAtEnd)
        { }

        virtual Instructions instance() const { return Instructions::IR_Load; }
    };

    class Store : public Instruction
    {
    public:
        Store(Value *value, Value *addr, Instruction *insertBefore)
            : Instruction("", insertBefore)
        { 
            init(value, addr);
        }

        Store(Value *value, Value *addr, BasicBlock *insertAtEnd)
            : Instruction("", insertAtEnd)
        { 
            init(value, addr);
        }

        virtual Instructions instance() const { return Instructions::IR_Store; }

    protected:
        void init(Value *value, Value *addr);
    };

    class Invoke : public Instruction
    {
    public:
        Invoke(const std::string functionName, const std::vector<Value*> args,
            const std::string name, Instruction *insertBefore)
            : Instruction(name, insertBefore), functionName_(functionName)
        { }

        Invoke(const std::string functionName, const std::vector<Value*> args,
            const std::string name, BasicBlock *insertAtEnd)
            : Instruction(name, insertAtEnd), functionName_(functionName)
        { }

        virtual Instructions instance() const { return Instructions::IR_Invoke; }
    private:
        std::string functionName_;
    };

    class Branch : public Instruction
    {
    public:
        Branch(Value *cond, BasicBlock *then, BasicBlock *_else,
            Instruction *insertBefore)
            : Instruction("", insertBefore)
        {

        }

        Branch(Value *cond, BasicBlock *then, BasicBlock *_else,
            BasicBlock *insertAtEnd)
            : Instruction("", insertAtEnd)
        {

        }

        virtual Instructions instance() const { return Instructions::IR_Branch; }
    };

    // TODO: 这个函数有待确认是否需要
    class Assign : public Instruction
    {
    public:
    };

    class NotOp : public Instruction
    {
    public:
        NotOp(Value *value, const std::string &name, Instruction *insertBefore)
            : Instruction(name, insertBefore)
        {}

        NotOp(Value *value, const std::string &name, BasicBlock *insertAtEnd)
            : Instruction(name, insertAtEnd)
        {}

        virtual Instructions instance() const { return Instructions::IR_NotOp; }
    };

    class Return : public Instruction
    {
    public:
        Return(Value *value, Instruction *insertBefore)
            : Instruction("", insertBefore)
        {}

        Return(Value *value, BasicBlock *insertAtEnd)
            : Instruction("", insertAtEnd)
        {}

        virtual Instructions instance() const { return Instructions::IR_Return; }
    };

    class ReturnVoid : public Instruction
    {
    public:
        ReturnVoid(Instruction *insertBefore)
            : Instruction("", insertBefore)
        {}

        ReturnVoid(BasicBlock *insertAtEnd)
            : Instruction("", insertAtEnd)
        {}

        virtual Instructions instance() const { return Instructions::IR_ReturnVoid; }
    };

    class BinaryOperator : public Instruction 
    {
    public:
        BinaryOperator(BinaryOps bop, Value *lhs, Value *rhs,
            const std::string &name, Instruction *insertBefore)
            : Instruction(name, insertBefore) {
        }
        BinaryOperator(BinaryOps bop, Value *lhs, Value *rhs,
            const std::string &name, BasicBlock *insertAtEnd)
            : Instruction(name, insertAtEnd) {
        }
        
        virtual Instructions instance() const { return Instructions::IR_BinaryOps; }

    protected:

        void init(BinaryOps iTyps, Value *S1, Value* S2);

        
    };

    class Index : public Instruction
    {
    public:
        Index(Value *table, Value *index, const std::string &name, 
            Instruction *insertBefore)
            : Instruction(name, insertBefore)
        {}

        Index(Value *table, Value *index, const std::string &name,
            Instruction *insertAtEnd)
            : Instruction(name, insertAtEnd)
        {}

        virtual Instructions instance() const { return Instructions::IR_Index; }
    };

    class SetIndex : public Instruction
    {
    public:
        SetIndex(Value *table, Value *index, Value *to, Instruction *insertBefore)
            : Instruction("", insertBefore)
        {}

        SetIndex(Value *table, Value *index, Value *to, BasicBlock *insertAtEnd)
            : Instruction("", insertAtEnd)
        {}

        virtual Instructions instance() const { return Instructions::IR_SetIndex; }
    };

}
}
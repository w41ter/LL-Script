#pragma once

#include <list>
#include <string>
#include <vector>

namespace script
{

    class BasicBlock;

namespace ir 
{
    class Value;
    class User;

    enum class Instructions
    {
        IR_Constant,
        IR_Table,
        IR_Undef,
        IR_Store,
        IR_Invoke,
        IR_Branch,
        IR_Goto,
        IR_Assign,
        IR_NotOp,
        IR_Return,
        IR_ReturnVoid,
        IR_BinaryOps,
        IR_Index,
        IR_SetIndex,
        IR_Phi,
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

    // Relation between User and Value.
    class Use
    {
    public:
        Use(Value *value, User *user);
        ~Use();

        Value *getValue() const { return value_; }
        User *getUser() const { return user_; }
        void replaceValue(Value *value);

        bool operator == (const Use &rhs)
        {
            return rhs.value_ == value_ && rhs.user_ == user_
                && prev_ == rhs.prev_ && next_ == rhs.next_;
        }
    private:
        Value *value_;
        User *user_;
        User *prev_, *next_;
    };

    class Value
    {
    public:
        virtual ~Value() = default;

        // addUse/killUse - These two methods should only used by the Use class.
        void addUse(Use &u) { uses_.push_back(u); }
        void killUse(Use &u) { uses_.remove(u); }

        typedef std::list<Use>::iterator use_iterator;
        use_iterator use_begin() { return uses_.begin(); }
        use_iterator use_end() { return uses_.end(); }
        size_t use_size() const { return uses_.size(); }

        virtual Instructions instance() const = 0;

    protected:
        std::list<Use> uses_;
    };

    class Constant : public Value
    {
    public:
        enum { Null, Boolean, Character, Integer, Float, String, };

        Constant();
        Constant(int num);
        Constant(bool state);
        Constant(char c);
        Constant(float fnum);
        Constant(std::string str);

        virtual ~Constant() = default;

        unsigned type() const { return type_; }
        int getInteger() const { return num_; }
        char getChar() const { return c_; }
        float getFloat() const { return fnum_; }
        bool getBoolean() const { return bool_; }
        const std::string &getString() const { return str_; }

        virtual Instructions instance() const { return Instructions::IR_Constant; }
    protected:
        unsigned type_;
        int num_;
        bool bool_;
        char c_;
        float fnum_;
        std::string str_;
    };

    class Table : public Value
    {
    public:
        virtual ~Table() = default;
        virtual Instructions instance() const { return Instructions::IR_Table; }
    };

    class User : public Value 
    {
    protected:
        // 相对应的，User使用vector来组织Use Object
        std::vector<Use> operands_;
    public:
        virtual ~User() = default;
        typedef std::vector<Use>::iterator op_iterator;
        op_iterator op_begin() { return operands_.begin(); }
        op_iterator op_end() { return operands_.end(); }
        unsigned getNumOperands() const { return operands_.size(); }
        void op_reserve(unsigned NumElements) { operands_.reserve(NumElements); }
        Use getOperand(size_t idx) { return operands_[idx]; }
    };

    class Instruction : public User 
    {
    public:
        virtual ~Instruction() = default;
        virtual Instructions instance() const = 0;

        Instruction(const std::string &name);

        void setParent(BasicBlock *parent);
        void eraseFromParent();
        void replaceBy(Value *val);

        const std::string &name() const { return name_; }
        BasicBlock *getParent() { return parent_; }
    protected:    
        BasicBlock *parent_;

        std::string name_;
    };

    class Undef : public Instruction
    {
    public:
        Undef(std::string &name) : Instruction(name) {}
        virtual ~Undef() = default;
        
        virtual Instructions instance() const { return Instructions::IR_Undef; }
    };

    class Store : public Instruction
    {
    public:
        Store(Value *value, const std::string &name)
            : Instruction(name)
        {
            init(value);
        }

        virtual ~Store() = default;
        virtual Instructions instance() const { return Instructions::IR_Store; }

    protected:
        void init(Value *value);
    };

    class Invoke : public Instruction
    {
    public:
        Invoke(const std::string functionName, const std::vector<Value*> &args,
            const std::string name)
            : Instruction(name)
            , functionName_(functionName)
            , callByName_(true)
        {
            init(functionName, args);
        }

        Invoke(Value *function, const std::vector<Value*> &args,
            const std::string name)
            : Instruction(name)
            , functionName_("")
            , callByName_(false)
        {
            init(function, args);
        }

        virtual ~Invoke() = default;
        virtual Instructions instance() const { return Instructions::IR_Invoke; }

        bool isCalledByName() const { return callByName_; }
        const std::string &invokedName() const { return functionName_; }
    protected:
        void init(const std::string functionName, const std::vector<Value*> &args);
        void init(Value *function, const std::vector<Value*> &args);

    protected:
        std::string functionName_;
        bool callByName_;
    };

    class Branch : public Instruction
    {
    public:
        Branch(BasicBlock *parent, Value *cond, BasicBlock *then, BasicBlock *_else)
            : Instruction(""), then_(then), else_(_else)
        {
            setParent(parent);
            init(cond);
            init(then, _else);
        }

        virtual ~Branch() = default;
        virtual Instructions instance() const { return Instructions::IR_Branch; }

        BasicBlock *then() { return then_; }
        BasicBlock *_else() { return else_; }
    protected:
        void init(Value *cond);
        void init(BasicBlock *then, BasicBlock *_else);

    protected:
        BasicBlock *then_;
        BasicBlock *else_;
    };

    class Goto : public Instruction
    {
    public:
        Goto(BasicBlock *parent, BasicBlock *block) 
            : Instruction(""), block_(block)
        {
            setParent(parent);
            init(block);
        }

        virtual ~Goto() = default;
        virtual Instructions instance() const { return Instructions::IR_Goto; }

        const BasicBlock *block() const { return block_; }
    protected:
        void init(BasicBlock *block);

    protected:
        BasicBlock *block_;
    };


    class Assign : public Instruction
    {
    public:
        Assign(Value *value, const std::string &name)
            : Instruction(name)
        {
            init(value);
        }

        virtual ~Assign() = default;
        virtual Instructions instance() const { return Instructions::IR_Assign; }

    protected:
        void init(Value *value);
    };

    class NotOp : public Instruction
    {
    public:
        NotOp(Value *value, const std::string &name)
            : Instruction(name)
        {
            init(value);
        }

        virtual ~NotOp() = default;
        virtual Instructions instance() const { return Instructions::IR_NotOp; }

    protected:
        void init(Value *value);
    };

    class Return : public Instruction
    {
    public:
        Return(Value *value)
            : Instruction("")
        {
            init(value);
        }

        virtual ~Return() = default;
        virtual Instructions instance() const { return Instructions::IR_Return; }

    protected:
        void init(Value *value);
    };

    class ReturnVoid : public Instruction
    {
    public:
        ReturnVoid()
            : Instruction("")
        {}

        virtual ~ReturnVoid() = default;
        virtual Instructions instance() const { return Instructions::IR_ReturnVoid; }
    };

    class BinaryOperator : public Instruction 
    {
    public:
        BinaryOperator(BinaryOps bop, Value *lhs, Value *rhs, std::string &name)
            : Instruction(name) 
        {
            init(bop, lhs, rhs);
        }

        virtual ~BinaryOperator() = default;
        virtual Instructions instance() const { return Instructions::IR_BinaryOps; }

        BinaryOps op() const { return op_; }
    protected:
        void init(BinaryOps bop, Value *lhs, Value* rhs);

    protected:
        BinaryOps op_;
    };

    class Index : public Instruction
    {
    public:
        Index(Value *table, Value *index, std::string &name)
            : Instruction(name)
        {
            init(table, index);
        }

        virtual ~Index() = default;
        virtual Instructions instance() const { return Instructions::IR_Index; }

        Value *table() { return operands_[0].getValue(); }
        Value *index() { return operands_[1].getValue(); }
    protected:
        void init(Value *table, Value *index);
    };

    class SetIndex : public Instruction
    {
    public:
        SetIndex(Value *table, Value *index, Value *to)
            : Instruction("")
        {
            init(table, index, to);
        }

        virtual ~SetIndex() = default;
        virtual Instructions instance() const { return Instructions::IR_SetIndex; }

    protected:
        void init(Value *table, Value *index, Value *to);
    };

    class Phi : public Instruction
    {
    public:
        Phi(std::string &name)
            : Instruction(name)
        {

        }

        Phi(std::string &name, std::initializer_list<Value*> &params)
            : Instruction(name)
        {
            init(params);
        }

        void appendOperand(Value *value);

        virtual ~Phi() = default;

        virtual Instructions instance() const { return Instructions::IR_Phi; }

    protected:
        void init(std::initializer_list<Value*> &params)
        {
            for (auto *value : params)
            {
                operands_.push_back(Use(value, this));
            }
        }
    };

}
}
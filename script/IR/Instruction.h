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
        IR_Value,
        IR_Alloca,
        IR_Load,
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
        IR_Catch,
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

    class Use
    {
    public:
        Use(Value *value, User *user);
        ~Use();

        Value *getValue() const { return value_; }
        User *getUser() const { return user_; }

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
        User *use_back() { return uses_.back().getUser(); }
    
        virtual Instructions instance() const = 0;

    protected:
        std::list<Use> uses_;
        std::string name_;
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

        virtual Instructions instance() const { return Instructions::IR_Value; }
    protected:
        unsigned type_;
        int num_;
        bool bool_;
        char c_;
        float fnum_;
        std::string str_;
    };

    class User : public Value 
    {
    protected:
        // 相对应的，User使用vector来组织Use Object
        std::vector<Use> operands_;
    public:
        virtual ~User() = default;
        typedef std::vector<Use>::const_iterator op_iterator;
        unsigned getNumOperands() const { return operands_.size(); }
        void op_reserve(unsigned NumElements) { operands_.reserve(NumElements); }
        op_iterator op_begin() const { return operands_.begin(); }
        op_iterator op_end() const { return operands_.end(); }
    };

    class Instruction : public User 
    {
        friend class BasicBlock;
    public:
        virtual ~Instruction() = default;
        virtual Instructions instance() const = 0;

        Instruction(const std::string &name, Instruction *before);
        Instruction(const std::string &name, BasicBlock *end);

        const Instruction *prev() const { return prev_; }
        const Instruction *next() const { return next_; }
        const std::string &name() const { return name_; }
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

        virtual ~Alloca() = default;
        virtual Instructions instance() const { return Instructions::IR_Alloca; }
    };

    class Load : public Instruction
    {
    public:
        Load(Value *from, const std::string &name, Instruction *insertBefore)
            : Instruction(name, insertBefore)
        {
            init(from);
        }

        Load(Value *from, const std::string &name, BasicBlock *insertAtEnd)
            : Instruction(name, insertAtEnd)
        { 
            init(from);
        }

        virtual ~Load() = default;
        virtual Instructions instance() const { return Instructions::IR_Load; }

    protected:
        void init(Value *from);
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

        virtual ~Store() = default;
        virtual Instructions instance() const { return Instructions::IR_Store; }

    protected:
        void init(Value *value, Value *addr);
    };

    class Invoke : public Instruction
    {
    public:
        Invoke(const std::string functionName, const std::vector<Value*> &args,
            const std::string name, Instruction *insertBefore)
            : Instruction(name, insertBefore)
            , functionName_(functionName)
            , callByName_(true)
        {
            init(functionName, args);
        }

        Invoke(const std::string functionName, const std::vector<Value*> &args,
            const std::string name, BasicBlock *insertAtEnd)
            : Instruction(name, insertAtEnd)
            , functionName_(functionName)
            , callByName_(true)
        {
            init(functionName, args);
        }

        Invoke(Value *function, const std::vector<Value*> &args,
            const std::string name, Instruction *insertBefore)
            : Instruction(name, insertBefore)
            , functionName_("")
            , callByName_(false)
        {
            init(function, args);
        }

        Invoke(Value *function, const std::vector<Value*> &args,
            const std::string name, BasicBlock *insertAtEnd)
            : Instruction(name, insertAtEnd)
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
        Branch(Value *cond, BasicBlock *then, BasicBlock *_else,
            Instruction *insertBefore)
            : Instruction("", insertBefore), then_(then), else_(_else)
        {
            init(cond);
            init(then, _else);
        }

        Branch(Value *cond, BasicBlock *then, BasicBlock *_else,
            BasicBlock *insertAtEnd)
            : Instruction("", insertAtEnd), then_(then), else_(_else)
        {
            init(then, _else);
            init(cond);
        }

        Branch(Value *cond, BasicBlock *then, Instruction *insertBefore)
            : Instruction("", insertBefore), then_(then), else_(nullptr)
        {
            init(cond);
            init(then);
        }

        Branch(Value *cond, BasicBlock *then, BasicBlock *insertAtEnd)
            : Instruction("", insertAtEnd), then_(then), else_(nullptr)
        {
            init(then);
            init(cond);
        }

        virtual ~Branch() = default;
        virtual Instructions instance() const { return Instructions::IR_Branch; }

        bool hasElseBlock() const { return else_ != nullptr; }
        const BasicBlock *then() const { return then_; }
        const BasicBlock *_else() const { return else_; }
    protected:
        void init(Value *cond);
        void init(BasicBlock *then, BasicBlock *_else = nullptr);

    protected:
        BasicBlock *then_;
        BasicBlock *else_;
    };

    class Goto : public Instruction
    {
    public:
        Goto(BasicBlock *block, BasicBlock *insertAtEnd) 
            : Instruction("", insertAtEnd), block_(block)
        {
            init(block);
        }

        Goto(BasicBlock *block, Instruction *insertBefore)
            : Instruction("", insertBefore)
        {
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

    // TODO: 这个函数有待确认是否需要
    class Assign : public Instruction
    {
    public:
        Assign(Value *value, const std::string &name, Instruction *insertBefore)
            : Instruction(name, insertBefore)
        {
            init(value);
        }

        Assign(Value *value, const std::string &name, BasicBlock *insertAtEnd)
            : Instruction(name, insertAtEnd)
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
        NotOp(Value *value, const std::string &name, Instruction *insertBefore)
            : Instruction(name, insertBefore)
        {
            init(value);
        }

        NotOp(Value *value, const std::string &name, BasicBlock *insertAtEnd)
            : Instruction(name, insertAtEnd)
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
        Return(Value *value, Instruction *insertBefore)
            : Instruction("", insertBefore)
        {
            init(value);
        }

        Return(Value *value, BasicBlock *insertAtEnd)
            : Instruction("", insertAtEnd)
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
        ReturnVoid(Instruction *insertBefore)
            : Instruction("", insertBefore)
        {}

        ReturnVoid(BasicBlock *insertAtEnd)
            : Instruction("", insertAtEnd)
        {}

        virtual ~ReturnVoid() = default;
        virtual Instructions instance() const { return Instructions::IR_ReturnVoid; }
    };

    class BinaryOperator : public Instruction 
    {
    public:
        BinaryOperator(BinaryOps bop, Value *lhs, Value *rhs,
            const std::string &name, Instruction *insertBefore)
            : Instruction(name, insertBefore) 
        {
            init(bop, lhs, rhs);
        }
        BinaryOperator(BinaryOps bop, Value *lhs, Value *rhs,
            const std::string &name, BasicBlock *insertAtEnd)
            : Instruction(name, insertAtEnd) 
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
        Index(Value *table, Value *index, const std::string &name, 
            Instruction *insertBefore)
            : Instruction(name, insertBefore)
        {
            init(table, index);
        }

        Index(Value *table, Value *index, const std::string &name,
            BasicBlock *insertAtEnd)
            : Instruction(name, insertAtEnd)
        {
            init(table, index);
        }

        virtual ~Index() = default;
        virtual Instructions instance() const { return Instructions::IR_Index; }

    protected:
        void init(Value *table, Value *index);
    };

    class SetIndex : public Instruction
    {
    public:
        SetIndex(Value *table, Value *index, Value *to, Instruction *insertBefore)
            : Instruction("", insertBefore)
        {
            init(table, index, to);
        }

        SetIndex(Value *table, Value *index, Value *to, BasicBlock *insertAtEnd)
            : Instruction("", insertAtEnd)
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
        Phi(std::string &name, BasicBlock *insertAtEnd,
            std::initializer_list<Value*> &params)
            : Instruction(name, insertAtEnd)
        {
            init(params);
        }

        Phi(std::string &name, Instruction *insertBefore,
            std::initializer_list<Value*> &params)
            : Instruction(name, insertBefore)
        {
            init(params);
        }
        
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

    class Catch : public Instruction
    {
    public:
        Catch(Value *value, std::string &name, BasicBlock *insertAtEnd)
            : Instruction(name, insertAtEnd)
        {
            operands_.reserve(1);
            operands_.push_back(Use(value, this));
        }

        Catch(Value *value, std::string &name, Instruction *insertBefore)
            : Instruction(name, insertBefore)
        {
            operands_.reserve(1);
            operands_.push_back(Use(value, this));
        }

        virtual ~Catch() = default;
        virtual Instructions instance() const { return Instructions::IR_Catch; }
    };
}
}
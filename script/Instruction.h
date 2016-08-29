#pragma once

#include <list>
#include <string>
#include <vector>
#include <initializer_list>

#include "User.h"
#include "Value.h"
#include "MachineRegister.h"

namespace script
{
    class BasicBlock;

    class Instruction : public User
    {
    public:
        enum InstrVal {
            InvokeVal,
            BranchVal,
            GotoVal,
            AssignVal,
            NotOpVal,
            ReturnVal,
            ReturnVoidVal,
            BinaryOpsVal,
            IndexVal,
            SetIndexVal,
            PhiVal,
        };

        Instruction(unsigned value_id);
        Instruction(unsigned value_id, const char *value_name);
        Instruction(unsigned value_id, const std::string &value_name);
        virtual ~Instruction();

        void set_parent(BasicBlock *parent);
        BasicBlock *get_parent() { return parent; }

        // does not delete it
        virtual void remove_from_parent();
        // deletes it
		virtual void erase_from_parent();
        // replace and remove.
		virtual void replace_with(Instruction *to);

        inline unsigned get_opcode() const { return opcode; };

        bool is_invoke()        const { return get_opcode() == InvokeVal; }
        bool is_branch()        const { return get_opcode() == BranchVal; }
        bool is_goto()          const { return get_opcode() == GotoVal; }
        bool is_assign()        const { return get_opcode() == AssignVal; }
        bool is_not_op()        const { return get_opcode() == NotOpVal; }
        bool is_return()        const { return get_opcode() == ReturnVal; }
        bool is_return_void()   const { return get_opcode() == ReturnVoidVal; }
        bool is_binary_ops()    const { return get_opcode() == BinaryOpsVal; }
        bool is_index()         const { return get_opcode() == IndexVal; }
        bool is_set_index()     const { return get_opcode() == SetIndexVal; }
        bool is_phi_node()      const { return get_opcode() == PhiVal; }
        bool is_output()        const {
            unsigned op = get_opcode();
            return !(op == SetIndexVal
                || op == BranchVal
                || op == GotoVal
                || op == ReturnVal
                || op == ReturnVoidVal);
        }

		void setSelfReg(MachineRegister reg) {
			self_reg = reg;
		}

		MachineRegister getSelfReg() const {
			return self_reg;
		}

		void pushUsingReg(MachineRegister reg) {
			registers.push_back({ reg });
		}

		typedef std::vector<MachineRegister>::iterator reg_iterator;
		reg_iterator reg_begin() { return registers.begin(); }
		reg_iterator reg_end()   { return registers.end(); }
		size_t reg_size()        { return registers.size(); }
		const MachineRegisters &refRegisters() const {
			return registers; 
		}

		void setID(unsigned ID) { opID = ID; }
		unsigned getID() const { return opID; }
    protected:    
        BasicBlock *parent;

		unsigned opID;
        const unsigned opcode;
		MachineRegister self_reg;
        MachineRegisters registers;
    };

    class Invoke : public Instruction
    {
    public:
        Invoke(Value *func, const std::vector<Value*> &args, const char *name);
        Invoke(Value *func, const std::vector<Value*> &args, const std::string &name);
        virtual ~Invoke() = default;
        
        typedef std::vector<Use>::iterator param_iterator;
        param_iterator param_begin() { return ++operands.begin();}
        param_iterator param_end() { return operands.end(); }
        Value *get_func();

		void enable_tail_call() { tail_call = true; }
		bool is_enable_tail_call() const { return tail_call; }
    protected:
        void init(Value *function, const std::vector<Value*> &args);

		bool tail_call;
    };

    class Branch : public Instruction
    {
    public:
        Branch(Value *cond, BasicBlock *then, BasicBlock *_else);
        virtual ~Branch() = default;

        Value *get_cond();
        BasicBlock *then() { return then_; }
        BasicBlock *_else() { return else_; }
		void setThen(BasicBlock *then) {
			then_ = then;
		}
		void setElse(BasicBlock *_else) {
			else_ = _else;
		}
    protected:
        void init(Value *cond);

    protected:
        BasicBlock *then_;
        BasicBlock *else_;
    };

    class Goto : public Instruction
    {
    public:
        Goto(BasicBlock *block);
        virtual ~Goto() = default;

		void setTarget(BasicBlock *block) {
			block_ = block;
		}

        BasicBlock *block() { return block_; }

    protected:
        BasicBlock *block_;
    };

    class Assign : public Instruction
    {
    public:
        Assign(Value *value, const char *name);
        Assign(Value *value, const std::string &name);
        virtual ~Assign() = default;
        
        Value *get_value();

    protected:
        void init(Value *value);
    };

    class NotOp : public Instruction
    {
    public:
        NotOp(Value *value, const char *name);
        NotOp(Value *value, const std::string &name);
        virtual ~NotOp() = default;

        Value *get_value();
    protected:
        void init(Value *value);
    };

    class Return : public Instruction
    {
    public:
        Return(Value *value);
        virtual ~Return() = default;

        Value *get_value();
		bool is_tail_call_return() const { return tail_call; }
    protected:
        void init(Value *value);
		bool tail_call;
    };

    class ReturnVoid : public Instruction
    {
    public:
        ReturnVoid();
        virtual ~ReturnVoid() = default;
    };

    class BinaryOperator : public Instruction 
    {
    public:
        enum BinaryOps {
            Add,
            Sub,
            Mul,
            Div,
            And,
            Or,
            Great,
            NotLess,
            Less,
            NotGreat,
            Equal,
            NotEqual,
        };

        BinaryOperator(unsigned ops, Value *lhs, Value *rhs, const char *name);
        BinaryOperator(unsigned ops, Value *lhs, Value *rhs, const std::string &name);
        virtual ~BinaryOperator() = default;

        unsigned op() const { return op_; }
        Value *get_lhs();
        Value *get_rhs();
    protected:
        void init(Value *lhs, Value* rhs);

    protected:
        unsigned op_;
    };

    class Index : public Instruction
    {
    public:
        Index(Value *table, Value *index, const char *name);
        Index(Value *table, Value *index, const std::string &name);
        virtual ~Index() = default;

        Value *table();
        Value *index();
    protected:
        void init(Value *table, Value *index);
    };

    class SetIndex : public Instruction
    {
    public:
        SetIndex(Value *table, Value *index, Value *to);
        virtual ~SetIndex() = default;
        
        Value *table();
        Value *index();
        Value *to();
    protected:
        void init(Value *table, Value *index, Value *to);
    };

    class Phi : public Instruction
    {
    public:
        Phi(const char *name);
        Phi(const std::string &name);
        Phi(const char *name, std::initializer_list<Value*> &params);
        Phi(const std::string &name, std::initializer_list<Value*> &params);
        virtual ~Phi() = default;

        void appendOperand(Value *value);

    protected:
        void init(std::initializer_list<Value*> &params);
    };
}
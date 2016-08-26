#pragma once 

#include <list>
#include <string>

namespace script
{
    class Use;

    class Value 
    {
        friend class Use;
    public:
        enum ValueTy {
            UndefVal,
            TableVal,
            ParamVal,
            FunctionVal,
            ConstantVal,
            InstructionVal,
        };

        typedef std::list<Use*>::iterator use_iterator;

        Value(unsigned scid);
        virtual ~Value();

        use_iterator use_begin() { return uses.begin(); }
        use_iterator use_end() { return uses.end(); }
        size_t use_size() const { return uses.size(); }

        void set_value_name(const char *name);
        void set_value_name(const std::string &name);
        const std::string &get_value_name() const;

        void replace_all_uses_with(Value *value);
        void unchecked_replace_all_uses_with(Value *value);

        unsigned get_subclass_id() const { return subclass_id; }
        bool is_value() const { return !is_instr(); }
		bool is_undef() const { return get_subclass_id() == UndefVal; }
		bool is_table() const { return get_subclass_id() == TableVal; }
		bool is_param() const { return get_subclass_id() == ParamVal; }
		bool is_const() const { return get_subclass_id() == ConstantVal; }
		bool is_funct() const { return get_subclass_id() == FunctionVal; }
		bool is_instr() const { return get_subclass_id() == InstructionVal; }
    protected:
        std::list<Use*> uses;
        std::string value_name;

    private:
        // These two methods should only used by the Use class.
        void add_use(Use *use);
        void kill_use(Use *use);

        const unsigned short subclass_id;
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

        unsigned type()         const { return type_; }
        int     getInteger()    const { return num_; }
        char    getChar()       const { return c_; }
        float   getFloat()      const { return fnum_; }
        bool    getBoolean()    const { return bool_; }
        const std::string &getString() const { return str_; }

    protected:
        unsigned    type_;
        int         num_;
        bool        bool_;
        char        c_;
        float       fnum_;
        std::string str_;
    };

    class Table : public Value
    {
    public:
        Table() : Value(ValueTy::TableVal) {}
        virtual ~Table() = default;
    };

    class Undef : public Value
    {
    public:
        Undef() : Value(ValueTy::UndefVal) {}
        virtual ~Undef() = default;
    };

    class Function : public Value 
    {
    public:
        Function(const char *funcname) 
            : Value(ValueTy::FunctionVal) 
        {
            this->set_value_name(funcname);
        }

        Function(const std::string &name) 
            : Value(ValueTy::FunctionVal)
        {
            this->set_value_name(name.c_str());
        }

        const std::string &getFuncName() const
        {
            return get_value_name();
        }
    };

	class Param : public Value
	{
	public:
		Param(const std::string &name) 
			: Value(ValueTy::ParamVal) 
		{
			this->set_value_name(name);
		}

		virtual ~Param() = default;
		const std::string &getParamName() const
		{
			return get_value_name();
		}
	};
}
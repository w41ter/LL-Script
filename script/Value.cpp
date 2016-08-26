#include "Value.h"

#include <cassert>
#include <string>

#include "Use.h"

using std::string;

namespace script
{
    Value::Value(unsigned scid)
        : subclass_id(scid), value_name("")
    {

    }

	Value::~Value()
	{
		unchecked_replace_all_uses_with(0);
		assert(uses.size() == 0);
	}

    void Value::add_use(Use *use)
    {
        assert(use);
        this->uses.push_back(use);
    }

    void Value::kill_use(Use *use) 
    {
        assert(use);
        this->uses.remove(use);
    }

    void Value::set_value_name(const char *name)
    {
        assert(name);
        value_name = std::string{ name };
    }

    void Value::set_value_name(const std::string &name) 
    {
        this->set_value_name(name.c_str());
    }

    const std::string &Value::get_value_name() const  
    {
        return this->value_name;
    }

    void Value::replace_all_uses_with(Value *value)
    {
        assert(value && "Value::replace_all_uses_with(null) is invalid!");
        assert(value != this && "Value::replace_all_uses_with(this) is invalid!");
        this->unchecked_replace_all_uses_with(value);
    }
    
    void Value::unchecked_replace_all_uses_with(Value *value)
    {
		while (!uses.empty()) {
			Use *use = uses.front();
			use->set(value);
		}
    }

    Constant::Constant()
        : Value(ValueTy::ConstantVal), type_(Null)
    {
    }

    Constant::Constant(int num)
        : Value(ValueTy::ConstantVal), type_(Integer), num_(num)
    {}

    Constant::Constant(bool state)
        : Value(ValueTy::ConstantVal), type_(Boolean), bool_(state)
    {
    }

    Constant::Constant(char c)
        : Value(ValueTy::ConstantVal), type_(Character), c_(c)
    {}

    Constant::Constant(float fnum)
        : Value(ValueTy::ConstantVal), type_(Float), fnum_(fnum)
    {}

    Constant::Constant(string str)
        : Value(ValueTy::ConstantVal), type_(String), str_(str)
    {}
}
#include "Use.h"

#include "Value.h"
#include "User.h"

namespace script
{
	Use::Use(const Use & rhs)
	{
		init(rhs.value, rhs.user);
	}

	Use::Use(const Use && rhs)
	{
		init(rhs.value, rhs.user);
	}

	Use::Use(Value *value, User *user)
    {
		init(value, user);
    }

    Use::~Use() 
    {
        if (this->value)
        {
            this->value->kill_use(this);
        }
    }

	void Use::init(Value *value, User *user)
	{
		this->value = value;
		this->user = user;
		if (this->value)
		{
			this->value->add_use(this);
		}
		user->unique();
	}

	Use &Use::operator = (const Use &rhs) {
		this->user = rhs.user;
		set(rhs.value);
		return *this;
	}

    bool Use::operator == (const Use &rhs) 
    {
        return this->value == rhs.value 
            && this->user == rhs.user;
    }

    void Use::set(Value *value) 
    {
        if (this->value)
            this->value->kill_use(this); 
        this->value = value;
		if (this->value) 
            this->value->add_use(this);
		this->user->unique();
    }
}
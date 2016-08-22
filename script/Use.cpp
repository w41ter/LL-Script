#include "Use.h"

#include "Value.h"

namespace script
{
    Use::Use(Value *value, User *user)
    {
        this->value = value;
        this->user = user;
        if (this->value) 
        {
            this->value->add_use(this);
        }
    }

    Use::~Use() 
    {
        if (this->value)
        {
            this->value->kill_use(this);
        }
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
    }
}
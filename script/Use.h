#pragma once 

namespace script
{
    class Value;
    class User;

    class Use 
    {
    public:
        Use(Value *value, User *user);
        ~Use();

        Value *get_value() { return value; }
        User *get_user() { return user; }

        bool operator == (const Use &rhs);

        void set(Value *value);
    protected:
        Value *value;
        User *user;
    };
}
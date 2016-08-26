#pragma once 

namespace script
{
    class Value;
    class User;

    class Use 
    {
    public:
		Use(const Use &rhs);
		Use(const Use &&rhs);
        Use(Value *value, User *user);
        ~Use();

		void init(Value *value, User *user);
        Value *get_value() { return value; }
        User *get_user() { return user; }

		Use &operator = (const Use &rhs);
        bool operator == (const Use &rhs);

        void set(Value *value);
    protected:
        Value *value;
        User *user;
    };
}
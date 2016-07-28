#pragma once

#include <list>

namespace script
{
    class Value;

    class IRContext
    {
    public:
        ~IRContext();

        template<typename T, typename ...Args>
        T *create(Args ...args)
        {
            auto *buffer = new T(args...);
            codes_.push_back(buffer);
            return buffer;
        }

    protected:
        std::list<Value*> codes_;
    };
}
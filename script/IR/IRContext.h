#pragma once

#include <list>

namespace script
{
namespace ir
{
    class Value;
}

    class IRContext
    {
    public:
        ~IRContext();

        template<typename T, typename ...Args>
        ir::Value *create(Args ...args)
        {
            auto *buffer = new T(args...);
            codes_.push_back(buffer);
            return buffer;
        }

    protected:
        std::list<ir::Value*> codes_;
    };
}
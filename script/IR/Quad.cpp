#include <list>

#include "Quad.h"

namespace script
{

    class QuadManager
    {
        QuadManager() = default;
        ~QuadManager() { destroy(); }

        std::list<Quad*> manager_;
        std::list<Value*> values_;
    public:
        static QuadManager &instance()
        {
            static QuadManager manager;
            return manager;
        }

        void push_back(Quad *quad) { manager_.push_back(quad); }
        void insert_value(Value *value) { values_.push_back(value); }

        void destroy()
        {
            for (auto &i : manager_)
                delete i, i = nullptr;
            for (auto &i : values_)
                delete i, i = nullptr;
        }
    };

    template<typename T, typename ...Args>
    T *Quad::Create(Args ...args)
    {
        auto &ins = QuadManager::instance();
        auto *buffer = new T(args...);
        ins.push_back(buffer);
        return buffer;
    }

    template<typename T, typename ...Args>
    T * Quad::CreateValue(Args ...args)
    {
        auto &ins = QuadManager::instance();
        auto *buffer = new T(args...);
        ins.insert_value(buffer);
        return buffer;
    }
}

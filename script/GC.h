#ifndef __GC_H__
#define __GC_H__

#include <functional>
#include "Runtime.h"

namespace script
{
    struct Semispace
    {
        Semispace(size_t size);
        ~Semispace();

        Object allocateMemory(size_t size);
        bool contains(Object obj);
        void reset();

        char* bottom_;
        char* top_;
        char* end_;
		size_t space_size_;
        size_t free_space_;
    };

    class GarbageCollector
    {
        using GloablVariable = void();
        using VariableReference = void(Object *);
    public:
        GarbageCollector(size_t size);
        ~GarbageCollector();

        Object allocate(size_t size);

        void bindReference(std::function<VariableReference> call);
        void bindGlobals(std::function<GloablVariable> call);
        void processReference(Object *slot);

    private:
        void garbageCollect();

        void swapSpace();

        bool isForwarded(Object obj);
        void forwardTo(Object obj, Object new_addr);
        Object forwardee(Object obj);

        Object swap(Object obj, size_t size);

    private:
        std::function<VariableReference> variableReference_;
        std::function<GloablVariable> globalVariable_;

        Semispace *from_space_;
        Semispace *to_space_;
        size_t size_;
        size_t space_size_;
        size_t free_space_;
        Object *forward_;
    };

}

#endif // !__GC_H__

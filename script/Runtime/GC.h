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

        Pointer allocateMemory(size_t size);
        bool contains(Pointer obj);
        void reset();

        char* bottom_;
        char* top_;
        char* end_;
    };

    class GC
    {
        using GloablVariable = void();
        using VariableReference = void(Pointer *);
    public:
        GC(size_t size);
        ~GC();

        Pointer allocate(size_t size);

        void bindReference(std::function<VariableReference> call);
        void bindGlobals(std::function<GloablVariable> call);
        void processReference(Pointer *slot);

    private:
        void garbageCollect();

        void swapSpace();

        bool isForwarded(Pointer obj);
        void forwardTo(Pointer obj, Pointer new_addr);
        Pointer forwardee(Pointer obj);

        Pointer swap(Pointer obj, size_t size);

    private:
        std::function<VariableReference> variableReference_;
        std::function<GloablVariable> globalVariable_;

        Semispace *from_space_;
        Semispace *to_space_;
        size_t size_;
        size_t space_size_;
        Pointer *forward_;
    };

}

#endif // !__GC_H__

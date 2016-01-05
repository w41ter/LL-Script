#ifndef __GC_H__
#define __GC_H__

#include <functional>
#include "Runtime.h"

namespace script
{
    size_t Ceil(size_t size)
    {
        int c = size % sizeof(Pointer);
        if (c != 0) c = sizeof(Pointer) - c;
        return size + c;
    }

    struct Semispace
    {
        Semispace(size_t size)
        {
            size_t space_size = Ceil(size);
            bottom_ = new char[space_size];
            end_ = bottom_ + space_size;
            top_ = bottom_;
        }

        ~Semispace()
        {
            delete bottom_;
        }

        Pointer allocateMemory(size_t size)
        {
            size_t space_size = Ceil(size);
            if (top_ + space_size > end_)
                return (Pointer)nullptr;
            Pointer obj = (Pointer)top_;
            top_ += space_size;
            return obj;
        }

        bool contains(Pointer obj)
        {
            return ((Pointer)bottom_ <= obj && obj < (Pointer)end_);
        }

        void reset()
        {
            top_ = bottom_;
        }

        char* bottom_;
        char* top_;
        char* end_;
    };

    class GC
    {
    public:
        GC(size_t size)
        {
            this->size_ = size;
            this->space_size_ = Ceil(size) >> 2;
            this->from_space_ = new Semispace(space_size_);
            this->to_space_ = new Semispace(space_size_);
        }

        ~GC()
        {
            delete from_space_;
            delete to_space_;
        }

        void swapSpace()
        {
            Semispace* temp = from_space_;
            from_space_ = to_space_;
            to_space_ = temp;

            // After swapping, the to-space is assumed to be empty.
            // Reset its allocation pointer.
            to_space_->reset();
        }

        // offset / 4 is the index of obj's forward.
        bool isForwarded(Pointer obj)
        {
            size_t offset = obj - (Pointer)from_space_->bottom_;
            int *forward = (int*)this->forward_;
            return (forward[offset >> 2] != NULL);
        }

        // set the obj's forward to new addr.
        void forwardTo(Pointer obj, Pointer new_addr)
        {
            size_t offset = obj -(Pointer)from_space_->bottom_;
            int *forward = (int*)this->forward_;
            forward[offset >> 2] = new_addr;
        }

        // get the new addr of obj.
        Pointer forwardee(Pointer obj)
        {
            size_t offset = obj - (Pointer)from_space_->bottom_;
            int *forward = (int*)this->forward_;
            return forward[offset >> 2];
        }

        // copy object to to_space.
        Pointer swap(Pointer obj, size_t size)
        {
            Pointer dest = to_space_->allocateMemory(size);
            memcpy((void*)dest, (void*)obj, size);
            return dest;
        }

        void processReference(Pointer *slot);

        void processObjectReference(Pointer *slot)
        {
            variable_use(slot, ProcessReference);
        }

        void garbageCollect();

        Pointer allocate(size_t size);

        Semispace *from_space_;
        Semispace *to_space_;
        size_t size_;
        size_t space_size_;
        Pointer *forward_;
    };

}

#endif // !__GC_H__

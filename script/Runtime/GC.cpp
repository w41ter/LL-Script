#include <stdexcept>
#include <assert.h>

#include "GC.h"

namespace script
{
    size_t Ceil(size_t size)
    {
        int c = size % sizeof(Pointer);
        if (c != 0) c = sizeof(Pointer) - c;
        return size + c;
    }

    Semispace::Semispace(size_t size)
    {
        size_t space_size = Ceil(size);
        bottom_ = new char[space_size];
        end_ = bottom_ + space_size;
        top_ = bottom_;
    }

    Semispace::~Semispace()
    {
        delete []bottom_;
    }

    inline bool Semispace::contains(Pointer obj)
    {
        return ((Pointer)bottom_ <= obj && obj < (Pointer)end_);
    }

    inline void Semispace::reset()
    {
        top_ = bottom_;
    }

    Pointer Semispace::allocateMemory(size_t size)
    {
        size_t space_size = Ceil(size);
        if (top_ + space_size > end_)
            return (Pointer)nullptr;
        Pointer obj = (Pointer)top_;
        top_ += space_size;
        return obj;
    }


    GC::GC(size_t size)
    {
        this->size_ = size;
        this->space_size_ = Ceil(size) >> 2;
        this->from_space_ = new Semispace(space_size_);
        this->to_space_ = new Semispace(space_size_);
    }

    GC::~GC()
    {
        delete from_space_;
        delete to_space_;
    }

    void GC::swapSpace()
    {
        Semispace* temp = from_space_;
        from_space_ = to_space_;
        to_space_ = temp;

        // After swapping, the to-space is assumed to be empty.
        // Reset its allocation pointer.
        to_space_->reset();
    }

    // offset / 4 is the index of obj's forward.
    bool GC::isForwarded(Pointer obj)
    {
        size_t offset = obj - (Pointer)from_space_->bottom_;
        int *forward = (int*)this->forward_;
        return (forward[offset >> 2] != NULL);
    }

    // set the obj's forward to new addr.
    void GC::forwardTo(Pointer obj, Pointer new_addr)
    {
        size_t offset = obj - (Pointer)from_space_->bottom_;
        int *forward = (int*)this->forward_;
        forward[offset >> 2] = new_addr;
    }

    // get the new addr of obj.
    Pointer GC::forwardee(Pointer obj)
    {
        size_t offset = obj - (Pointer)from_space_->bottom_;
        int *forward = (int*)this->forward_;
        return forward[offset >> 2];
    }

    // copy object to to_space.
    Pointer GC::swap(Pointer obj, size_t size)
    {
        Pointer dest = to_space_->allocateMemory(size);
        memcpy((void*)dest, (void*)obj, size);
        return dest;
    }

    void GC::processReference(Pointer *slot)
    {
        size_t size = SizeOfObject(*slot);
        if (size <= 0)
            return;

        Pointer obj = *slot;
        if (obj == NULL) return;

        if (from_space_->contains(obj))
        {
            if (!isForwarded(obj))
            {
                Pointer new_obj = swap(obj, size);
                forwardTo(obj, new_obj);
                *slot = new_obj;
            }
            else
            {
                *slot = forwardee(obj);
            }
        }
    }

    void GC::garbageCollect()
    {
        forward_ = new Pointer[Ceil(space_size_)];
        memset((void*)forward_, 0, Ceil(space_size_));

        // queue
        Pointer scanned = (Pointer)to_space_->bottom_;

        // copy all glboal variables
        globalVariable_();

        // breadth-first scanning of object graph
        while (scanned < (Pointer)to_space_->top_)
        {
            Pointer parent_obj = scanned;
            variableReference_(&parent_obj);
            scanned += SizeOfObject(parent_obj);
        }

        // Now all live objects will have been evacuated into the to-space,
        // and we don't need the data in the from-space anymore.
        swapSpace();

        delete[]forward_;
        forward_ = nullptr;
    }

    Pointer GC::allocate(size_t size)
    {
        assert(variableReference_ && globalVariable_);

        Pointer address = (Pointer)from_space_->allocateMemory(size);
        if (address == NULL)
        {
            garbageCollect();
            address = from_space_->allocateMemory(size);
            if (address == NULL)
            {
                // error
                throw std::runtime_error("Allocate memory failure!");
            }
        }
        return address;
    }

    void GC::bindReference(std::function<VariableReference> call)
    {
        variableReference_ = std::move(call);
    }

    void GC::bindGlobals(std::function<GloablVariable> call)
    {
        globalVariable_ = std::move(call);
    }
}

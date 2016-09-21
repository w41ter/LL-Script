#include <stdexcept>
#include <assert.h>
#include <iostream>
#include <string.h>

#include "GC.h"

namespace script
{
    size_t Ceil(size_t size)
    {
        int c = size % sizeof(Object);
        if (c != 0) c = sizeof(Object) - c;
        return size + c;
    }

    Semispace::Semispace(size_t size)
    {
        space_size_ = Ceil(size);
        bottom_ = new char[space_size_];
        end_ = bottom_ + space_size_;
        top_ = bottom_;
        free_space_ = space_size_;
    }

    Semispace::~Semispace()
    {
        delete []bottom_;
    }

    inline bool Semispace::contains(Object obj)
    {
        return ((Object)bottom_ <= obj && obj < (Object)end_);
    }

    inline void Semispace::reset()
    {
        top_ = bottom_;
		free_space_ = space_size_;
    }

    Object Semispace::allocateMemory(size_t size)
    {
        size_t space_size = Ceil(size);
        if (top_ + space_size > end_)
            return (Object)nullptr;
        Object obj = (Object)top_;
        top_ += space_size;
        free_space_ -= size;
        return obj;
    }


    GarbageCollector::GarbageCollector(size_t size)
    {
        this->size_ = size;
        this->space_size_ = Ceil(size) >> 2;
        this->from_space_ = new Semispace(space_size_);
        this->to_space_ = new Semispace(space_size_);
    }

    GarbageCollector::~GarbageCollector()
    {
        delete from_space_;
        delete to_space_;
    }

    void GarbageCollector::swapSpace()
    {
        Semispace* temp = from_space_;
        from_space_ = to_space_;
        to_space_ = temp;

        // After swapping, the to-space is assumed to be empty.
        // Reset its allocation Object.
        to_space_->reset();
    }

    // offset / 4 is the index of obj's forward.
    bool GarbageCollector::isForwarded(Object obj)
    {
        size_t offset = obj - (Object)from_space_->bottom_;
        int *forward = (int*)this->forward_;
        return (forward[offset >> 2] != 0);
    }

    // set the obj's forward to new addr.
    void GarbageCollector::forwardTo(Object obj, Object new_addr)
    {
        size_t offset = obj - (Object)from_space_->bottom_;
        int *forward = (int*)this->forward_;
        forward[offset >> 2] = new_addr;
    }

    // get the new addr of obj.
    Object GarbageCollector::forwardee(Object obj)
    {
        size_t offset = obj - (Object)from_space_->bottom_;
        int *forward = (int*)this->forward_;
        return forward[offset >> 2];
    }

    // copy object to to_space.
    Object GarbageCollector::swap(Object obj, size_t size)
    {
        Object dest = to_space_->allocateMemory(size);
        memcpy((void*)dest, (void*)obj, size);
        return dest;
    }

    void GarbageCollector::processReference(Object *slot)
    {
        size_t size = SizeOfObject(*slot);
        if (size <= 0)
            return;

        Object obj = *slot;
        if (obj == 0) return;

        if (from_space_->contains(obj))
        {
            if (!isForwarded(obj))
            {
                Object new_obj = swap(obj, size);
                forwardTo(obj, new_obj);
                *slot = new_obj;
            }
            else
            {
                *slot = forwardee(obj);
            }
        }
    }

    void GarbageCollector::garbageCollect()
    {
        size_t old = from_space_->free_space_;

        forward_ = new Object[Ceil(space_size_)];
        memset((void*)forward_, 0, Ceil(space_size_));

        // queue
        Object scanned = (Object)to_space_->bottom_;

        // copy all glboal variables
        globalVariable_();

        // breadth-first scanning of object graph
        while (scanned < (Object)to_space_->top_)
        {
            Object parent_obj = scanned;
            variableReference_(&parent_obj);
            scanned += SizeOfObject(parent_obj);
        }

        // Now all live objects will have been evacuated into the to-space,
        // and we don't need the data in the from-space anymore.
        swapSpace();

        delete[]forward_;
        forward_ = nullptr;

#ifdef _DEBUG
        std::cout << "[GC] before free: " << old
            << ", after free: " << from_space_->free_space_
			<< ", used space:" << from_space_->space_size_ - from_space_->free_space_
            << ", total free space: " << from_space_->free_space_ - old << std::endl;
#endif // _DEBUG
	}

    Object GarbageCollector::allocate(size_t size)
    {
        assert(variableReference_ && globalVariable_);

        Object address = (Object)from_space_->allocateMemory(size);
        if (address == 0)
        {
            garbageCollect();
            address = from_space_->allocateMemory(size);
            if (address == 0)
            {
                // error
                throw std::runtime_error("Allocate memory failure!");
            }
        }
        return address;
    }

    void GarbageCollector::bindReference(std::function<VariableReference> call)
    {
        variableReference_ = std::move(call);
    }

    void GarbageCollector::bindGlobals(std::function<GloablVariable> call)
    {
        globalVariable_ = std::move(call);
    }
}

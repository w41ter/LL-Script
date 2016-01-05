#include "GC.h"

namespace script
{
    void GC::processReference(Pointer *slot)
    {
        size_t size = scheme->SizeOfObject(*slot);
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
        Pointer scanned = to_space_->bottom_;

        // copy all glboal variables
        //scheme->global_variables(scheme, ProcessReference);

        // breadth-first scanning of object graph
        while (scanned < to_space_->top_)
        {
            Pointer parent_obj = scanned;
            ProcessObjectReference(&parent_obj);
            scanned += SizeOfObject((void*)parent_obj);
        }

        // Now all live objects will have been evacuated into the to-space,
        // and we don't need the data in the from-space anymore.
        swapSpace();

        delete[]forward_;
        forward_ = nullptr;
    }

    Pointer GC::allocate(size_t size)
    {
        Pointer address = from_space->allocateMemory(size);
        if (address == NULL)
        {
            garbageCollect();
            address = from_space->allocateMemory(size);
            if (address == NULL)
            {
                // error
            }
            else
            {
                return address;
            }
        }
        else
        {
            return address;
        }
    }
}

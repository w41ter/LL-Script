#ifndef __REGISTER_ALLOCATOR_H__
#define __REGISTER_ALLOCATOR_H__

#include "cfg.h"
#include <vector>

namespace script
{
    struct Register;
    struct UseDefine;

    class RegisterAllocator
    {
    public:
        RegisterAllocator(BasicBlock *block);

        Register allocate(Temp *temp);
        int getReg(Temp *temp);
    private:
        std::vector<UseDefine*> temps_;
        Register register_[16];
    };
}

#endif // !__REGISTER_ALLOCATOR_H__

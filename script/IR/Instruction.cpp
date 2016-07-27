#include "Instruction.h"

namespace script
{
namespace ir
{
    Use::Use(Value *value, User *user)
        : value_(value), user_(user)
    {
        if (value)
            value->addUse(*this);
    }

    Use::~Use()
    {
        if (value_)
            value_->killUse(*this);
    }
}
}
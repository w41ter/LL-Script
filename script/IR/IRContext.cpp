#include "IRContext.h"

namespace script
{
    IRContext::~IRContext()
    {
        for (Value * value : codes_)
            delete value;
    }
}



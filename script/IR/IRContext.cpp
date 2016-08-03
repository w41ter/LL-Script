#include "IRContext.h"

namespace script
{
    IRContext::~IRContext()
    {
        for (ir::Value * value : codes_)
            delete value;
    }
}



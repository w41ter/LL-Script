#include "IRModule.h"

#include "IRContext.h"
#include "SymbolTable.h"

namespace script
{
    IRCode::IRCode() : context_(new IRContext()), table_(new SymbolTable())
    {

    }

    IRCode::~IRCode()
    {
        if (context_ != nullptr)
            delete context_;
        if (table_ != nullptr)
            delete context_;
    }

    IRContext *IRCode::getContext()
    {
        return context_;
    }

    SymbolTable *IRCode::getTable()
    {
        return table_;
    }
}
#include "SymbolTable.h"

namespace script
{
    SymbolTable::SymbolTable()
        : parent_(nullptr)
    {
    }

    SymbolTable::~SymbolTable()
    {
    }

    void SymbolTable::bindParent(SymbolTable * parent)
    {
        parent_ = parent;
    }

    void SymbolTable::insert(std::string & name, Symbol & symbol)
    {
        table_[name] = symbol;
    }

    unsigned SymbolTable::find(std::string & name, Symbol & symbol)
    {
        auto sym = table_.find(name);
        if (sym != table_.end())
        {
            symbol = sym->second;
            return sym->second.type_;
        }
        return Symbol::None;
    }

    void SymbolTable::setCatched(std::string & name)
    {
        auto symbol = table_.find(name);
        if (symbol != table_.end())
        {
            symbol->second.beCaught_ = true;
        }
    }

}

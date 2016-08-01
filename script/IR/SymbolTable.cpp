#include "SymbolTable.h"

namespace script
{
    SymbolTable::SymbolTable()
    {
    }

    SymbolTable::~SymbolTable()
    {
    }

    void SymbolTable::bindParent(const SymbolTable * parent)
    {
        parent_ = parent;
    }

    void SymbolTable::insertDefines(std::string & str, Token & token)
    {
        defines_[str] = token;
    }

    void SymbolTable::insertVariables(std::string & str, Token & token)
    {
        variables_[str] = token;
    }

    unsigned SymbolTable::findName(std::string & str) const
    {
        if (variables_.count(str) != 0)
            return SymbolTable::Let;
        else if (defines_.count(str) != 0)
            return SymbolTable::Define;
        else
            return SymbolTable::None;
    }

    Token SymbolTable::getToken(std::string & str) const
    {
        auto result = defines_.find(str);
        if (result != defines_.end())
            return result->second;
        result = variables_.find(str);
        if (result != variables_.end())
            return result->second;
        return Token();
    }

}

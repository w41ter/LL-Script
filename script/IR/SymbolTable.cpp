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

    //void SymbolTable::bindValue(std::string & str, ir::Value * value)
    //{
    //    values_[str] = value;
    //}

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

    //ir::Value * SymbolTable::getValue(std::string & str)
    //{
    //    return values_[str];
    //}

    void SymbolTable::catchedName(std::string & str)
    {
        catched_.insert(str);
    }

}

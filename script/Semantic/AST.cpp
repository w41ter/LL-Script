#include "AST.h"

namespace script
{
    SymbolTable::SymbolTable(SymbolTable * parent) : parent_(parent)
    {
    }

    bool SymbolTable::insert(const std::string & name, SymbolType type, Token token)
    {
        if (table_.count(name) == 0)
        {
             table_.insert(std::pair<std::string, Symbol>(name, { token, type }));
            return true;
        }
        return false;
    }

    Symbol & SymbolTable::find(const std::string & name)
    {
        if (table_.count(name) == 0)
            return end();
        return table_[name];
    }

    Symbol & SymbolTable::findInTree(const std::string &name)
    {
        if (table_.count(name) == 0)
        {
            if (parent_ != nullptr)
                return parent_->findInTree(name);
            return end();
        }
        return table_[name];
    }

    Symbol & SymbolTable::end() const
    {
        static Symbol t = { Token(), SymbolType::ST_NONE };
        return t;
    }

    ASTProgram::ASTProgram(SymbolTable * table, 
        std::vector<ASTDefine*> defines, 
        std::vector<ASTFunction*> function)
        : table_(table)
        , defines_(std::move(defines))
        , function_(std::move(function))
    {}

    ASTProgram::~ASTProgram()
    {
        delete table_;
    }
    bool ASTProgram::accept(ASTVisitor * v)
    {
        return v->visit(this);
    }
}
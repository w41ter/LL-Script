#pragma once

#include "../Parser/lexer.h"
#include "../IR/Instruction.h"
#include <map>
#include <set>

namespace script
{
    struct Symbol
    {
        enum { None, Define, Let };
        unsigned type_;
        bool beCaught_;
        bool caught_;
        bool param_;
        Token tok_;
    };

    class SymbolTable
    {
    public:
        SymbolTable();
        ~SymbolTable();

        void bindParent(SymbolTable *parent);

        void insert(std::string &name, Symbol &symbol);
        unsigned find(std::string &name, Symbol &symbol);
        void setCatched(std::string &name);

        typedef std::map<std::string, Symbol>::iterator iterator;
        iterator begin() { return table_.begin(); }
        iterator end() { return table_.end(); }

        SymbolTable *getParent() { return parent_; }

    protected:
        SymbolTable * parent_;
        std::map<std::string, Symbol> table_;
    };
}
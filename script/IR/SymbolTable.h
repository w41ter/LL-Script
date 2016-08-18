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

        /*void insertDefines(std::string &str, Token &token);
        void insertVariables(std::string &str, Token &token);

        unsigned findName(std::string &str) const;

        Token getToken(std::string &str) const;*/
        //ir::Value *getValue(std::string &str);
        SymbolTable *getParent() { return parent_; }
        //void catchedName(std::string &str);

    protected:
        SymbolTable * parent_;
        std::set<std::string> catched_;
        std::map<std::string, Symbol> table_;
        std::map<std::string, Token> defines_;
        std::map<std::string, Token> variables_;
        //std::map<std::string, ir::Value*> values_;
    };
}
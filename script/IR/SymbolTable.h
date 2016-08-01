#pragma once

#include "../Parser/lexer.h"
#include <map>

namespace script
{
    class SymbolTable
    {
    public:
        enum { None, Define, Let };

        SymbolTable();
        ~SymbolTable();

        void bindParent(const SymbolTable *parent);
        void insertDefines(std::string &str, Token &token);
        void insertVariables(std::string &str, Token &token);
        unsigned findName(std::string &str) const;
        Token getToken(std::string &str) const;
    protected:
        const SymbolTable * parent_;
        std::map<std::string, Token> defines_;
        std::map<std::string, Token> variables_;
    };
}
#pragma once

#include "CFG.h"

#include "../Parser/lexer.h"
#include <map>

namespace script
{
    class SymbolTable;

    class IRCode : public CFG
    {
        friend class DumpIR;
    public:
        IRCode();
        ~IRCode();

        SymbolTable *getTable();

    protected:
        SymbolTable *table_;
    };

    class IRFunction : public IRCode
    {
        friend class DumpIR;
    public:
        IRFunction(std::string name);

        std::string &getName();
        void setParams(std::vector<std::pair<std::string, Token>> &params);
        std::vector<std::string> &getParams();

    protected:
        std::string name_;
        std::vector<std::pair<std::string, Token>> params_;
    };

    class IRModule : public IRCode
    {
        friend class DumpIR;
    public:

        ~IRModule();

        IRFunction *createFunction(std::string &name);
        IRFunction *getFunction(std::string &name);

    private:
        void destory();

    protected:
        std::map<std::string, IRFunction*> functions_;
    };
}
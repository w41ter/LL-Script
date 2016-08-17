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
        
        typedef std::vector<std::pair<std::string, Token>>::iterator param_iterator;
        param_iterator param_begin() { return params_.begin(); }
        param_iterator param_end() { return params_.end(); }

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

        typedef std::map<std::string, IRFunction*>::iterator func_iterator;
        func_iterator begin() { return functions_.begin(); }
        func_iterator end() { return functions_.end(); }

    private:
        void destory();

    protected:
        std::map<std::string, IRFunction*> functions_;
    };
}
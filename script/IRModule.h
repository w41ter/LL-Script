#pragma once

#include "CFG.h"

#include "lexer.h"
#include <map>
#include <set>

namespace script
{
    class SymbolTable;

    class IRCode : public CFG
    {
        friend class DumpIR;
    public:
        IRCode();
        ~IRCode();
    };

    class IRFunction : public IRCode
    {
        friend class DumpIR;
    public:
        IRFunction(std::string name);

        std::string &getFunctionName();
        void setParams(std::vector<std::string> &&params);
        
        typedef std::vector<std::string>::iterator param_iterator;
        param_iterator param_begin() { return params_.begin(); }
        param_iterator param_end() { return params_.end(); }

    protected:
        std::string name_;
        std::vector<std::string> params_;
    };

    class IRModule
    {
        friend class DumpIR;
    public:
        ~IRModule();

        IRFunction *createFunction(const std::string &name);
        IRFunction *getFunctionByName(const std::string &name);

        typedef std::map<std::string, IRFunction*>::iterator iterator;
		iterator begin() { return functions_.begin(); }
		iterator end() { return functions_.end(); }

    protected:
        std::map<std::string, IRFunction*> functions_;
    };
}
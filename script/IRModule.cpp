#include "IRModule.h"

#include "IRContext.h"
#include "SymbolTable.h"

namespace script
{
    IRCode::IRCode() : table_(new SymbolTable())
    {

    }

    IRCode::~IRCode()
    {
        if (table_ != nullptr)
            delete table_;
    }

    IRFunction::IRFunction(std::string name)
        : name_(name)
    {
        
    }

    std::string & IRFunction::getName()
    {
        return name_;
    }

    void IRFunction::setParams(std::vector<std::pair<std::string, Token>> &params)
    {
        params_ = params;
    }

    SymbolTable *IRCode::getTable()
    {
        return table_;
    }

    IRModule::~IRModule()
    {
        for (auto &i : functions_)
            delete i.second;
    }

    IRFunction *IRModule::createFunction(std::string &name)
    {
        auto *function = new IRFunction(name);
        functions_.insert(std::pair<std::string, IRFunction*>(name, function));
        return function;
    }
}

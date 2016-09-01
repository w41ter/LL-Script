#include "IRModule.h"

#include "IRContext.h"
namespace script
{
    IRCode::IRCode()
    {

    }

    IRCode::~IRCode()
    {
    }

    IRFunction::IRFunction(std::string name)
        : name_(name)
    {
        
    }

    std::string & IRFunction::getFunctionName()
    {
        return name_;
    }

    void IRFunction::setParams(std::vector<std::string> &&params)
    {
        params_ = std::move(params);
    }

    IRModule::~IRModule()
    {
        for (auto &i : functions_)
            delete i.second;
    }

    IRFunction *IRModule::createFunction(const std::string &name)
    {
        auto *function = new IRFunction(name);
        functions_.insert({ name, function });
        return function;
    }
}

#include "IRGenerator.h"
#include "IRModule.h"

namespace script
{
    IRCode::IRCode()
    {
        context_ = new IRContext();
    }
    
    QuadContext * IRCode::getContext()
    {
        return context_;
    }

    CFG * IRCode::getCFG()
    {
        return cfg_.get();
    }

    bool IRCode::isCFGForm() const
    {
        return cfg_.get() != nullptr;
    }

    void IRCode::translateToCFG()
    {
        cfg_ = std::move(CFG::buildCFG(&context_));
    }

    IRFunction::IRFunction(std::string name)
        : name_(std::move(name)), begin_(nullptr), end_(nullptr)
    {
    }

    void IRFunction::set(Label * begin, Label * end)
    {
        begin_ = begin; end_ = end;
    }

    std::string & IRFunction::getName()
    {
        return name_;
    }

    void IRFunction::setParams(std::vector<std::string> params)
    {
        params_ = std::move(params);
    }

    std::vector<std::string>& IRFunction::getParams()
    {
        return params_;
    }

    IRModule::~IRModule()
    {
        destory();
    }

    IRFunction * IRModule::createFunction(std::string name)
    {
        return functions_[name] = new IRFunction(name);
    }

    IRFunction * IRModule::getFunction(std::string & name)
    {
        return functions_[name];
    }

    void IRModule::translateToCFG()
    {
        for (auto &i : functions_)
            i.second->translateToCFG();
        IRCode::translateToCFG();
    }

    void IRModule::destory()
    {
        for (auto &i : functions_)
        {
            delete i.second;
            i.second = nullptr;
        }
    }

}
#ifndef __IR_GENERATOR_H__
#define __IR_GENERATOR_H__

#include <list>
#include <map>
#include <memory>

#include "QuadGenerator.h"
#include "CFG.h"

namespace script
{
    class IRCode
    {
        friend class DumpQuad;
    public:
        QuadContext *getContext() { return &context_; }

        virtual void translateToCFG()
        {
            cfg_ = std::move(CFG::buildCFG(&context_));
        }

        CFG *getCFG() { return cfg_.get(); }
    protected:
        std::unique_ptr<CFG> cfg_;
        QuadContext context_;
    };

    class IRFunction : public IRCode
    {
        friend class DumpQuad;
    public:
        IRFunction(std::string name)
            : name_(std::move(name)), begin_(nullptr), end_(nullptr)
        {}

        void set(Label *begin, Label *end)
        {
            begin_ = begin; end_ = end;
        }

        std::string &getName() { return name_; }

    private:
        std::string name_;
        Label *begin_;
        Label *end_;
    };

    class IRModule : public IRCode
    {
        friend class DumpQuad;
        friend class DumpCFG;
    public:
        ~IRModule() { destory(); }

        IRFunction *createFunction(std::string name)
        {
            return functions_[name] = new IRFunction(name);
        }

        IRFunction *getFunction(std::string &name)
        {
            return functions_[name];
        }

        virtual void translateToCFG() override
        {
            for (auto &i : functions_)
                i.second->translateToCFG();
            IRCode::translateToCFG();
        }
    private:
        void destory()
        {
            for (auto &i : functions_)
            {
                delete i.second;
                i.second = nullptr;
            }
        }

    private:
        std::map<std::string, IRFunction*> functions_;
    };
}

#endif // !__IR_GENERATOR_H__


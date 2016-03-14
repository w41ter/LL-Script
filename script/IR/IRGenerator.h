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
        friend class CodeGenerator;
    public:
        QuadContext *getContext();
        CFG *getCFG();
        bool isCFGForm() const;
        virtual void translateToCFG();

    protected:
        std::unique_ptr<CFG> cfg_;
        QuadContext context_;
    };

    class IRFunction : public IRCode
    {
        friend class DumpQuad;
        friend class CodeGenerator;
    public:
        IRFunction(std::string name);

        void set(Label *begin, Label *end);
        std::string &getName();
        void setParams(std::vector<std::string> params);
        std::vector<std::string> &getParams();

    private:
        std::string name_;
        Label *begin_;
        Label *end_;
        std::vector<std::string> params_;
    };

    class IRModule : public IRCode
    {
        friend class DumpQuad;
        friend class DumpCFG;
        friend class CodeGenerator;
    public:

        ~IRModule();

        IRFunction *createFunction(std::string name);
        IRFunction *getFunction(std::string &name);
        virtual void translateToCFG() override;

    private:
        void destory();

    private:
        std::map<std::string, IRFunction*> functions_;
    };
}

#endif // !__IR_GENERATOR_H__


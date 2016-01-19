#ifndef __DUMP_CFG_H__
#define __DUMP_CFG_H__

#include "CFG.h"
#include "IRGenerator.h"
#include "dumpQuad.h"

namespace script
{
    class DumpCFG : public DumpQuad
    {
    public:
        DumpCFG(std::fstream &file) : DumpQuad(file) {}

        virtual bool visit(Label *v) override;

        virtual void dump(IRModule &module) override;

    protected:
        virtual void dumpFunction(IRFunction *func) override;
        virtual void dumpCode(IRCode *code) override;

    private:
        void dumpBasicBlock(BasicBlock *block);
        void dumpName(unsigned id);

        std::string module_;
        std::map<Quad*, Quad*> *labelTarget_;
        std::map<Quad*, BasicBlock*> head_;
    };
}

#endif // !__DUMP_CFG_H__

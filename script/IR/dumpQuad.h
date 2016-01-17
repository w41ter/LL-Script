#ifndef __DUMP_QUAD_H__
#define __DUMP_QUAD_H__

#include <fstream>
#include "Quad.h"

namespace script
{
    class DumpQuad : public QuadVisitor
    {
    public:
        DumpQuad(std::fstream &file) : file_(file) {}

        virtual ~DumpQuad() = default;
        virtual bool visit(Constant *v) override;
        virtual bool visit(Temp *v) override;
        virtual bool visit(Identifier *v) override;
        virtual bool visit(Array *v) override;
        virtual bool visit(ArrayIndex *v) override;

        virtual bool visit(If *v) override;
        virtual bool visit(Call *v) override;
        virtual bool visit(Goto *v) override;
        virtual bool visit(Copy *v) override;
        virtual bool visit(Load *v) override;
        virtual bool visit(Store *v) override;
        virtual bool visit(Label *v) override;
        virtual bool visit(Param *v) override;
        virtual bool visit(Invoke *v) override;
        virtual bool visit(Return *v) override;
        virtual bool visit(IfFalse *v) override;
        virtual bool visit(Function *v) override;
        virtual bool visit(Operation *v) override;
        virtual bool visit(AssignArray *v) override;
        virtual bool visit(ArrayAssign *v) override;

        void dump(std::list<Quad*> &codes);
    private:
        std::fstream &file_;
    };
}

#endif // !__DUMP_QUAD_H__

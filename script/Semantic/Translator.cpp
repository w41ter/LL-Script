#include "Translator.h"

namespace script
{
    bool Translator::visit(ASTContinueStatement & v)
    {
        quads_.push_back(MallocQuad<Goto>(jmpLabels_.top()));
        return false;
    }

    bool Translator::visit(ASTBreakStatement & v)
    {
        quads_.push_back(MallocQuad<Goto>(jmpLabels_.top()));
        return false;
    }

    bool Translator::visit(ASTReturnStatement & v)
    {
        if (v.expr_ != nullptr)
            v.expr_->accept(*this);
        else
            var_ = MallocMemory<Temp>();
        quads_.push_back(MallocQuad<Return>(var_));
        return false;
    }

    bool Translator::visit(ASTBlock & v)
    {
        auto *table = table_;
        table_ = v.table_;
        for (auto i : v.statements_)
            i->accept(*this);
        table_ = table;
        return false;
    }

    bool Translator::visit(ASTFunction & v)
    {
        return false;
    }

    bool Translator::visit(ASTProgram & v)
    {

        return false;
    }
}


#include "ASTContext.h"

namespace script
{
    ASTContext::ASTContext()
    {
    }


    ASTContext::~ASTContext()
    {
        for (auto i : astree_)
        {
            delete i;
        }
    }

    void ASTContext::setProgram(ASTProgram * program)
    {
        program_ = program;
    }
}
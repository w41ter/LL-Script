#include "dumpAST.h"

namespace script
{
    bool DumpAST::visit(ASTIdentifier & v)
    {
        file_ << v.name_;
        return false;
    }

    bool DumpAST::visit(ASTNull & v)
    {
        file_ << "nil";
        return false;
    }

}

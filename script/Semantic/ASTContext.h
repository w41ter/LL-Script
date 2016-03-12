#ifndef __AST_CONTEXT_H__
#define __AST_CONTEXT_H__

#include <list>

#include "AST.H"

namespace script
{
    class ASTContext
    {
    public:
        ASTContext();
        ~ASTContext();

        void setProgram(ASTProgram *program);

        template<typename Type, typename ...Args>
        Type *allocate(Args ...args) {
            Type *type = new Type(args...);
            astree_.push_back(type);
            return type;
        }

    private:
        ASTProgram *program_;
        std::list<ASTree*> astree_;
    };
}

#endif // !__AST_CONTEXT_H__



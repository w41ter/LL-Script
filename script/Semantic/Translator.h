#ifndef __TRANSLATOR_H__
#define __TRANSLATOR_H__

#include <stack>
#include <vector>
#include <memory>

#include "AST.h"

#include "../IR/IR.h"

namespace script
{
    class Translator : public ASTVisitor
    {
    public:
        Translator() : module_(std::make_unique<IR>()) {}

        virtual ~Translator() {};
        virtual bool visit(ASTExpressionList *v) override;
        virtual bool visit(ASTIdentifier *v) override;
        virtual bool visit(ASTNull *v) override;
        virtual bool visit(ASTConstant *v) override;
        virtual bool visit(ASTArray *v) override;
        virtual bool visit(ASTCall *v) override;
        virtual bool visit(ASTArrayIndex *v) override;
        virtual bool visit(ASTSingleExpression *v) override;
        virtual bool visit(ASTBinaryExpression *v) override;
        virtual bool visit(ASTRelationalExpression *v) override;
        virtual bool visit(ASTAndExpression *v) override;
        virtual bool visit(ASTOrExpression *v) override;
        virtual bool visit(ASTAssignExpression *v) override;
        virtual bool visit(ASTVarDeclStatement *v) override;
        virtual bool visit(ASTContinueStatement *v) override;
        virtual bool visit(ASTBreakStatement *v) override;
        virtual bool visit(ASTReturnStatement *v) override;
        virtual bool visit(ASTWhileStatement *v) override;
        virtual bool visit(ASTIfStatement *v) override;
        virtual bool visit(ASTBlock *v) override;
        virtual bool visit(ASTFunction *v) override;
        virtual bool visit(ASTProgram *v) override;

    private:
        std::unique_ptr<IR> module_;

        Quad *result_;
    };
}

#endif // !__TRANSLATOR_H__

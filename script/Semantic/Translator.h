#ifndef __TRANSLATOR_H__
#define __TRANSLATOR_H__

#include <stack>
#include <list>
#include <map>

#include "AST.h"
#include "../IR/Quad.h"
#include "../IR/IRGenerator.h"

namespace script
{
    class Translator : public ASTVisitor
    {
    public:
        Translator(IRModule &module) 
            : module_(module) 
        { 
            gen_ = module_.getContext(); 
        }

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
        virtual bool visit(ASTClosure *v) override;
        virtual bool visit(ASTStatement *v) override;
        virtual bool visit(ASTPrototype *v) override;
        virtual bool visit(ASTDefine *v) override;

        void translate(ASTProgram *program);

    private:
        Value *loadValue(Value *value);

    private:
        Value *result_;
        std::stack<Label*> breakLabels_;
        std::stack<Label*> continueLabels_;

        std::map<std::string, Label*> function_;
        std::map<std::string, Identifier*> *symbols_;
        QuadContext *gen_;

        IRModule &module_;
    };
}

#endif // !__TRANSLATOR_H__
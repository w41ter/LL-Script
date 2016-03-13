#ifndef __ANALYSIS_H__
#define __ANALYSIS_H__

#include "AST.h"

namespace script
{
    class ASTContext;

    class Analysis : public ASTVisitor
    {
        enum Type {
            TP_Integer,
            TP_Character,
            TP_Float,
            TP_Identifier,
            TP_Constant,
            TP_Array,
            TP_String,
            TP_Error
        };
    public:
        Analysis() : table_(nullptr), breakLevel_(0) {}
        virtual ~Analysis() {}

        void analysis(ASTContext &context);

    protected:
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
        virtual bool visit(ASTPrototype *v) override;
        virtual bool visit(ASTClosure *v) override;
        virtual bool visit(ASTDefine *v) override;
        virtual bool visit(ASTStatement *v) override;

    private:
        void except(unsigned tp);
        bool number(unsigned type);
        unsigned maxType(unsigned left, unsigned right);

    private:
        Type type_;

        unsigned breakLevel_;
        SymbolTable *table_;
    };
}

#endif // __ANALYSIS_H__
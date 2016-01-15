#ifndef __ANALYSIS_H__
#define __ANALYSIS_H__

#include <set>
#include <string>
#include <stdexcept>
#include <vector>

#include "AST.h"

namespace script
{
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
        void except(unsigned tp)
        {
            if (type_ == TP_Identifier 
                || type_ == TP_Constant
                || type_ == tp)
                return;

            if (number(tp))
            {
                if (number(type_))
                    return;
            }

            throw std::runtime_error("¿‡–Õ¥ÌŒÛ");
        }

        bool number(unsigned type)
        {
            return (type == TP_Integer ||
                type == TP_Character ||
                type == TP_Float);
        }
        
        unsigned maxType(unsigned left, unsigned right) 
        {
            if (left == TP_Constant || right == TP_Constant)
                return TP_Identifier;
            if (left == TP_Identifier || right == TP_Identifier)
                return TP_Identifier;
            if (left == TP_Float || right == TP_Float)
                return TP_Float;
            if (left == TP_Integer || right == TP_Integer)
                return TP_Integer;
            return TP_Character;
        }
    private:
        Type type_;

        unsigned breakLevel_;
        Symbols *table_;
    };
}

#endif // __ANALYSIS_H__
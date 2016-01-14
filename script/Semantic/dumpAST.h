#ifndef __DUMP_AST_H__
#define __DUMP_AST_H__

#include "AST.h"
#include <fstream>

namespace script
{
    class DumpAST : public Visitor
    {
    public:
        DumpAST(std::fstream &file) : file_(file) {}
        virtual ~DumpAST() = default;

        virtual bool visit(ASTExpressionList &v) override;
        virtual bool visit(ASTIdentifier &v) override;
        virtual bool visit(ASTNull &v) override;
        virtual bool visit(ASTConstant &v) override;
        virtual bool visit(ASTArray &v) override;
        virtual bool visit(ASTCall &v) override;
        virtual bool visit(ASTArrayIndex &v) override;
        virtual bool visit(ASTSingleExpression &v) override;
        virtual bool visit(ASTBinaryExpression &v) override;
        virtual bool visit(ASTRelationalExpression &v) override;
        virtual bool visit(ASTAndExpression &v) override;
        virtual bool visit(ASTOrExpression &v) override;
        virtual bool visit(ASTAssignExpression &v) override;
        virtual bool visit(ASTVarDeclStatement &v) override;
        virtual bool visit(ASTContinueStatement &v) override;
        virtual bool visit(ASTBreakStatement &v) override;
        virtual bool visit(ASTReturnStatement &v) override;
        virtual bool visit(ASTWhileStatement &v) override;
        virtual bool visit(ASTIfStatement &v) override;
        virtual bool visit(ASTBlock &v) override;
        virtual bool visit(ASTFunction &v) override;
        virtual bool visit(ASTProgram &v) override;
        virtual bool visit(ASTPrototype &v) override;
    private:
        void indent() { ++indent_; }
        void decdent() { --indent_; }
        void printIndent() 
        {
            for (int i = 0; i < indent_; ++i)
                file_ << '\t';
        }
    private:
        int indent_;
        std::fstream &file_;
    };
}

#endif // !__DUMP_AST_H__


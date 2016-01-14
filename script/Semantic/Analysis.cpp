#include "Analysis.h"

namespace script
{
    bool Analysis::visit(ASTExpressionList & v)
    {
        for (auto i : v.exprs_)
            i->accept(*this);
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTIdentifier & v)
    {
        if (table_->find(v.name_))
            type_ = TP_Identifier;
        else
            throw std::runtime_error("undefined identifier" + v.name_);
        return false;
    }

    bool Analysis::visit(ASTNull & v)
    {
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTConstant & v)
    {
        switch (v.type_)
        {
        case ASTConstant::T_Charactor: type_ = TP_Character; break;
        case ASTConstant::T_Float: type_ = TP_Float; break;
        case ASTConstant::T_Integer: type_ = TP_Integer; break;
        case ASTConstant::T_String: type_ = TP_String; break;
        default:
            type_ = TP_Error;
        }
        return false;
    }
    bool Analysis::visit(ASTArray & v)
    {
        /*for (auto i : v.array_)
            i->accept(*this);*/
        v.array_->accept(*this);
        type_ = TP_Array;
        return false;
    }

    bool Analysis::visit(ASTCall & v)
    {
        v.function_->accept(*this);
        except(TP_Identifier);
        v.arguments_->accept(*this);
        type_ = TP_Identifier;
        return false;
    }

    bool Analysis::visit(ASTArrayIndex & v)
    {
        v.array_->accept(*this);
        except(TP_Array);
        v.index_->accept(*this);
        except(TP_Integer);
        type_ = TP_Identifier;
        return false;
    }

    bool Analysis::visit(ASTSingleExpression & v)
    {
        v.expr_->accept(*this);
        except(TP_Integer);
        type_ = TP_Integer;
        return false;
    }

    bool Analysis::visit(ASTBinaryExpression & v)
    {
        v.left_->accept(*this);
        except(TP_Integer);
        v.right_->accept(*this);
        except(TP_Integer);
        type_ = TP_Integer;
        return false;
    }
    bool Analysis::visit(ASTRelationalExpression & v)
    {
        v.left_->accept(*this);
        except(TP_Integer);
        v.right_->accept(*this);
        except(TP_Integer);
        type_ = TP_Integer;
        return false;
    }

    bool Analysis::visit(ASTAndExpression & v)
    {
        for (auto i : v.relations_)
        {
            i->accept(*this);
            except(TP_Integer);
        }
        type_ = TP_Integer;
        return false;
    }

    bool Analysis::visit(ASTOrExpression & v)
    {
        for (auto i : v.relations_)
        {
            i->accept(*this);
            except(TP_Integer);
        }
        type_ = TP_Integer;
        return false;
    }

    bool Analysis::visit(ASTAssignExpression & v)
    {
        v.left_->accept(*this);
        except(TP_Identifier);
        v.right_->accept(*this);
        // type_ = v.right_.type_;
        return false;
    }

    bool Analysis::visit(ASTVarDeclStatement & v)
    {
        table_->insert(v.name_);
        v.expr_->accept(*this);
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTContinueStatement & v)
    {
        if (breakLevel_ <= 0)
            throw std::runtime_error("continue need while");
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTBreakStatement & v)
    {
        if (breakLevel_ <= 0)
            throw std::runtime_error("break need while");
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTReturnStatement & v)
    {
        if (v.expr_ != nullptr)
            v.expr_->accept(*this);
        type_ = TP_Identifier;
        return false;
    }

    bool Analysis::visit(ASTWhileStatement & v)
    {
        v.condition_->accept(*this);
        except(TP_Integer);
        breakLevel_++;
        v.statement_->accept(*this);
        breakLevel_--;
        /// 这里有个问题，就是
        /// while (conditon)
        ///     break;
        return false;
    }

    bool Analysis::visit(ASTIfStatement & v)
    {
        v.condition_->accept(*this);
        except(TP_Integer);
        v.ifStatement_->accept(*this);
        if (v.elseStatement_ != nullptr)
            v.elseStatement_->accept(*this);
        return false;
    }

    bool Analysis::visit(ASTBlock & v)
    {
        SymbolTable *tables = table_;
        table_ = v.table_;
        for (auto i : v.statements_)
            i->accept(*this);
        table_ = tables;
        return false;
    }

    bool Analysis::visit(ASTFunction & v)
    {
        for (auto &i : v.params_)
            v.block_->table_->insert(i);
        v.accept(*this);
        return false;
    }

    bool Analysis::visit(ASTProgram & v)
    {
        table_ = v.table_;
        for (auto &i : v.functions_)
            table_->insert(i->name_);
        for (auto &i : v.functions_)
            i->accept(*this);
        return false;
    }

    bool Analysis::visit(ASTPrototype & v)
    {
        return false;
    }
}
#include <iostream>
#include <set>
#include <string>
#include <stdexcept>
#include <vector>

#include "ASTContext.h"
#include "Analysis.h"

namespace script
{
    void Analysis::analysis(ASTContext & context)
    {
        context.program_->accept(this);
    }

    bool Analysis::visit(ASTExpressionList *v)
    {
        for (auto &i : v->exprs_)
            i->accept(this);
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTIdentifier *v)
    {
        auto end = table_->find(v->name_);
        if (end.type_ == SymbolType::ST_Variable)
            type_ = TP_Identifier;
        else if (end.type_ == SymbolType::ST_Constant)
            type_ = TP_Constant;
        else
            throw std::runtime_error("undefined identifier " + v->name_);
        return false;
    }

    bool Analysis::visit(ASTNull *v)
    {
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTConstant *v)
    {
        switch (v->type_)
        {
        case ASTConstant::T_Character: type_ = TP_Character; break;
        case ASTConstant::T_Float: type_ = TP_Float; break;
        case ASTConstant::T_Integer: type_ = TP_Integer; break;
        case ASTConstant::T_String: type_ = TP_String; break;
        default:
            type_ = TP_Error;
        }
        return false;
    }
    bool Analysis::visit(ASTArray *v)
    {
        /*for (auto i : v->array_)
            i->accept(this);*/
        v->array_->accept(this);
        type_ = TP_Array;
        return false;
    }

    bool Analysis::visit(ASTCall *v)
    {
        //std::cout << "\t\tanalysis call..." << std::endl;
        v->function_->accept(this);
        except(TP_Identifier);
        v->arguments_->accept(this);
        type_ = TP_Identifier;
        return false;
    }

    bool Analysis::visit(ASTArrayIndex *v)
    {
        v->array_->accept(this);
        except(TP_Array);
        v->index_->accept(this);
        except(TP_Integer);
        type_ = TP_Identifier;
        return false;
    }

    bool Analysis::visit(ASTSingleExpression *v)
    {
        v->expr_->accept(this);
        except(TP_Integer);
        type_ = TP_Integer;
        return false;
    }

    bool Analysis::visit(ASTBinaryExpression *v)
    {
        v->left_->accept(this);
        except(TP_Integer);
        v->right_->accept(this);
        except(TP_Integer);
        type_ = TP_Integer;
        return false;
    }
    bool Analysis::visit(ASTRelationalExpression *v)
    {
        v->left_->accept(this);
        except(TP_Integer);
        v->right_->accept(this);
        except(TP_Integer);
        type_ = TP_Integer;
        return false;
    }

    bool Analysis::visit(ASTAndExpression *v)
    {
        for (auto &i : v->relations_)
        {
            i->accept(this);
            except(TP_Integer);
        }
        type_ = TP_Integer;
        return false;
    }

    bool Analysis::visit(ASTOrExpression *v)
    {
        for (auto &i : v->relations_)
        {
            i->accept(this);
            except(TP_Integer);
        }
        type_ = TP_Integer;
        return false;
    }

    bool Analysis::visit(ASTAssignExpression *v)
    {
        //std::cout << "\t\tanalysis assignment..." << std::endl;
        v->left_->accept(this);
        if (type_ == TP_Constant)
            throw std::runtime_error("cann't assign to constant!");
        except(TP_Identifier);
        v->right_->accept(this);
        // type_ = v->right_.type_;
        return false;
    }

    bool Analysis::visit(ASTVarDeclStatement *v)
    {
        //std::cout << "\t\tanalysis let... " << v->name_ << std::endl;
        v->expr_->accept(this);
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTContinueStatement *v)
    {
        if (breakLevel_ <= 0)
            throw std::runtime_error("continue need while");
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTBreakStatement *v)
    {
        if (breakLevel_ <= 0)
            throw std::runtime_error("break need while");
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTReturnStatement *v)
    {
        if (v->expr_ != nullptr)
            v->expr_->accept(this);
        type_ = TP_Identifier;
        return false;
    }

    bool Analysis::visit(ASTWhileStatement *v)
    {
        v->condition_->accept(this);
        except(TP_Integer);
        breakLevel_++;
        v->statement_->accept(this);
        breakLevel_--;
        /// 这里有个问题，就是
        /// while (conditon)
        ///     break;
        return false;
    }

    bool Analysis::visit(ASTIfStatement *v)
    {
        v->condition_->accept(this);
        except(TP_Integer);
        v->ifStatement_->accept(this);
        if (v->elseStatement_ != nullptr)
            v->elseStatement_->accept(this);
        return false;
    }

    bool Analysis::visit(ASTBlock *v)
    {
        for (auto &i : v->statements_)
            i->accept(this);
        return false;
    }

    bool Analysis::visit(ASTFunction *v)
    {
        //v->prototype_->accept(this); 
        //std::cout << "\tbegin analysis function: " 
        //    << v->prototype_->name_ << std::endl;
        SymbolTable *table = table_;
        table_ = v->table_;
        v->block_->accept(this);
        table_ = table;
        return false;
    }

    bool Analysis::visit(ASTProgram *v)
    {
        //std::cout << "begin analysis:" << std::endl;
        table_ = v->table_;
        //for (auto &i : v->function_)
        //    table_->insert(i->name_);
        for (auto &i : v->function_)
            i->accept(this);
        for (auto &i : v->defines_)
            i->accept(this);
        //std::cout << "end analysis" << std::endl;
        return false;
    }

    bool Analysis::visit(ASTPrototype *v)
    {
        return false;
    }

    bool Analysis::visit(ASTClosure * v)
    {
        //std::cout << "\t\tanalysis closure..." << v->name_ << std::endl;
        type_ = TP_Identifier;
        return false;
    }

    bool Analysis::visit(ASTDefine * v)
    {
        v->expr_->accept(this);
        return false;
    }

    bool Analysis::visit(ASTStatement * v)
    {
        v->tree_->accept(this);
        return false;
    }

    void Analysis::except(unsigned tp) {
        if (type_ == TP_Identifier
            || type_ == TP_Constant
            || type_ == tp)
            return;

        if (number(tp))
        {
            if (number(type_))
                return;
        }

        throw std::runtime_error("类型错误");
    }

    bool Analysis::number(unsigned type)
    {
        return (type == TP_Integer ||
            type == TP_Character ||
            type == TP_Float);
    }

    unsigned Analysis::maxType(unsigned left, unsigned right)
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

}
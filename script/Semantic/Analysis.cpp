#include <iostream>
#include <set>
#include <string>
#include <stdexcept>
#include <vector>

#include "ASTContext.h"
#include "Analysis.h"

using std::cout;
using std::endl;

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
        if (end == table_->end()) {
            end = table_->getParent()->find(v->name_);
        }
        if (end.type_ == SymbolType::ST_Variable)
            type_ = TP_Identifier;
        else if (end.type_ == SymbolType::ST_Constant)
            type_ = TP_Constant;
        else
            error(std::string("undefined identifier " + v->name_), end.token_);
        return false;
    }

    bool Analysis::visit(ASTNull *v)
    {
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTConstant *v)
    {
        token_ = &v->token_;
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
        v->array_->accept(this);
        type_ = TP_Array;
        return false;
    }

    bool Analysis::visit(ASTCall *v)
    {
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
        v->left_->accept(this);
        if (type_ == TP_Constant)
            error(std::string("cann't assign to constant!"), *token_);
        except(TP_Identifier);
        v->right_->accept(this);
        // type_ = v->right_.type_;
        return false;
    }

    bool Analysis::visit(ASTVarDeclStatement *v)
    {
        v->expr_->accept(this);
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTContinueStatement *v)
    {
        if (breakLevel_ <= 0)
            error(std::string("continue need while"), v->token_);
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTBreakStatement *v)
    {
        if (breakLevel_ <= 0)
            error(std::string("break need while"), v->token_);
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
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTIfStatement *v)
    {
        v->condition_->accept(this);
        except(TP_Integer);
        v->ifStatement_->accept(this);
        if (v->elseStatement_ != nullptr)
            v->elseStatement_->accept(this);
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTBlock *v)
    {
        for (auto &i : v->statements_)
            i->accept(this);
        type_ = TP_Error;
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
        type_ = TP_Error;
        return false;
    }

    bool Analysis::visit(ASTPrototype *v)
    {
        return false;
    }

    bool Analysis::visit(ASTClosure * v)
    {
        //std::cout << "\t\tanalysis closure..." << v->name_ << std::endl;
        if (v->params_.size() > v->total_)
            error(std::string("参数过多"));
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

    bool Analysis::visit(ASTProgram *v)
    {
        //std::cout << "begin analysis:" << std::endl;
        table_ = v->table_;
        //for (auto &i : v->function_)
        //    table_->insert(i->name_);
        for (auto &i : v->function_)
            i->accept(this);
        for (auto &i : v->statements_)
            i->accept(this);
        //std::cout << "end analysis" << std::endl;
        return false;
    }

    void Analysis::except(unsigned tp) {
        if (type_ == TP_Identifier || type_ == TP_Constant || type_ == tp)
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
        return (type == TP_Integer || type == TP_Character || type == TP_Float);
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

    void Analysis::error(std::string & msg, Token &token)
    {
        std::cout << token.coord_.fileName_
            << "(" << token.coord_.lineNum_
            << "," << token.coord_.linePos_ << "): "
            << msg << endl;
        throw std::runtime_error(msg);
    }

    void Analysis::error(std::string & msg)
    {
        std::cout << msg << endl;
        throw std::runtime_error(msg);
    }

}
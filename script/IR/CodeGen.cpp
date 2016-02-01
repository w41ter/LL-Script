#include <functional>

#include "RegisterAllocator.h"
#include "../Parser/lexer.h"
#include "CodeGen.h"

namespace script
{
    CodeGenerator::CodeGenerator(OpcodeContext & context)
        : context_(context)
    {
        context_.bindGetLabelTarget(std::bind(&CodeGenerator::backLabel, this, std::placeholders::_1));
        context_.bindGetFunctionTarget(std::bind(&CodeGenerator::backFunction, this, std::placeholders::_1));
    }

    void CodeGenerator::gen(IRModule & module)
    {
        for (auto &i : module.functions_)
        {
            int offset = genIRCode(*i.second);
            functions_.insert(std::pair<std::string, int>(i.first, offset));
        }

        genIRCode(module);
        context_.insertHalt();
    }

    int CodeGenerator::backLabel(Quad * label)
    {
        return labels_[label];
    }

    int CodeGenerator::backFunction(std::string & str)
    {
        return functions_[str];
    }

    // 
    // before gen code, connect bb with the first code.
    // 
    int CodeGenerator::genIRCode(IRCode & code)
    {
        String2Int frames;
        slotStack_ = &frames;

        blocks_.clear();
        for (auto &i : code.cfg_->blocks_)
            blocks_.insert(std::pair<Quad*, BasicBlock*>(i->begin(), i));
        return genBasicBlock(code.cfg_->start_);
    }

    // 
    // the way gen bb is liner.
    // 
    int CodeGenerator::genBasicBlock(BasicBlock * block)
    {
        // check block's code has generate.
        if (hasVisit_.find(block) != hasVisit_.end())
            return labels_[block->begin()];
        hasVisit_.insert(block);

        int index = context_.getNextPos();
        labels_.insert(std::pair<Quad*, int>(block->begin(), index));
        Quad *begin = block->begin(), *end = block->end();
        RegisterAllocator allocator(context_, block);
        allocator_ = &allocator;
        while (begin != nullptr)
        {
            begin->accept(this);
            if (begin == end)
                break;
            begin = begin->next_;
        }
        allocator_ = nullptr;
        return index;
    }

    bool CodeGenerator::visit(Constant * v)
    {
        return false;
    }

    bool CodeGenerator::visit(Temp * v)
    {
        return false;
    }

    bool CodeGenerator::visit(Identifier * v)
    {
        return false;
    }

    bool CodeGenerator::visit(Array * v)
    {
        return false;
    }

    bool CodeGenerator::visit(ArrayIndex * v)
    {
        return false;
    }

    bool CodeGenerator::visit(If * v)
    {
        v->condition_->accept(this);
        Register reg = allocator_->getReg(v->condition_);
        context_.insertIf(reg, v->label_);

        BasicBlock *next = targetBasicBlock(v->next_);
        genBasicBlock(next);
        next = targetBasicBlock(v->label_);
        genBasicBlock(next);
        return false;
    }

    bool CodeGenerator::visit(Call * v)
    {
        v->result_->accept(this);
        Register reg = allocator_->allocate(v->result_);
        context_.insertCall(v->name_, v->num_, reg);
        return false;
    }

    bool CodeGenerator::visit(Goto * v)
    {
        context_.insertGoto(v->label_);
        BasicBlock *next = targetBasicBlock(v->label_);
        genBasicBlock(next);
        return false;
    }

    bool CodeGenerator::visit(Copy * v)
    {
        v->sour_->accept(this);
        v->dest_->accept(this);
        Register reg = allocator_->allocate(v->dest_);
        if (v->sour_->kind() == Value::V_Constant)
        {
            Constant *c = (Constant*)v->sour_;
            switch (c->type_)
            {
            case Constant::T_Character:
                context_.insertMoveI(reg, c->c_);
                break;
            case Constant::T_Integer:
                context_.insertMoveI(reg, c->num_);
                break;
            case Constant::T_Float:
                context_.insertMoveF(reg, c->fnum_);
                break;
            case Constant::T_String:
                context_.insertMoveS(reg, context_.insertString(c->str_));
                break;
            }
        } 
        else
        {
            Register regS = allocator_->getReg(v->sour_);
            context_.insertMove(reg, regS);
        }

        return false;
    }

    bool CodeGenerator::visit(Load * v)
    {
        if (v->id_->kind() == Value::V_ArrayIndex)
        {
            ArrayIndex *ai = (ArrayIndex*)(v->id_);
            ai->index_->accept(this);
            ai->value_->accept(this);
            v->result_->accept(this);
            Register regID = allocator_->getReg(ai->index_),
                regIndex = allocator_->getReg(ai->value_),
                result = allocator_->allocate(v->result_);
            context_.insertLoadA(regID, regIndex, result);
        }
        else
        {
            Identifier *id = (Identifier*)v->id_;
            id->accept(this);
            v->result_->accept(this);
            Register result = allocator_->allocate(v->result_);
            context_.insertLoad(newLocalSlot(id->name_), result);
            // context_.insertLoad(context_.insertString(id->name_), result);
        }
        return false;
    }

    bool CodeGenerator::visit(Store * v)
    {
        if (v->id_->kind() == Value::V_ArrayIndex)
        {
            ArrayIndex *ai = (ArrayIndex*)(v->id_);
            ai->index_->accept(this);
            ai->value_->accept(this);
            v->result_->accept(this);
            Register regID = allocator_->getReg(ai->index_),
                regIndex = allocator_->getReg(ai->value_),
                regFrom = allocator_->getReg(v->result_);
            context_.insertStoreA(regID, regIndex, regFrom);
        }
        else
        {
            Identifier *id = (Identifier*)v->id_;
            id->accept(this);
            v->result_->accept(this);
            Register result = allocator_->getReg(v->result_);
            context_.insertStore(newLocalSlot(id->name_), result);
        }
        return false;
    }

    bool CodeGenerator::visit(Label * v)
    {
        return false;
    }

    bool CodeGenerator::visit(Param * v)
    {
        v->Value_->accept(this);
        Register reg = allocator_->getReg(v->Value_);
        context_.insertParam(reg);
        return false;
    }

    bool CodeGenerator::visit(Invoke * v)
    {
        v->name_->accept(this);
        v->result_->accept(this);
        Register reg = allocator_->getReg(v->name_);
        Register result = allocator_->allocate(v->result_);
        context_.insertInvoke(reg, v->num_, result);
        return false;
    }

    bool CodeGenerator::visit(Return * v)
    {
        v->arg_->accept(this);
        Register reg = allocator_->getReg(v->arg_);
        context_.insertReturn(reg);
        return false;
    }

    bool CodeGenerator::visit(IfFalse * v)
    {
        v->condition_->accept(this);
        Register reg = allocator_->getReg(v->condition_);
        context_.insertIfFalse(reg, v->label_);

        BasicBlock *next = targetBasicBlock(v->next_);
        genBasicBlock(next);
        next = targetBasicBlock(v->label_);
        genBasicBlock(next);
        return false;
    }

    bool CodeGenerator::visit(Operation * v)
    {
        Register regLeft = v->left_ == nullptr 
            ? Register(0) 
            : (v->left_->accept(this), allocator_->getReg(v->left_));
        Register regRight = allocator_->getReg(v->right_);
        Register result = allocator_->allocate(v->result_);
        
        v->right_->accept(this);
        v->result_->accept(this);

        if (v->left_ == nullptr)
            context_.insertSingleOP(v->op_, regRight, result);
        else
            context_.insertBinaryOP(v->op_, regLeft, regRight, result);
        return false;
    }
    
    bool CodeGenerator::visit(AssignArray * v)
    {
        return false;
    }

    bool CodeGenerator::visit(ArrayAssign * v)
    {
        return false;
    }

    BasicBlock * CodeGenerator::targetBasicBlock(Quad * label)
    {
        return blocks_[LabelTarget::instance().getTarget(label)];
    }

    int CodeGenerator::newLocalSlot(std::string & name)
    {
        auto res = slotStack_->find(name);
        int result = 0;
        if (slotStack_->end() == res)
        {
            auto i = slotStack_->insert(std::pair<std::string, int>(name, slotStack_->size()));
            result = i.first->second;
        }
        else 
        {
            result = res->second;
        }
        return result;
    }

}

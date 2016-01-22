#include <functional>

#include "RegisterAllocator.h"
#include "../Parser/lexer.h"
#include "CodeGen.h"

namespace script
{
    CodeGenerator::CodeGenerator(OpcodeContext & context)
        : context_(context)
    {
        context_.bind(std::bind(&CodeGenerator::backLabel, this, std::placeholders::_1));
        context_.bindFunction(std::bind(&CodeGenerator::backFunction, this, std::placeholders::_1));
    }

    void CodeGenerator::gen(IRModule & module)
    {
        for (auto &i : module.functions_)
        {
            int offset = genIRCode(*i.second);
            functions_.insert(std::pair<std::string, int>(i.first, offset));
        }
        genIRCode(module);
    }

    int CodeGenerator::backLabel(Quad * label)
    {
        return 0;
    }

    int CodeGenerator::backFunction(std::string & str)
    {
        return 0;
    }

    int CodeGenerator::genIRCode(IRCode & code)
    {
        blocks_.clear();
        for (auto &i : code.cfg_->blocks_)
            blocks_.insert(std::pair<Quad*, BasicBlock*>(i->begin(), i));
        return genBasicBlock(code.cfg_->start_);
    }

    int CodeGenerator::genBasicBlock(BasicBlock * block)
    {
        int index = context_->getNextPos();
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
        Register reg = allocator_->getReg(v->condition_);
        context_.insertIf(reg, v->label_);
        BasicBlock *next = blocks_[LabelTarget::instance().getTarget(v->next_)];
        genBasicBlock(next);
        next = cfg_->target(v->label_);
        genBasicBlock(next);
        return false;
    }

    bool CodeGenerator::visit(Call * v)
    {
        Register reg = allocator_->allocate(v->result_);
        context_.insertCall(v->name_, v->num_, reg);
        return false;
    }

    bool CodeGenerator::visit(Goto * v)
    {
        context_.insertGoto(v->label_);
        return false;
    }

    bool CodeGenerator::visit(Copy * v)
    {
        Register reg = allocator_->getReg(v->sour_);
        Register reg = allocator_->allocate(v->dest_);
        return false;
    }

    bool CodeGenerator::visit(Load * v)
    {
        if (v->id_->kind() == Value::V_ArrayIndex)
        {
            ArrayIndex *ai = (ArrayIndex*)(v->id_);
            Register regID = allocator_->getReg(ai->index_),
                regIndex = allocator_->getReg(ai->value_);
            Register result = allocator_->allocate(v->result_);
            context_.insertLoadA(regID, regIndex, result);
        }
        else
        {
            Register result = allocator_->allocate(v->result_);
            context_.insertLoad(index, result);
        }
        return false;
    }

    bool CodeGenerator::visit(Store * v)
    {
        if (v->id_->kind() == Value::V_ArrayIndex)
        {
            ArrayIndex *ai = (ArrayIndex*)(v->id_);
            Register regID = allocator_->getReg(ai->index_),
                regIndex = allocator_->getReg(ai->value_);
            Register result = allocator_->allocate(v->result_);
            context_.insertLoadA(regID, regIndex, result);
        }
        else
        {
            Register result = allocator_->allocate(v->result_);
            context_.insertLoad(index, result);
        }
        return false;
    }

    bool CodeGenerator::visit(Label * v)
    {
        return false;
    }

    bool CodeGenerator::visit(Param * v)
    {
        Register reg = allocator_->getReg(v->Value_);
        context_.insertParam(reg);
        return false;
    }

    bool CodeGenerator::visit(Invoke * v)
    {
        Register reg = allocator_->getReg(v->name_);
        Register result = allocator_->allocate(v->result_);
        context_.insertInvoke(reg, v->num_, result);
        return false;
    }

    bool CodeGenerator::visit(Return * v)
    {
        Register reg = allocator_->getReg(v->arg_);
        context_.insertReturn(reg);
        return false;
    }

    bool CodeGenerator::visit(IfFalse * v)
    {
        Register reg = allocator_->getReg(v->condition_);
        context_.insertIf(reg, v->label_);
        BasicBlock *next = blocks_[LabelTarget::instance().getTarget(v->next_)];
        genBasicBlock(next);
        next = cfg_->target(v->label_);
        genBasicBlock(next);
        return false;
    }

    bool CodeGenerator::visit(Operation * v)
    {
        Register regLift = v->left_ == nullptr ? 0 : allocator_->getReg(v->left_);
        Register regRight = allocator_->getReg(v->right_);
        Register result = allocator_->allocate(v->result_);

        switch (v->op_)
        {
        case TK_Plus:
            context_.insertBinaryOP(op, left, right);
        // TODO
        }
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

}

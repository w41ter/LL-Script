#ifndef __IR_H__
#define __IR_H__

#include <cassert>

#include <vector>

namespace script
{
    enum Opcode 
    {
        OK_FAdd, OK_FSub, OK_FMul, OK_FDiv, 
        OK_IAdd, OK_ISub, OK_IDiv, OK_IMul,
        OK_FToI, OK_IToF,
        OK_FGreat, OK_FGreatThan, OK_FLess, OK_FLessThan,
        OK_FEqual, OK_FNotEqual, OK_Load, OK_Store, 
        OK_If, OK_IfFalse, OK_Goto, OK_Call, OK_Return,
        OK_Phi
    // add, sub, mul, div, cmp, load, store, if, if false, goto, call, return, phi ...
    };

    // 一条IR指令
    struct Quad 
    {
        // 四元组
        Opcode op;
        Quad *src1; // 直接使用别的指令作为operand
        Quad *src2;
        // 不需要指定dest; 自身就代表了运算和值

        Quad *prev, *next; // 双向链
    };

    // 一个基本块
    class BasicBlock 
    {
        Quad *head; // 用一个线性链表来表示整个基本块的逻辑；末尾必须是控制流指令；中间不包括phi
        std::vector<Quad*> phis; // 记录该基本块开头所需的phi指令
        std::vector<BasicBlock*> predecessors;   // 记录该基本块所有前驱
    // ArrayList<BasicBlock> successors;
    // 后继基本块；该信息可以从该基本块末尾的控制流指令提取，不需要显式用字段记录

    // ??? flags;
    // 可选的额外信息，可以用来记录诸如按reverse post-order（RPO）遍历CFG的序号，是否是循环开头（loop header），等等
    };

    class IR 
    {
        BasicBlock *start; // 起始基本块

        std::vector<BasicBlock*> blocks;

    public:
        IR() {}


    };

    class IRModule
    {
    public:
        IRModule() {}


    private:
    };

    class IRBuiler
    {
    public:
        IRBuiler() {}
        
        Quad *createFToI(Quad *params);
        Quad *createIToF(Quad *params);
        Quad *createFAdd(Quad *lhs, Quad *rhs);
        Quad *createFSub(Quad *lhs, Quad *rhs);
        Quad *createFMul(Quad *lhs, Quad *rhs);
        Quad *createFDiv(Quad *lhs, Quad *rhs);
        Quad *createIAdd(Quad *lhs, Quad *rhs);
        Quad *createISub(Quad *lhs, Quad *rhs);
        Quad *createIMul(Quad *lhs, Quad *rhs);
        Quad *createIDiv(Quad *lhs, Quad *rhs);
        Quad *createGreat(Quad *lhs, Quad *rhs);
        Quad *createGreatThan(Quad *lhs, Quad *rhs);
        Quad *createLess(Quad *lhs, Quad *rhs);
        Quad *createLessThan(Quad *lhs, Quad *rhs);
        Quad *createEqual(Quad *lhs, Quad *rhs);
        Quad *createNotEqual(Quad *lhs, Quad *rhs);
        Quad *createLoad(Quad *lhs, Quad *rhs);
        Quad *createStore(Quad *lhs, Quad *rhs);
        Quad *createIf(Quad *lhs, Quad *rhs);

        BasicBlock *createBasicBlock();
        BasicBlock *getInsertBlock();
    private:
    };

    
}

#endif // !__IR_H__
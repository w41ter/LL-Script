#ifndef __IR_H__
#define __IR_H__

#include <cassert>

#include <vector>
#include <list>

#include "Quad.h"

namespace script
{
   
    // 一个基本块
    class BasicBlock 
    {
        Quad *head; // 用一个线性链表来表示整个基本块的逻辑；末尾必须是控制流指令；中间不包括phi
        std::list<Quad*> phis; // 记录该基本块开头所需的phi指令
        std::list<BasicBlock*> predecessors;   // 记录该基本块所有前驱
        std::list<BasicBlock*> successors;  // 后继基本块；该信息可以从该基本块末尾的控制流指令提取，不需要显式用字段记录

        // ??? flags;
        // 可选的额外信息，可以用来记录诸如按reverse post-order（RPO）遍历CFG的序号，是否是循环开头（loop header），等等
    };

    class IR 
    {
        BasicBlock *start; // 起始基本块

        std::list<BasicBlock*> blocks;

    public:
        IR() {}


    };
}

#endif // !__IR_H__
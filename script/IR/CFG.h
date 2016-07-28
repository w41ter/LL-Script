#pragma once

#include <vector>
#include <list>
#include <memory>
#include <string>

namespace script
{
    class Instruction;

    // 一个基本块
    class BasicBlock
    {
    public:
        BasicBlock(int id, std::string name) 
            : ID_(id), head_(nullptr), end_(nullptr), name_(name)
        {}

        void addPrecursor(BasicBlock *block);
        void addSuccessor(BasicBlock *block);
        size_t numOfPrecursors() const { return precursors_.size(); }
        size_t numOfSuccessors() const { return successors_.size(); }
        //void set(Quad *head, Quad *end) { head_ = head; end_ = end; }
        //Quad *begin() { return head_; }
        //Quad *end() { return end_; }
        unsigned getID() const { return ID_; }

        void unique();

    protected:
        unsigned ID_;
        std::string name_;
        //Quad *head_, *end_;
        //std::list<Quad*> phis_; 
        Instruction *head_, *end_;
        std::list<BasicBlock*> precursors_;   // 记录该基本块所有前驱
        std::list<BasicBlock*> successors_;  // 后继基本块
    };

    class CFG
    {
    public:
        CFG();
        ~CFG();

        BasicBlock *createBasicBlock(std::string name);
        void setEntry(BasicBlock *entry);
        void setEnd(BasicBlock *end);
    protected:
        unsigned numBlockIDs_;
        BasicBlock *start_; // 起始基本块
        BasicBlock *end_;
        std::list<BasicBlock*> blocks_;
    };
}



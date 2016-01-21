#ifndef __CFG_H__
#define __CFG_H__

#include <cassert>
#include <map>
#include <vector>
#include <list>
#include <memory>

#include "Quad.h"

namespace script
{
    class QuadContext;

    // 一个基本块
    class BasicBlock
    {
        friend class DumpCFG;
        friend class CodeGenerator;
    public:
        BasicBlock(int id) : ID_(id), head_(nullptr), end_(nullptr) {}
        void addPrecursor(BasicBlock *block);
        void addSuccessor(BasicBlock *block);
        size_t numOfPrecursors() const { return precursors_.size(); }
        size_t numOfSuccessors() const { return successors_.size(); }
        void set(Quad *head, Quad *end) { head_ = head; end_ = end; }
        Quad *begin() { return head_; }
        Quad *end() { return end_; }
        unsigned getID() const { return ID_; }

        void unique();
    private:
        unsigned ID_;
        Quad *head_, *end_;
        std::list<Quad*> phis_; 
        std::list<BasicBlock*> precursors_;   // 记录该基本块所有前驱
        std::list<BasicBlock*> successors_;  // 后继基本块
    };

    class CFG
    {
        friend class DumpCFG;
        friend class CodeGenerator;
    public:
        CFG();
        ~CFG();

        static std::unique_ptr<CFG> buildCFG(QuadContext *context);
        static void buildTarget(QuadContext *context, CFG *cfg);
        static void removeDeadBlock(CFG *cfg);

        BasicBlock *createBlock();

    private:
        unsigned numBlockIDs_;
        BasicBlock *start_; // 起始基本块
        BasicBlock *end_;
        std::list<BasicBlock*> blocks_;
        std::map<Quad*, Quad*> labelTarget_;
    };
}

#endif // !__CFG_H__


#pragma once

#include <vector>
#include <list>
#include <memory>
#include <string>
#include <map>
#include <set>

namespace script
{
namespace ir
{
    class Phi;
    class Value;
    class Instruction;
}

    class IRContext;
    
    // 一个基本块
    class BasicBlock
    {
        friend class ir::Instruction;
    public:
        BasicBlock(int id, std::string name) 
            : ID_(id), head_(nullptr), end_(nullptr), name_(name)
        {}

        void addPrecursor(BasicBlock *block);
        void addSuccessor(BasicBlock *block);
        size_t numOfPrecursors() const { return precursors_.size(); }
        size_t numOfSuccessors() const { return successors_.size(); }
        BasicBlock *precursor(int idx);
        BasicBlock *successor(int idx);
        std::vector<BasicBlock*> &precursors() { return precursors_; }
        std::vector<BasicBlock*> &successors() { return successors_; }

        ir::Instruction *begin() { return head_; }
        ir::Instruction *end() { return end_; }

        const ir::Instruction *begin() const { return head_; }
        const ir::Instruction *end() const { return end_; }

        unsigned getID() const { return ID_; }
        const std::string &getName() const { return name_; }

        void push(ir::Instruction *instr);
        void unique();

    protected:
        unsigned ID_;
        std::string name_;
        ir::Instruction *head_, *end_;
        std::vector<BasicBlock*> precursors_;   // 记录该基本块所有前驱
        std::vector<BasicBlock*> successors_;  // 后继基本块
    };

    class CFG
    {
        friend class DumpIR;
    public:
        CFG();
        ~CFG();

        BasicBlock *createBasicBlock(std::string name);
        void setEntry(BasicBlock *entry);
        void setEnd(BasicBlock *end);
        BasicBlock *getEntryBlock();
        std::list<BasicBlock*> &blocks();

        IRContext *getContext();

        // SSA form construction.
        void sealOthersBlock();
        void sealBlock(BasicBlock *block);
        void saveVariableDef(std::string name, BasicBlock *block, ir::Value *value);
        ir::Value *readVariableDef(std::string name, BasicBlock *block);
        ir::Value *readVariableRecurisive(std::string name, BasicBlock *block);
        ir::Value *addPhiOperands(std::string name, ir::Phi *phi);
        ir::Value *tryRemoveTrivialPhi(ir::Phi *phi);
    protected:
        unsigned numBlockIDs_;
        BasicBlock *start_; // 起始基本块
        BasicBlock *end_;
        std::list<BasicBlock*> blocks_;

        IRContext *context_;

        typedef std::map<std::string, std::map<BasicBlock*, ir::Value*>> Definition;
        Definition currentDef_;
        std::set<BasicBlock*> sealedBlock_;
        typedef std::map<BasicBlock*, std::map<std::string, ir::Value*>> IncompletePhis;
        IncompletePhis incompletePhis_;
    };
}



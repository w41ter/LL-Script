#pragma once

#include <list>
#include <string>
#include <map>
#include <set>
#include <vector>

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
        using Instruction = ir::Instruction;
    public:
        BasicBlock(int id, std::string name) 
            : ID_(id), name_(name)
        {}

        void addPrecursor(BasicBlock *block);
        void addSuccessor(BasicBlock *block);
        size_t numOfPrecursors() const { return precursors_.size(); }
        size_t numOfSuccessors() const { return successors_.size(); }
        BasicBlock *precursor(int idx);
        BasicBlock *successor(int idx);

        typedef std::vector<BasicBlock*>::iterator precursor_iterator;
        typedef std::vector<BasicBlock*>::iterator successor_iterator;
        precursor_iterator precursor_begin() { return precursors_.begin(); }
        precursor_iterator precursor_end() { return precursors_.end(); }
        successor_iterator successor_begin() { return successors_.begin(); }
        successor_iterator successor_end() { return successors_.end(); }
        
        typedef std::list<Instruction*>::iterator instr_iterator;
        instr_iterator instr_begin() { return instrs_.begin(); }
        instr_iterator instr_end() { return instrs_.end(); }
        size_t numOfInstrs() const { return instrs_.size(); }
        void push_back(Instruction *instr);
        void push_front(Instruction *instr);
        void pop_back();
        void pop_front();
        void erase(Instruction *instr);

        unsigned getID() const { return ID_; }
        const std::string &getName() const { return name_; }
    protected:
        unsigned ID_;
        std::string name_;
        std::list<Instruction*> instrs_;
        std::vector<BasicBlock*> precursors_;   // 记录该基本块所有前驱
        std::vector<BasicBlock*> successors_;  // 后继基本块
    };

    class CFG
    {
        friend class DumpIR;
        using Value = ir::Value;
    public:
        CFG();
        ~CFG();

        BasicBlock *createBasicBlock(const std::string &name);
        void setEntry(BasicBlock *entry);
        void setEnd(BasicBlock *end);
        BasicBlock *getEntryBlock();
        
        typedef std::list<BasicBlock*>::iterator block_iterator;
        block_iterator begin() { return blocks_.begin(); }
        block_iterator end() { return blocks_.end(); }

        IRContext *getContext();

        // SSA form construction.
        void sealOthersBlock();
        void sealBlock(BasicBlock *block);
        void saveVariableDef(std::string name, BasicBlock *block, Value *value);
        Value *readVariableDef(std::string name, BasicBlock *block);

        std::string phiName(std::string &name);

    protected:
        // SSA
        Value *readVariableRecurisive(std::string name, BasicBlock *block);
        Value *addPhiOperands(std::string name, ir::Phi *phi);
        Value *tryRemoveTrivialPhi(ir::Phi *phi);

    protected:
        unsigned numBlockIDs_;
        BasicBlock *start_; // 起始基本块
        BasicBlock *end_;
        std::list<BasicBlock*> blocks_;

        IRContext *context_;

        typedef std::map<BasicBlock*, Value*> Block2Value;
        typedef std::map<std::string, Value*> String2Value;
        typedef std::map<std::string, Block2Value> Definition;
        typedef std::map<BasicBlock*, String2Value> IncompletePhis;
        Definition currentDef_;
        IncompletePhis incompletePhis_;
        std::set<BasicBlock*> sealedBlock_;

        std::map<std::string, int> phiCounts_;
    };
}



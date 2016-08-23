#pragma once

#include <list>
#include <string>
#include <map>
#include <set>
#include <vector>

namespace script
{
    class Phi;
    class Value;
    class Instruction;
    class IRContext;
    
    class BasicBlock
    {
    public:
        BasicBlock(std::string name) 
            : ID_(-1), name_(name)
        {}
        ~BasicBlock();

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
        typedef std::list<Instruction*>::reverse_iterator instr_riterator;
        instr_riterator instr_rbegin() { return instrs_.rbegin(); }
        instr_riterator instr_rend() { return instrs_.rend(); }
        size_t numOfInstrs() const { return instrs_.size(); }

        void push_back(Instruction *instr);
        void push_front(Instruction *instr);
        void pop_back();
        void pop_front();

        bool contains(Instruction *instr);
        void erase(Instruction *instr);
        void remove(Instruction *instr);
        void replaceInstrWith(Instruction *from, Instruction *to);
        
        void setID(unsigned ID) { ID_ = ID; }
        unsigned getID() const { return ID_; }
        const std::string &getName() const { return name_; }
        
        typedef std::list<Phi*>::iterator phi_iterator;
        phi_iterator phi_begin() { return phiNodes_.begin(); }
        phi_iterator phi_end() { return phiNodes_.end(); }
        size_t phi_size() { return phiNodes_.size(); }
    protected:
        // ID_ : use by liveIntervalAnalysis, default is -1.
        unsigned ID_;
        std::string name_;
        std::list<Phi*> phiNodes_;
        std::list<Instruction*> instrs_;
        std::vector<BasicBlock*> precursors_;
        std::vector<BasicBlock*> successors_; 
    };

    class CFG
    {
        friend class DumpIR;
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
        Value *addPhiOperands(std::string name, Phi *phi);
        Value *tryRemoveTrivialPhi(Phi *phi);

    protected:
        unsigned numBlockIDs_;
        BasicBlock *start_; 
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



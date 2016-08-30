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
		friend class CFG;
		friend class LiveIntervalAnalysis;
		friend class RegisterAllocator;
    public:
		enum BlockVisitState {
			Unvisit = 0x1,
			Visited = Unvisit << 1,
			Active  = Visited << 1,
			Assign  = Active  << 1,
		};

        BasicBlock(std::string name) 
            : name_(name)
        {}
        ~BasicBlock();

        typedef std::vector<BasicBlock*>::iterator precursor_iterator;
        typedef std::vector<BasicBlock*>::iterator successor_iterator;

        void addPrecursor(BasicBlock *block);
        void addSuccessor(BasicBlock *block);
        size_t numOfPrecursors() const { return precursors_.size(); }
        size_t numOfSuccessors() const { return successors_.size(); }
        BasicBlock *precursor(int idx);
        BasicBlock *successor(int idx);
        precursor_iterator precursor_begin() { return precursors_.begin(); }
        precursor_iterator precursor_end()   { return precursors_.end(); }
        successor_iterator successor_begin() { return successors_.begin(); }
        successor_iterator successor_end()   { return successors_.end(); }
        
        typedef std::list<Instruction*>::iterator instr_iterator;
        typedef std::list<Instruction*>::reverse_iterator instr_riterator;
        
        instr_iterator instr_begin()    { return instrs_.begin(); }
        instr_iterator instr_end()      { return instrs_.end(); }
        instr_riterator instr_rbegin()  { return instrs_.rbegin(); }
        instr_riterator instr_rend()    { return instrs_.rend(); }
		Instruction *front()            { return instrs_.front(); }
		Instruction *back()             { return instrs_.back(); }
        size_t numOfInstrs() const { return instrs_.size(); }

        void pop_back           ();
        void pop_front          ();
        void erase              (Instruction *instr);
        void remove             (Instruction *instr);
        bool contains           (Instruction *instr);
        void push_back          (Instruction *instr);
        void push_front         (Instruction *instr);
        void insert             (instr_iterator iter, Instruction *instr);
        void replaceInstrWith   (Instruction *from, Instruction *to);

        const std::string &getBlockName() const { return name_; }
        
        typedef std::list<Phi*>::iterator phi_iterator;
        phi_iterator phi_begin   () { return phiNodes_.begin(); }
        phi_iterator phi_end     () { return phiNodes_.end(); }
        size_t phi_size          () { return phiNodes_.size(); }

		void setBlockID(int ID) { blockID_ = ID; }
		int  getBlockID() const { return blockID_; }
		void setLoopIndex(int index) { loopIndex_ = index; }
		int  getLoopIndex() const { return loopIndex_; }
		void setLoopDepth(int index) { loopDepth_ = index; }
		int  getLoopDepth() const { return loopDepth_; }

		unsigned getStart() const { return start_; }
		unsigned getEnd()	const { return end_; }
    protected:
        void tryRemovePhiNode(Instruction *instr);
        void tryInsertPhiNode(Instruction *instr);
        void removePhiNodeRecord(Phi *phi);
        void recordPhiNode(Phi *phi);
		
		// falgs
		int blockID_;
		int loopIndex_;
		int loopDepth_;
		unsigned state_;
		unsigned incomingForwardBranches_;
		std::set<Value*> liveIn_;
		std::set<Value*> liveGen_;
		std::set<Value*> liveKill_;
		std::set<Value*> liveOut_;

		// instruction interval, [start_, end_)
		unsigned start_;
		unsigned end_;

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
		typedef std::list<BasicBlock*>::reverse_iterator 
			block_reverse_iterator;
        block_iterator begin() { return blocks_.begin(); }
        block_iterator end() { return blocks_.end(); }
		block_reverse_iterator rbegin() { return blocks_.rbegin(); }
		block_reverse_iterator rend() { return blocks_.rend(); }
        void erase(BasicBlock *block);
        
        // SSA form construction.
        void sealOthersBlock();
        void sealBlock(BasicBlock *block);
        void saveVariableDef(std::string name, BasicBlock *block, Value *value);
        Value *readVariableDef(std::string name, BasicBlock *block);

        std::string phiName(const std::string &name);

		void computeBlockOrder();
    protected:
        // SSA
        Value *readVariableRecurisive(std::string name, BasicBlock *block);
        Value *addPhiOperands(std::string name, Phi *phi);
        Value *tryRemoveTrivialPhi(Phi *phi);

		// compute block order
		struct BlockOrderCmp {
			bool operator () (
				const BasicBlock *LHS, 
				const BasicBlock *RHS) const
			{
				return LHS->loopDepth_ < RHS->loopDepth_;
			}
		};
		typedef std::map<BasicBlock*, BasicBlock*> B2B;
		void loopDetection();
		void numberOperations();
		void tryToDetect(B2B &set, BasicBlock *block);
		void tryToAssignIndex(
			int index, 
			BasicBlock *current, 
			BasicBlock *target,
			std::set<BasicBlock*> &visited);

    protected:
        unsigned numBlockIDs_;
		unsigned numLoopIndex_;
        BasicBlock *start_; 
        BasicBlock *end_;
        std::list<BasicBlock*> blocks_;

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



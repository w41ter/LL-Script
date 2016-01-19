#include <map>
#include <list>
#include <functional>
#include <algorithm>

#include "CFG.h"
#include "QuadGenerator.h"

namespace script
{

namespace {

    class LabelTarget : public QuadVisitor
    {
        LabelTarget() = default;
    public:
        static LabelTarget &instance()
        {
            static LabelTarget tar;
            return tar;
        }

        virtual ~LabelTarget() {}
        virtual bool visit(Constant *v) { return false; };
        virtual bool visit(Temp *v) { return false; };
        virtual bool visit(Identifier *v) { return false; };
        virtual bool visit(Array *v) { return false; };
        virtual bool visit(ArrayIndex *v) { return false; };

        virtual bool visit(If *v) { target_ = v; return true; };
        virtual bool visit(Call *v) { target_ = v; return false; };
        virtual bool visit(Goto *v) { target_ = v; return true; };
        virtual bool visit(Copy *v) { target_ = v; return false; };
        virtual bool visit(Load *v) { target_ = v; return false; };
        virtual bool visit(Store *v) { target_ = v; return false; };
        virtual bool visit(Label *v) { if (v->next_ != nullptr) v->next_->accept(this); else target_ = nullptr; return true; };
        virtual bool visit(Param *v) { target_ = v; return false; };
        virtual bool visit(Invoke *v) { target_ = v; return false; };
        virtual bool visit(Return *v) { target_ = v; return true; }; // 
        virtual bool visit(IfFalse *v) { target_ = v; return true; };
        virtual bool visit(Operation *v) { target_ = v; return false; };
        virtual bool visit(AssignArray *v) { target_ = v; return false; };
        virtual bool visit(ArrayAssign *v) { target_ = v; return false; };

        Quad *getTarget(Quad *quad)
        {
            quad->accept(this);
            return target_;
        }

    private:
        Quad *target_;
    };

    class TerminalrBranch : public QuadVisitor
    {
    public:
        TerminalrBranch(std::function<void(Label*, Quad*)> call)
            : callback_(std::move(call))
        {}

        virtual ~TerminalrBranch() {}
        virtual bool visit(Constant *v) { return false; };
        virtual bool visit(Temp *v) { return false; };
        virtual bool visit(Identifier *v) { return false; };
        virtual bool visit(Array *v) { return false; };
        virtual bool visit(ArrayIndex *v) { return false; };

        virtual bool visit(If *v) 
        { 
            callback_(v->label_, target_.getTarget(v->next_)); 
            return true; 
        };
        virtual bool visit(Call *v) { return false; };
        virtual bool visit(Goto *v) 
        { 
            callback_(v->label_, state ? nullptr 
                : target_.getTarget(v->next_)); 
            return true; 
        };
        virtual bool visit(Copy *v) { return false; };
        virtual bool visit(Load *v) { return false; };
        virtual bool visit(Store *v) { return false; };
        virtual bool visit(Label *v) { return false; };
        virtual bool visit(Param *v) { return false; };
        virtual bool visit(Invoke *v) { return false; };
        virtual bool visit(Return *v) 
        { 
            callback_(nullptr, state ? nullptr 
                : target_.getTarget(v->next_)); 
            return true; 
        }; // 
        virtual bool visit(IfFalse *v) 
        { 
            callback_(v->label_, target_.getTarget(v->next_)); 
            return true; 
        };
        virtual bool visit(Operation *v) { return false; };
        virtual bool visit(AssignArray *v) { return false; };
        virtual bool visit(ArrayAssign *v) { return false; };

        void bind(std::function<void(Label*, Quad*)> call)
        {
            callback_ = std::move(call);
        }

        void setState(bool s) { state = s; }
    private:
        bool state = false;     // false get all label, true get olny can arrive label.
        std::function<void(Label*, Quad*)> callback_;
        LabelTarget &target_ = LabelTarget::instance();
    };

}

    void CFG::buildTarget(QuadContext *context, CFG *cfg)
    {
        Quad *root = context->begin();
        while (root != nullptr)
        {
            Quad *t = LabelTarget::instance().getTarget(root);
            if (t != root)
            {
                cfg->labelTarget_.insert(
                    std::pair<Label*, Quad*>((Label*)root, t));
            }
            root = root->next_;
        }
    }

    void CFG::removeDeadBlock(CFG * cfg)
    {
        BasicBlock *first = cfg->blocks_.front();
        cfg->blocks_.pop_front();
        cfg->blocks_.remove_if([](BasicBlock *bb) -> bool {
            if (bb->numOfPrecursors() == 0)
            {
                delete bb;
                return true;
            }
            return false;
        });
        cfg->blocks_.push_front(first);
    }

    BasicBlock * CFG::createBlock()
    {
        BasicBlock *temp = new BasicBlock(numBlockIDs_++);
        blocks_.push_back(temp);
        return temp;
    }

    CFG::CFG()
        : numBlockIDs_(0), start_(nullptr), end_(nullptr)
    {
    }

    CFG::~CFG()
    {
        for (auto &i : blocks_)
        {
            delete i;
            i = nullptr;
        }
    }

    std::unique_ptr<CFG> CFG::buildCFG(QuadContext * context)
    {
        //context->codes_;
        std::unique_ptr<CFG> cfg = std::make_unique<CFG>();
        auto &targets = cfg->labelTarget_;

        buildTarget(context, cfg.get());

        std::list<Quad*> leader;
        std::map<Quad*, BasicBlock*> nodes;

        std::function<void(Quad*)> map(
            [&leader, &nodes, &cfg](Quad *quad) 
        {
            if (nodes.count(quad) != 0) return;
            nodes[quad] = cfg->createBlock();
            leader.push_back(quad);
        });

        TerminalrBranch breach([&map, &targets](Label *label, Quad *quad) 
        {
            Quad *tar = targets[label];
            if (tar != nullptr) map(tar);
            if (quad != nullptr)  map(quad);
        });

        // find all first quad of basic block.
        // first instructon is
        Quad *begin = LabelTarget::instance().getTarget(context->begin());
        if (begin == nullptr)   // if block is empty.
        {
            cfg->start_ = cfg->end_ = cfg->createBlock();
            cfg->start_->set(context->begin(), context->begin());
            return std::move(cfg);
        }

        breach.setState(false);
        map(begin);
        Quad *root = begin;
        while (root != nullptr)
        {
            root->accept(&breach);
            root = root->next_;
        }

        // find end and add edge.
        breach.setState(true);
        for (auto begin : leader)
        {
            Quad *end = begin;
            while (end->next_ != nullptr && nodes.count(end->next_) == 0)
                end = end->next_;

            BasicBlock *from = nodes[begin];
            from->set(begin, end);
            
            // add edge
            map = [&nodes, &from](Quad *quad) {
                BasicBlock *to = nodes[quad];
                from->addSuccessor(to);
                to->addPrecursor(from);
            };
            end->accept(&breach);
        }

        removeDeadBlock(cfg.get());

        // now add succ for end block
        cfg->start_ = nodes[leader.front()];
        if (cfg->blocks_.size() > 1)
        {
            cfg->end_ = cfg->createBlock();
            auto begin = cfg->blocks_.begin()++;
            while (begin != cfg->blocks_.end())
            {
                BasicBlock *block = *begin++;
                if (block == cfg->end_) continue;
                if (block->numOfPrecursors() > 0 && block->numOfSuccessors() == 0)
                {
                    block->addSuccessor(cfg->end_);
                    cfg->end_->addPrecursor(block);
                }
            }
        }
        else
        {
            cfg->end_ = cfg->start_;
        }

        return std::move(cfg);
    }

    void BasicBlock::addPrecursor(BasicBlock * block)
    {
        precursors_.push_back(block);
    }

    void BasicBlock::addSuccessor(BasicBlock * block)
    {
        successors_.push_back(block);
    }

    void BasicBlock::unique()
    {
        precursors_.sort();
        auto end_unique = std::unique(precursors_.begin(), precursors_.end());
        precursors_.erase(end_unique, precursors_.end());
        
        successors_.sort();
        end_unique = std::unique(successors_.begin(), successors_.end());
        precursors_.erase(end_unique, successors_.end());
    }
}


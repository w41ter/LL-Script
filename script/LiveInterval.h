#pragma once

#include <stddef.h>
#include <cassert>
#include <vector>
#include <list>

namespace script 
{
    class Value;

	// These ranges are rendered as [start,end).
	struct LiveRange {
		unsigned start;  // Start point of the interval (inclusive)
		unsigned end;    // End point of the interval (exclusive)

		LiveRange(unsigned S, unsigned E) : start(S), end(E) {
			assert(S < E && "Cannot create empty or backwards range");
		}

		void setStart(unsigned F) { start = F; }
		void setEnd(unsigned E) { end = E; }

		/// contains - Return true if the index is covered by this range.
		///
		bool contains(unsigned I) const {
			return start <= I && I < end;
		}

		bool operator < (const LiveRange &LR) const {
			return start < LR.start || (start == LR.start && end < LR.end);
		}
		bool operator == (const LiveRange &LR) const {
			return start == LR.start && end == LR.end;
		}

	private:
		LiveRange(); // DO NOT IMPLEMENT
	};

	inline bool operator < (unsigned V, const LiveRange &LR) {
		return V < LR.start;
	}

	inline bool operator < (const LiveRange &LR, unsigned V) {
		return LR.start < V;
	}

	/// LiveInterval - This class represents some number of live ranges for a
	/// register or value.  This class also contains a bit of register allocator
	/// state.
	struct LiveInterval {
		typedef Value* VirtualRegister;
		typedef std::list<LiveRange> Ranges;
		typedef std::vector<unsigned> Positions;
		VirtualRegister reg;        // the register of this interval
		int assignedReg;
		Positions usePositions;
		LiveInterval *splitParent;
		std::vector<LiveInterval*> splitChildren;
		Ranges ranges;       // the ranges in which this register is live
	
	public:

		LiveInterval(VirtualRegister Reg)
			: reg(Reg), assignedReg(-1), splitParent(0) {
		}

		typedef Ranges::iterator iterator;
		iterator begin() { return ranges.begin(); }
		iterator end() { return ranges.end(); }

		typedef Ranges::const_iterator const_iterator;
		const_iterator begin() const { return ranges.begin(); }
		const_iterator end() const { return ranges.end(); }

		bool empty() const { return ranges.empty(); }

		int getAssignReg() const { return assignedReg; }
		bool isAssignReg() const { return assignedReg != -1; }

		bool isSplitParent() const { return splitParent == nullptr; }
		LiveInterval *getSplitParent() const { return splitParent; }
		void setSplitParent(LiveInterval *parent) {
			splitParent = parent;
		}
		LiveInterval *childAt(unsigned op) {
			for (auto range : ranges) {
				if (range.contains(op))
					return this;
			}
			for (auto *child : splitChildren) {
				auto *res = child->childAt(op);
				if (res)
					return res;
			}
			return nullptr;
		}

		// beginNumber - Return the lowest numbered slot covered 
		// by interval.
		unsigned beginNumber() const {
			assert(!empty() && "empty interval for register");
			return ranges.front().start;
		}

		void setStart(unsigned S) {
			//assert(!empty() && "empty interval for register");
			if (empty())
				ranges.push_back({ S, S + 2 });
			else 
				ranges.front().setStart(S);
		}

		void setEnd(unsigned E) {
			assert(!empty() && "empty interval for register");
			ranges.back().setEnd(E);
		}

		// endNumber - return the maximum point of the interval 
		// of the whole, exclusive.
		unsigned endNumber() const {
			assert(!empty() && "empty interval for register");
			return ranges.back().end;
		}

		bool expiredAt(unsigned index) const {
			return index >= endNumber();
		}

		bool liveAt(unsigned I) const;

		// overlaps - Return true if the intersection of the two 
		// live intervals is not empty.
		bool overlaps(const LiveInterval& other) const {
			return overlapsFrom(other, other.begin());
		}

		/// overlapsFrom - Return true if the intersection of the two 
		// live intervals is not empty.  The specified iterator is a
		// hint that we can begin scanning the Other interval starting at I.
		bool overlapsFrom(const LiveInterval& other, const_iterator I) const;

		// addRange - Add the specified LiveRange to this interval, 
		// merging intervals as appropriate.  This returns an iterator 
		// to the inserted live range (which may have grown since it 
		// was inserted.
		void addRange(LiveRange LR);

		void addUsePosition(unsigned pos);

		unsigned nextUseAfter(unsigned pos);
		unsigned firstUse() const;

		/// advanceTo - Advance the specified iterator to point to the LiveRange
		/// containing the specified position, or end() if the position is past the
		/// end of the interval.  If no LiveRange contains this position, but the
		/// position is in a hole, this method returns an iterator pointing the the
		/// LiveRange immediately after the hole.
		iterator advanceTo(iterator I, unsigned Pos) {
			if (Pos >= endNumber())
				return end();
			while (I->end <= Pos) ++I;
			return I;
		}

		unsigned getSize() const;

		bool operator<(const LiveInterval& other) const {
			return beginNumber() < other.beginNumber();
		}
	};
}

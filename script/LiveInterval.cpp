#include "LiveInterval.h"

#include <algorithm>

namespace script
{

	// An example for liveAt():
	//
	// this = [1,4), liveAt(0) will return false. The instruction defining this
	// spans slots [0,3]. The interval belongs to an spilled definition of the
	// variable it represents. This is because slot 1 is used (def slot) and spans
	// up to slot 3 (store slot).
	//
	bool LiveInterval::liveAt(unsigned I) const {
		Ranges::const_iterator r = 
			std::upper_bound(ranges.begin(), ranges.end(), I);

		if (r == ranges.begin())
			return false;

		--r;
		return r->contains(I);
	}

	// overlaps - Return true if the intersection of the two live intervals is
	// not empty.
	//
	// An example for overlaps():
	//
	// 0: A = ...
	// 4: B = ...
	// 8: C = A + B ;; last use of A
	//
	// The live intervals should look like:
	//
	// A = [3, 11)
	// B = [7, x)
	// C = [11, y)
	//
	// A->overlaps(C) should return false since we want to be able to join
	// A and C.
	//
	bool LiveInterval::overlapsFrom(const LiveInterval& other,
		const_iterator StartPos) const {
		const_iterator i = begin();
		const_iterator ie = end();
		const_iterator j = StartPos;
		const_iterator je = other.end();

		assert((StartPos->start <= i->start || StartPos == other.begin()) &&
			StartPos != other.end() && "Bogus start position hint!");

		if (i->start < j->start) {
			i = std::upper_bound(i, ie, j->start);
			if (i != ranges.begin()) --i;
		}
		else if (j->start < i->start) {
			++StartPos;
			if (StartPos != other.end() && StartPos->start <= i->start) {
				j = std::upper_bound(j, je, i->start);
				if (j != other.ranges.begin()) --j;
			}
		}
		else {
			return true;
		}

		if (j == je) return false;

		while (i != ie) {
			if (i->start > j->start) {
				std::swap(i, j);
				std::swap(ie, je);
			}

			if (i->end > j->start)
				return true;
			++i;
		}

		return false;
	}

	void LiveInterval::addRange(LiveRange LR)
	{
		if (ranges.size() && beginNumber() <= LR.end) {
			// join intersecting ranges
			LiveRange &front = ranges.front();
			front.setStart(std::min(LR.start, front.start));
			front.setEnd(std::max(LR.end, front.end));
		}
		else {
			// insert new range
			ranges.push_front(LR);
		}
	}

	void LiveInterval::addUsePosition(unsigned pos)
	{
		assert(liveAt(pos) && "use position not covered by live range");

		// Note: add_use is called in descending order, so list gets sorted
		// automatically by just appending new use positions  
		usePositions.push_back(pos);
	}

	unsigned LiveInterval::nextUseAfter(unsigned pos)
	{
		auto result = std::upper_bound(
			usePositions.begin(), usePositions.end(), pos);
		return *result;
	}

	unsigned LiveInterval::firstUse() const
	{		   
		assert(usePositions.size());
		return usePositions.front();
	}

	unsigned LiveInterval::getSize() const {
		unsigned Sum = 0;
		for (const_iterator I = begin(), E = end(); I != E; ++I)
			Sum += I->end - I->start;
		return Sum;
	}
}
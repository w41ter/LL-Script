#pragma once

#include <cassert>

namespace script
{
    class MachineRegister;

    typedef std::vector<MachineRegister> MachineRegisters;

	class MachineRegister
	{
	public:
		enum MRStatus {
			Allocated,
			Spilled,
		};

		MachineRegister(unsigned reg, MRStatus S = Allocated)
			: status(S), num(reg)
		{}

		bool isAllocated() const { return status == Allocated; }
		bool isSpilled()   const { return status == Spilled; }

		void setRegisterNum(unsigned reg) {
			assert(status == Allocated);
			num = reg;
		}

		void setStackSlotPos(unsigned pos) {
			assert(status == Spilled);
			num = pos;
		}

		unsigned getRegisterNum() const {
			assert(status == Allocated);
			return num;
		}

		unsigned getStackSlotPos() const {
			assert(status == Spilled);
			return num;
		}
	private:
		MRStatus status;
		unsigned num;
	};
}
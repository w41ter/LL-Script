#ifndef __VM_H__
#define __VM_H__

#include <stdexcept>
#include <vector>
#include <string>
#include <stack>
#include <queue>
#include <cassert>
#include <cstring>

#include "opcode.h"
#include "Runtime.h"
#include "GC.h"
#include "OpcodeModule.h"

namespace script
{
	struct VMFrame {
		VMFrame(size_t RN, size_t PN, unsigned RR,
			const OpcodeFunction *func)
			: regNums(RN), paramsNums(PN), ip(0)
			, content(func), resReg(RR) {
			create();
		}

		~VMFrame() {
			destory();
		}

		void create() {
			registers = new Object[regNums];
			params = new Object[paramsNums];
			memset(registers, 0, sizeof(Object) * regNums);
			memset(params, 0, sizeof(Object) * paramsNums);
		}

		void destory() {
			delete[] registers;
			delete[] params;
		}

		void resetSlot(size_t RN, size_t PN) {
			regNums = RN;
			paramsNums = PN;
			destory();
			create();
		}

		void resetContent(const OpcodeFunction *func) {
			content = func;
			ip = 0;
		}

		Object getParamVal(size_t idx) {
			assert(idx < paramsNums);
			return params[idx];
		}

		void setParamVal(size_t idx, Object val) {
			assert(idx < paramsNums);
			params[idx] = val;
		}

		Object getRegVal(size_t idx) {
			assert(idx < regNums);
			return registers[idx];
		}

		void setRegVal(size_t idx, Object val) {
			assert(idx < regNums);
			registers[idx] = val;
		}

		unsigned resReg;
		size_t ip;
		size_t regNums;
		size_t paramsNums;
		Object *registers;
		Object *params;
		const OpcodeFunction *content;
	};

	struct VMScene {
		VMScene(OpcodeModule &OM)
			: module(OM), GC(1024 * 1024) {}

		OpcodeModule &module;
		GarbageCollector GC;

		std::vector<Object> paramsStack;
		std::vector<VMFrame*> frames;
	};

	class VMState
	{
	public:
		VMState();

		void bindScene(VMScene *scene);
		void execute();

		void call(Object func, int32_t paramsNums, unsigned res);
		void tailCall(Object func, int32_t paramsNums, unsigned res);
		Object fillClosureWithParams(Object func, int32_t paramsNum);

	private:
		void runtimeError(const char *str);
		void popParamsStack(size_t nums);
		void clearSceneStack();
		
		int32_t getInteger(size_t & ip);
		float getFloat(size_t & ip);

		void executeBinary(size_t &ip, unsigned op);
		void executeNotOP(size_t &ip);
		void executeMove(size_t &ip);
		void executeMoveI(size_t &ip);
		void executeMoveF(size_t &ip);
		void executeMoveS(size_t &ip);
		void executeMoveN(size_t &ip);
		void executeCall(size_t &ip);
		void executeTailCall(size_t &ip);
		void executeGoto(size_t &ip);
		void executeIf(size_t &ip);
		void executeReturn(size_t &ip);
		void executeLoad(size_t &ip);
		void executeStore(size_t &ip);
		void executeIndex(size_t &ip);
		void executeSetIndex(size_t &ip);
		void executeParam(size_t &ip);
		void executeNewClosure(size_t &ip);
		void executeNewHash(size_t &ip);

		VMFrame *topFrame;
		VMScene *currentScene;
	};

	void BindGCProcess(VMScene *scene);
}

#endif // !__VM_H__

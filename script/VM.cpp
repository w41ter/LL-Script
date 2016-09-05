#include "VM.h"

#include <string>
#include <vector>
#include <functional>
#include <cassert>
#include <string.h>
#include <iostream>	
#include <iomanip>

#include "Buildin.h"

using std::vector;
using std::string;

namespace script
{
	static size_t FrameMaxSize = 256;
	static GarbageCollector *globalGC;

	void SetGlobalGC(GarbageCollector *GC)
	{
		assert(GC);
		globalGC = GC;
	}

	void ReleaseGlobalGC()
	{
		globalGC = nullptr;
	}

	extern "C" Object Allocate(size_t size)
	{
		assert(globalGC && "Please bind gc object");
		return globalGC->allocate(size);
	}

	VMState::VMState() { }

	void VMState::bindScene(VMScene * scene)
	{
		currentScene = scene;
		SetGlobalGC(&scene->GC);
	}

	VMScene * VMState::getScene()
	{
		return currentScene;
	}

	void VMState::execute()
	{
		if (!currentScene || !currentScene->frames.size())
			return;		 

		bool runState = true;
		while (runState) {
			if (currentScene->frames.size() == 0)
				break;
			topFrame = &currentScene->frames.back();
			auto &ip = topFrame->ip;
			switch (topFrame->content->codes[ip++])
			{
			case OK_Add:
			case OK_Sub:
			case OK_Div:
			case OK_Mul:
			case OK_Great:
			case OK_GreatThan:
			case OK_Less:
			case OK_LessThan:
			case OK_NotEqual:
			case OK_Equal:
				executeBinary(ip, topFrame->content->codes[ip - 1]);
				break;
			case OK_Not:
				executeNotOP(ip);
				break;
			case OK_Call:
				executeCall(ip);
				break;
			case OK_TailCall:
				executeTailCall(ip);
				break;
			case OK_Goto:
				executeGoto(ip);
				break;
			case OK_If:
				executeIf(ip);
				break;
			case OK_Return:
				executeReturn(ip);
				break;
			case OK_Load:
				executeLoad(ip);
				break;
			case OK_Store:
				executeStore(ip);
				break;
			case OK_Index:
				executeIndex(ip);
				break;
			case OK_SetIndex:
				executeSetIndex(ip);
				break;
			case OK_Move:
				executeMove(ip);
				break;
			case OK_MoveF:
				executeMoveF(ip);
				break;
			case OK_MoveI:
				executeMoveI(ip);
				break;
			case OK_MoveS:
				executeMoveS(ip);
				break;
			case OK_MoveN:
				executeMoveN(ip);
				break;
			case OK_Halt:
				runState = false;
				break;
			case OK_Param:
				executeParam(ip);
				break;
			case OK_NewClosure:
				executeNewClosure(ip);
				break;
			case OK_UserClosure:
				executeUserClosure(ip);
				break;
			case OK_NewHash:
				executeNewHash(ip);
				break;
			default:
				break;
			}
		}
	}

	void VMState::callUserClosure(Object closure,
		int32_t paramsNums, unsigned res)
	{
		typedef Object(*UserDefClosure)(VMState*, size_t);
		UserDefClosure call = (UserDefClosure)UserClosureGet(closure);
		assert(call);

		// save return reg.
		currentScene->lastValue = res;
		Object result = call(this, paramsNums);
		topFrame->setRegVal(res, result);
		popParamsStack(paramsNums);
	}

	void VMState::runtimeError(const char * str)
	{
		std::cout << "Except: " << str << std::endl;
		size_t total = currentScene->frames.size() - 1;
		for (int i = 0; i < 5; ++i) {
			VMFrame &frame = currentScene->frames[total - i];
			size_t idx = frame.content->name;
			const std::string &name = currentScene->module.getString(idx);
			std::cout << "\t#" << i << "  0x" << std::setfill('0')
				<< std::setw(8) << frame.ip << ":\t" << name << std::endl;
			if (total - i == 0)
				break;
		}
		clearSceneStack();
		throw;
	}

	void VMState::popParamsStack(size_t nums)
	{
		while (nums--)
			currentScene->paramsStack.pop_back();
	}

	void VMState::clearSceneStack()
	{
		currentScene->frames.clear();
	}

	void VMState::call(Object func, int32_t paramsNums, unsigned res)
	{
		size_t hold = ClosureHold(func);
		size_t stackSize = currentScene->paramsStack.size();

		OpcodeFunction *content =
			static_cast<OpcodeFunction*>(ClosureContent(func));
		currentScene->pushFrame(res, content);

		VMFrame *newFrame = &currentScene->frames.back();
		for (size_t i = 0; i < hold; ++i)
			newFrame->setParamVal(i, ClosureParamAt(func, i));
		for (size_t i = hold; i < hold + paramsNums; ++i) {
			size_t from = stackSize - (hold + paramsNums - i);
			newFrame->setParamVal(i, currentScene->paramsStack[from]);
		}
		if (currentScene->frames.size() + 1 >= FrameMaxSize) {
			runtimeError("stackoverflow");
			return;
		}
		popParamsStack(paramsNums);
	}

	void VMState::tailCall(Object func, int32_t paramsNums, unsigned res)
	{
		size_t hold = ClosureHold(func);
		size_t stackSize = currentScene->paramsStack.size();

		OpcodeFunction *content =
			static_cast<OpcodeFunction*>(ClosureContent(func));
		if (content != topFrame->content) {
			unsigned resultReg = topFrame->resReg;
			currentScene->popFrame(0);
			currentScene->pushFrame(resultReg, content);
		}
		VMFrame *newFrame = topFrame;
		for (size_t i = 0; i < hold; ++i)
			newFrame->setParamVal(i, ClosureParamAt(func, i));
		for (size_t i = hold; i < hold + paramsNums; ++i) {
			size_t from = stackSize - (hold + paramsNums - i);
			newFrame->setParamVal(i, currentScene->paramsStack[from]);
		}
		popParamsStack(paramsNums);
	}

	// 
	// create new closure and fill it's params with old and params stack.
	Object VMState::fillClosureWithParams(Object func, int32_t paramsNums)
	{
		size_t total = ClosureTotal(func);
		size_t hold = ClosureHold(func);
		size_t stackSize = currentScene->paramsStack.size();

		Object closure = currentScene->GC.allocate(
			SizeOfClosure(total));
		CreateClosure(closure, ClosureContent(func), total);

		for (size_t idx = 0; idx < hold; ++idx)
			ClosurePushParam(closure, ClosureParamAt(func, idx));
		for (size_t idx = hold; idx < hold + paramsNums; ++idx) {
			size_t from = stackSize - (hold + paramsNums - idx);
			ClosurePushParam(closure, currentScene->paramsStack[from]);
		}
		popParamsStack(paramsNums);
		return closure;
	}

	void VMState::executeBinary(size_t & ip, unsigned op)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		Object left = topFrame->getRegVal(opcode[ip++]);
		Object right = topFrame->getRegVal(opcode[ip++]);

		Object calRes = 0;
		switch (op)
		{
		case OK_Add: calRes = Add(left, right); break;
		case OK_Sub: calRes = Sub(left, right); break;
		case OK_Div: calRes = Div(left, right); break;
		case OK_Mul: calRes = Mul(left, right); break;
		case OK_Great: calRes = Great(left, right); break;
		case OK_GreatThan: calRes = NotLess(left, right); break;
		case OK_Less: calRes = Less(left, right); break;
		case OK_LessThan: calRes = NotGreat(left, right); break;
		case OK_NotEqual: calRes = NotEqual(left, right); break;
		case OK_Equal: calRes = Equal(left, right); break;
		}
		topFrame->setRegVal(result, calRes);
	}

	void VMState::executeNotOP(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		Object val = topFrame->getRegVal(opcode[ip++]);
		Object res = Not(val);
		topFrame->setRegVal(result, res);
	}

	void VMState::executeMove(size_t &ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		Object value = topFrame->getRegVal(opcode[ip++]);
		topFrame->setRegVal(result, value);
	}

	void VMState::executeMoveF(size_t &ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		Object value = CreateReal(getFloat(ip));
		topFrame->setRegVal(result, value);
	}

	void VMState::executeMoveI(size_t &ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		Object value = CreateFixnum(getInteger(ip));
		topFrame->setRegVal(result, value);
	}

	void VMState::executeMoveS(size_t &ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		int32_t stringIndex = getInteger(ip);
		const std::string &string = 
			currentScene->module.getString(stringIndex);
		Object str = currentScene->GC.allocate(
			SizeOfString(string.size()));
		CreateString(str, string.c_str(), string.size());
		topFrame->setRegVal(result, str);
	}

	void VMState::executeMoveN(size_t &ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		topFrame->setRegVal(result, CreateNil());
	}

	void VMState::executeCall(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned resultReg = opcode[ip++];
		Object func = topFrame->getRegVal(opcode[ip++]);
		int32_t argc = getInteger(ip);

		assert(currentScene->paramsStack.size() >= argc);

		if (!IsCallable(func)) {
			runtimeError("try to invoke incallable object");
			return;
		}

		if (IsUserClosure(func)) {
			callUserClosure(func, argc, resultReg);
			return;
		}

		int32_t total = ClosureTotal(func);
		int32_t hold = ClosureHold(func);
		int32_t target = argc + hold;

		if (target > total) {
			runtimeError("too many params");
			return;
		}
		if (target == total) 
			call(func, argc, resultReg);
		else 
			topFrame->setRegVal(resultReg,
				fillClosureWithParams(func, argc));
	}

	void VMState::executeTailCall(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned resultReg = opcode[ip++];
		Object func = topFrame->getRegVal(opcode[ip++]);
		int32_t argc = getInteger(ip);

		assert(currentScene->paramsStack.size() >= argc);

		if (!IsCallable(func)) {
			runtimeError("try to invoke incallable object");
			return;
		}

		if (IsUserClosure(func)) {
			callUserClosure(func, argc, resultReg);
			return;
		}

		int32_t total = ClosureTotal(func);
		int32_t hold = ClosureHold(func);
		int32_t target = total + hold;

		if (target > total) {
			runtimeError("too many params");
			return;
		}
		if (target == total)
			tailCall(func, argc, resultReg);
		else
			topFrame->setRegVal(resultReg,
				fillClosureWithParams(func, argc));
	}

	void VMState::executeGoto(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned offset = getInteger(ip);
		ip = offset;
	}

	void VMState::executeIf(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		Object cond = topFrame->getRegVal(opcode[ip++]);
		unsigned offset = getInteger(ip);
		if (ToLogicValue(cond))
			ip = offset;
	}

	void VMState::executeReturn(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		Object val = topFrame->getRegVal(opcode[ip++]);
		currentScene->popFrame(val);
	}

	void VMState::executeLoad(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		int32_t slot = getInteger(ip);
		Object val = topFrame->getParamVal(slot);
		topFrame->setRegVal(result, val);
	}

	void VMState::executeStore(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		Object val = topFrame->getRegVal(opcode[ip++]);
		int32_t slot = getInteger(ip);
		topFrame->setParamVal(slot, val);
	}

	void VMState::executeIndex(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		Object table = topFrame->getRegVal(opcode[ip++]);
		Object index = topFrame->getRegVal(opcode[ip++]);
		Object res = HashFind(table, index);
		topFrame->setRegVal(result, res);
	}

	void VMState::executeSetIndex(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		Object table = topFrame->getRegVal(opcode[ip++]);
		Object index = topFrame->getRegVal(opcode[ip++]);
		Object data = topFrame->getRegVal(opcode[ip++]);
		HashSetAndUpdate(table, index, data);
	}

	void VMState::executeParam(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		Object val = topFrame->getRegVal(opcode[ip++]);
		currentScene->paramsStack.push_back(val);
	}

	void VMState::executeNewClosure(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		int32_t offset = getInteger(ip);
		int32_t argc = getInteger(ip);

		assert(currentScene->paramsStack.size() >= argc);

		const std::string &name = currentScene->module.getString(offset);
		auto *content = &currentScene->module.getFunction(name);
		size_t numOfParams = content->paramSize;
		Object closure = currentScene->GC.allocate(
			SizeOfClosure(numOfParams));
		CreateClosure(closure, content, numOfParams);

		size_t stackSize = currentScene->paramsStack.size();
		for (size_t idx = 0; idx < argc; ++idx) {
			size_t from = stackSize - (argc - idx);
			ClosurePushParam(closure, currentScene->paramsStack[from]);
		}
		topFrame->setRegVal(result, closure);
		popParamsStack(argc);
	}

	void VMState::executeUserClosure(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		int32_t offset = getInteger(ip);
		const std::string &name = currentScene->module.getString(offset);
		auto *content = currentScene->module.getUserClosure(name);
		Object closure = currentScene->GC.allocate(SizeOfUserClosure());
		CreateUserClosure(closure, (void*)(content));
		topFrame->setRegVal(result, closure);
	}

	void VMState::executeNewHash(size_t & ip)
	{
		auto &opcode = topFrame->content->codes;
		unsigned result = opcode[ip++];
		topFrame->setRegVal(result, CreateHash());
	}

	int32_t VMState::getInteger(size_t & ip)
	{
		int32_t result = 0;
		auto &opcode = topFrame->content->codes;
		for (int i = 0; i < 4; ++i)
		{
			result <<= 8; 
			result |= (unsigned char)opcode[ip++];
		}
		return result;
	}

	float VMState::getFloat(size_t & ip)
	{
		int ival = getInteger(ip), *iptr = &ival;
		float *fptr = reinterpret_cast<float*>(iptr);
		return *fptr;
	}

	void BindGCProcess(VMScene * scene)
	{
		auto &GC = scene->GC;
		GC.bindGlobals(std::bind(&ProcessGlobals, scene));
		GC.bindReference(std::bind(&ProcessVariableReference,
			scene, std::placeholders::_1));
	}

	void VMScene::pushFrame(unsigned RR, const OpcodeFunction * content)
	{
		size_t regs = content->numOfregisters,
			args = content->paramSize;
		Object registers = GC.allocate(SizeOfArray(regs));
		Object params = GC.allocate(SizeOfArray(args));
		CreateArray(registers, regs);
		CreateArray(params, args);
		frames.push_back({ registers, params, RR, content });
	}

	void VMScene::popFrame(Object result)
	{
		unsigned resReg = frames.back().resReg;
		frames.pop_back();
		if (frames.size() > 0)
			frames.back().setRegVal(resReg, result);
		else
			lastValue = result;
	}
}
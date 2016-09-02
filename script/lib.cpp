#include "lib.h"

#include <iostream>
#include <functional>

#include "VM.h"
#include "lexical_cast.h"

using script::VMFrame;
using script::VMState;
using script::VMScene;

struct Lib
{
	const char *name;
	UserDefLibClosure closure;
};

static RequireCallback globalReguireCallback;

static void DumpObject(Object object)
{
	if (IsFixnum(object))
		std::cout << GetFixnum(object);
	else if (IsString(object))
		std::cout << StringGet(object);
	else if (IsUserClosure(object))
		std::cout << "User def<" << UserClosureGet(object) << ">";
	else if (IsHash(object))
		std::cout << "<hash>";
	else if (IsCallable(object))
		std::cout << "<closue>";
	else
		std::cout << "<object>";
}

Object lib_to_string(VMState *state, size_t paramsNums)
{
	if (paramsNums != 1) {
		state->runtimeError("to_string only takes one parameter");
	}
	Object res = state->getScene()->paramsStack.back();
	if (IsString(res))
		return res;
	else if (IsFixnum(res)) {
		std::string str = std::to_string(GetFixnum(res));
		Object result = state->getScene()->
			GC.allocate(SizeOfString(str.length()));
		return CreateString(result, str.c_str(), str.size());
	}
	else {
		const char *str = "";
		if (IsUserClosure(res))
			str = "<user closure>";
		else if (IsHash(res))
			str = "<hash>";
		else if (IsCallable(res))
			str = "<closue>";
		else
			str = "<object>";
		size_t length = strlen(str);
		Object result = state->getScene()->
			GC.allocate(SizeOfString(length));
		return CreateString(result, str, length);
	}
}

Object lib_to_integer(VMState *state, size_t paramsNums)
{
	if (paramsNums != 1) {
		state->runtimeError("to_integer only takes one parameter");
	}

	Object res = state->getScene()->paramsStack.back();
	if (IsString(res)) {
		try {
			int value = lexical_cast<int>(StringGet(res));
			return CreateFixnum(value);
		}
		catch (const std::exception &e) {
			return CreateNil();
		}
	}
	else if (IsFixnum(res))
		return res;
	else
		return CreateFixnum(1);
}

Object lib_is_null(VMState *state, size_t paramsNums)
{
	if (paramsNums != 1) {
		state->runtimeError("is_null only takes one parameter");
	}

	Object res = state->getScene()->paramsStack.back();
	return CreateFixnum(IsUndef(res) || IsNil(res));
}

Object lib_output(VMState * state, size_t paramsNums)
{
	VMScene *scene = state->getScene();
	for (size_t idx = scene->paramsStack.size() - paramsNums;
		idx < scene->paramsStack.size(); ++idx) {
		Object arg = scene->paramsStack[idx];
		DumpObject(arg);
	}
	return CreateNil();
}

Object lib_input(VMState * state, size_t paramsNums)
{
	lib_output(state, paramsNums);
	std::string str;
	std::cin >> str;
	Object result = state->getScene()->
		GC.allocate(SizeOfString(str.length()));
	return CreateString(result, str.c_str(), str.size());
}

Object lib_println(VMState *state, size_t paramsNums)
{
	lib_output(state, paramsNums);
	std::cout << std::endl;
	return CreateNil();
}

Object lib_require(VMState *state, size_t paramsNums)
{
	assert(globalReguireCallback);
	if (paramsNums != 1) {
		state->runtimeError("require only takes one parameter");
	}

	Object res = state->getScene()->paramsStack.back();
	if (IsString(res)) {
		// save it.
		std::string filename = StringGet(res); 
		VMScene *scene = state->getScene();
		unsigned resReg = static_cast<unsigned>(scene->lastValue);
		globalReguireCallback(filename.c_str(), resReg);
	}
	return CreateUndef();
}

static Lib libs[] = {
	{ "output", lib_output },
	{ "input", lib_input },
	{ "println", lib_println },
	{ "require", lib_require },
	{ "is_null", lib_is_null },
	{ "to_string", lib_to_string },
	{ "to_integer", lib_to_integer },
	{ nullptr, nullptr }
};

void RegisterLibrary(LibRegister lib_register)
{
	Lib *lib = libs;
	while (lib->name) {
		lib_register(lib->name, lib->closure);
		lib++;
	}
}

void RegisterRequire(RequireCallback require)
{
	globalReguireCallback = require;
}

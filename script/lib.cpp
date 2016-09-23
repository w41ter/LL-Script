#include "lib.h"

#include <iostream>

#include "VM.h"

using script::VMFrame;
using script::VMState;
using script::VMScene;


Object lib_input(VMState * state, size_t paramsNums)
{
	lib_output(state, paramsNums);
	return CreateNil();
}

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


struct Lib
{
	const char *name;
	UserDefLibClosure closure;
};

static Lib libs[] = {
	{ "output", lib_output },
	{ "input", lib_input },
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
#pragma once

#include <functional>

#include "Runtime.h"

namespace script {
	class VMState;
}

typedef Object(*UserDefLibClosure)(script::VMState*, size_t);
typedef std::function<void(const char *, UserDefLibClosure)> LibRegister;

void RegisterLibrary(LibRegister lib_register);

Object lib_input(script::VMState *state, size_t paramsNums);
Object lib_output(script::VMState *state, size_t paramsNums);

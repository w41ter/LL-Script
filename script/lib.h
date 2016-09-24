#pragma once

#include <functional>

#include "Runtime.h"

namespace script {
	class VMState;
}

typedef Object(*UserDefLibClosure)(script::VMState*, size_t);
typedef std::function<void(const char *, UserDefLibClosure)> LibRegister;

void RegisterLibrary(LibRegister lib_register);


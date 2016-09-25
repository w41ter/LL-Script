#pragma once

#include <functional>

#include "Runtime.h"

namespace script {
	class VMState;
}

typedef Object(*UserDefLibClosure)(script::VMState*, size_t);
typedef std::function<void(const char *, UserDefLibClosure)> LibRegister;
typedef std::function<void(const char*, unsigned)> RequireCallback;
void RegisterLibrary(LibRegister lib_register);
void RegisterRequire(RequireCallback require);


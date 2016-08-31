#include "OpcodeModule.h"

#include <cassert>

namespace script
{
    OpcodeModule::~OpcodeModule()
    {
    }

    size_t OpcodeModule::push_string(const std::string & str)
    {
		size_t idx = stringPool_.size();
		if (stringMap.count(str)) {
			return stringMap[str];
		}
		stringPool_.push_back(str);
		stringMap.insert({ str, idx });
		return idx;
    }

    const std::string & OpcodeModule::getString(size_t idx)
    {
		assert(stringPool_.size() > idx);
        return stringPool_[idx];
    }

    OpcodeFunction & OpcodeModule::getFunction(const std::string &name)
    {
		size_t idx = push_string(name);
		if (!functions_.count(idx)) {
			auto &func = functions_[idx];
			func.name = idx;
			return func;
		}
        return functions_[idx];
    }
}
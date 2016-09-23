#pragma once

#include "opcode.h"

#include <set>
#include <vector>
#include <map>
#include <unordered_map>

namespace script
{
    struct Opcodes
    {
		size_t numOfregisters;
		std::vector<size_t> params;
		std::vector<Byte> codes;
    };

    struct OpcodeFunction : public Opcodes
    {
        size_t name;
		size_t paramSize;
		size_t codeIndex;
    };

	class VMState;
	typedef Object(*UserDefClosure)(VMState*, size_t);

    class OpcodeModule : public Opcodes
    {
		friend class DumpOpcode;
    public:
        ~OpcodeModule();

		OpcodeFunction &getFunction(const std::string &name);
		UserDefClosure getUserClosure(const std::string &name);
		void pushUserClosure(const std::string &name, 
			UserDefClosure closure);
        size_t push_string(const std::string &str);
        const std::string &getString(size_t idx);

        size_t string_size() const { return stringPool_.size(); }
    protected:
		std::vector<std::string> stringPool_;
        std::unordered_map<std::string, const size_t> stringMap;
        std::map<size_t, OpcodeFunction> functions_;
		std::map<size_t, UserDefClosure> userClosure;
    };
}
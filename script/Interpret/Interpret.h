#pragma once

#include <unordered_map>
#include <map>

#include "../Syntax/syntax.h"

namespace ScriptCompile
{
	using std::map;
	using std::unordered_map;

	struct ReturnValue
	{
		int kind;
		Variable var;
	};

	typedef double(*OpFunction)(double left, double right);

	Variable ConstructVariable(wstring& value);
	void Run(Program* pro);
}
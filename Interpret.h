#pragma once

#include "syntax.h"

namespace ScriptCompile
{
	using std::map;

	struct ReturnValue
	{
		int kind;
		Variable var;
	};

	typedef double(*OpFunction)(double left, double right);

	bool Condition(Variable& slove, map<wstring, wstring>& envir);
	inline Variable ConstructVariable(wstring& value);
	inline wstring& GetValue(wstring& name, map<wstring, wstring>& envir);
	Variable Caculate(Variable& left, Variable& right, int kind, Operator op);
	Variable Caculate(Variable left, Variable right, int op, map<wstring, wstring>& envir);
	Variable EvalExpression(ASTreeExpression& expression, map<wstring, wstring>& envir);
	ReturnValue RunStatements(ASTreeStatements& statements, map<wstring, wstring>& envir);
	Variable RunFunction(Function* function, map<wstring, wstring>& envir);
	void Run(Program* pro);
}
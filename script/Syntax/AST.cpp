#include "AST.h"

namespace ScriptCompile
{
	//Variable::Variable(const Variable &v)
	//{
	//	kind = v.kind;
	//	dvalue = v.dvalue;
	//	svalue = v.svalue;
	//}

	//Variable &Variable::operator = (const Variable &v)
	//{
	//	kind = v.kind;
	//	dvalue = v.dvalue;
	//	svalue = v.svalue;
	//	return *this;
	//}

	//void Variable::SetValue(int k, const wstring &w)
	//{
	//	kind = k;
	//	svalue = w;
	//}

	//void Variable::SetValue(int k, double d)
	//{
	//	kind = k;
	//	svalue = d;
	//}

	void ASTreeStatements::InsertStatement(ASTreeStatements& rls)
	{
		for (auto i : rls.statements)
		{
			statements.push_back(i);
		}
	}
}

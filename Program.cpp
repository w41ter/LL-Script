#include "Program.h"

namespace ScriptCompile
{

	//Variable Plugin::Call(wstring& name, vector<wstring>& params)
	//{
	//	
	//}

	void Program::AddStatements(ASTreeStatements* s)
	{
		// 因为需要为statements分配空间
		if (statements == nullptr)
			statements = s;
		else
			statements->InsertStatement(*s);
	}

	void Program::AddFunction(Function* f)
	{
		functions[f->GetFunctionName()] = f;
		AddIdentifier(f->GetFunctionName());
	}

	void Program::AddPlugin(Plugin* p)
	{
		auto name = p->GetName();

		for (auto i : name)
			AddIdentifier(i);

		plugin = p;
	}
}
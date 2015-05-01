#include "Program.h"

namespace ScriptCompile
{
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
		identifier.insert(f->GetFunctionName());
	}

	void Program::AddPlugin(Plugin* p)
	{
		auto name = p->GetName();

		for (auto i : name)
			identifier.insert(i);

		plugin = p;
	}

	void Program::Check(BaseProgram* base)
	{
		for (auto i : base->GetIdentifierUse())
		{
			if (base->GetVariable().find(i) == base->GetVariable().end())
				throw ASTError({ TK_ERROR, 0, L"" }, L"未定义的标识符：" + i);
		}

		for (auto i : base->GetFuntionUse())
		{
			if (FindFunction(i.first) && functions[i.first]->GetParams().size() == i.second)
				continue;
			else if (plugin->GetName().find(i.first) != plugin->GetName().end())
				continue;
			else
			{
				wstring temp = L"调用函数：" + i.first;
				temp += L"时出错";
				throw ASTError({ TK_ERROR, 0, L"" }, temp);
			}
				
		}
	}

	void Program::CheckAll()
	{
		Check(this);

		for (auto i : functions)
		{
			Check(i.second);
		}
	}
}
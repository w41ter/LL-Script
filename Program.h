#pragma once

#include <set>
#include "AST.h"
#include <unordered_set>
#include <unordered_map>

namespace ScriptCompile
{
	using std::set;
	using std::unordered_set;
	using std::unordered_map;
	using std::unordered_multimap;

	struct Variable
	{
		int kind;
		// double dvalue;
		wstring value;
	};

	class Plugin
	{
	public:
		virtual Variable Call(wstring& name, vector<wstring>& params) = 0;

		unordered_set<wstring>& GetName() { return name; }
	protected:
		unordered_set<wstring> name;
		unordered_map<wstring, int> pluginName;
	};

	class BaseProgram
	{
	public:
		BaseProgram() {}

		void AddVariable(const wstring& i) { variable.insert(i); AddIdentifier(i); }
		unordered_set<wstring>& GetVariable() { return variable; }

		// identifier 包括变量名和函数名，在当前作用于中不能覆盖当前作用域已出现的名字
		void AddIdentifier(const wstring& i) { identifier.insert(i); }
		bool FindIndetifier(const wstring& i) { return identifier.find(i) != identifier.end(); }
		unordered_set<wstring>& GetIdentifier() { return identifier; }

		void AddIdentifierUse(const wstring& i) { identifierUse.insert(i); }
		unordered_set<wstring>& GetIdentifierUse() { return identifierUse; }

		void AddFuntionUse(const wstring& n, int p) { functionUse.insert({ n, p }); }
		unordered_multimap<wstring, int>& GetFuntionUse() { return functionUse; }

	private:
		unordered_set<wstring> variable;
		unordered_set<wstring> identifier;
		unordered_set<wstring> identifierUse;
		unordered_multimap<wstring, int> functionUse;
	};

	class Function : public BaseProgram
	{
	public:
		Function() : statements(NULL) {}
		~Function() {}

		void SetFunctionName(const wstring& n)	{ name = n; }
		wstring& GetFunctionName()				{ return name; }

		void AddParam(const wstring& s)			{ params.push_back(s); AddIdentifier(s); }
		vector<wstring>& GetParams()			{ return params; }

		void SetStatements(ASTreeStatements* s) { statements = s; }
		ASTreeStatements& GetStatements()		{ return *statements; }
	private:
		wstring				name;
		ASTreeStatements*	statements;
		vector<wstring>		params;
	};

	// 此处的addStatements和function中的并不一样，因为一个program可能有多个
	// statements，但是在Function中只能有一个Statements。
	class Program : public BaseProgram
	{
	public:
		Program() : statements(nullptr) {}
		~Program() {}

		void AddStatements(ASTreeStatements* s);
		void AddFunction(Function* f);
		void AddPlugin(Plugin* p);

		bool FindFunction(const wstring& name) const  { return functions.find(name) != functions.end(); }
		Function* GetFunction(const wstring& name)	{ return functions.find(name)->second; }
		map<wstring, Function*>& GetFunctions()	{ return functions; }

		Plugin& GetPlugin()	{ return *plugin; }

		ASTreeStatements& GetStatements()		{ return *statements; }

	private:
		Plugin*	plugin;
		ASTreeStatements* statements;
		map<wstring, Function*>	functions;
		
	};

}

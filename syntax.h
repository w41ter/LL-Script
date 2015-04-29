#pragma once

#include <string>
#include <vector>
#include <set>
#include <stack>
#include <queue>
#include <map>

#include "lexer.h"
#include "AST.h"
#include "Program.h"

namespace ScriptCompile
{
	using std::wstring;
	using std::vector;
	using std::set;
	using std::stack;
	using std::queue;
	using std::map;

	struct Operator
	{
		int kind;
		int lever;
		int bit;
	};

	// 
	// ≥Ã–Ú√Ë ˆ≤ø∑÷
	//
	//struct Variable
	//{
	//	wstring name;
	//	wstring value;
	//};

	//class Variables
	//{
	//public:
	//	Variables() {}
	//	~Variables() {}

	//	int Lookup(wstring& name);
	//	bool Insert(Variable& v);
	//	// bool Remove(wstring& name);
	//	bool SetValue(wstring& name, wstring& value);
	//	Variable& GetValue(unsigned int pos);
	//	
	//private:
	//	vector<Variable> members;
	//};

	template<class Type>
	class MemoryManager
	{
	public:
		MemoryManager() {}
		~MemoryManager() { Release(); }

		void Insert(Type* pointer) { if (pointer != nullptr) memory.push_back(pointer); }

		void Release()
		{
			for (auto i : memory)
			{
				delete i;
			}
		}
	private:
		vector<Type*> memory;
	};

	class ASTError
	{
	public:
		ASTError(Token t, wstring str) : token(t), msg(str) {}

		wstring GetMsg() { return msg; }
		Token GetToken() { return token; }
	private:
		Token	token;
		wstring msg;
	};
	
	ASTreeExpression* Expression(Lexer& lexer, BaseProgram* base);
	ASTreeReturnStatement* BreakOrReturnStatements(Lexer& lexer, BaseProgram* base);
	ASTreeWhileStatement* WhileStatements(Lexer& lexer, BaseProgram* base);
	ASTreeIfStatement* IfStatements(Lexer& lexer, BaseProgram* base);
	ASTreeStatements* Statements(Lexer& lexer, BaseProgram* base);
	Function* Defination(Lexer& lexer, BaseProgram* base);
	void Parser(wstring& file, Program& program);
}
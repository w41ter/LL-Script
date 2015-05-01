#pragma once

#include <string>
#include <vector>
#include <set>
#include <stack>
#include <queue>

#include "../Parser/lexer.h"
#include "AST.h"
#include "Program.h"

namespace ScriptCompile
{
	using std::wstring;
	using std::vector;
	using std::set;
	using std::stack;
	using std::queue;

	enum OperatorSign
	{
		OP_EQUAL,
		OP_VERTICAL,
		OP_AND,
		OP_LEFT_RIGHT,
		OP_NOT_EQUAL,
		OP_LEFT_ARROW,
		OP_RIGHT_ARROW,
		OP_LEFT_ARROW_EQUAL,
		OP_RIGHT_ARROW_EQUAL,
		OP_PLUS,
		OP_MINUS,
		OP_STAR,
		OP_SLASH,
		OP_PERCENT,
		OP_NOT,
	};

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

	class Parser
	{
	public:
		Parser(wstring &file) : lexer(file) {}

		void Bind(BaseProgram* base) { program = base; }
		Lexer &GetLexer() { return lexer; }

		void ParserStatements();
		
	protected:

		ASTreeExpression* Expression();
		ASTreeBreakStatement* BreakStatements();
		ASTreeReturnStatement* ReturnStatements();
		ASTreeWhileStatement* WhileStatements();
		ASTreeIfStatement* IfStatements();
		ASTreeExpression* VariableDefination();
		ASTreeStatements* Statements();
		Function* Defination();

	protected:
		Lexer lexer;
		BaseProgram *program;
		
	};

	void ParserASTreeUnit(wstring& file, Program& program);
}
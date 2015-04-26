#pragma once

#include <string>
#include <vector>
#include <set>
#include <stack>
#include <queue>
#include <map>
#include <initializer_list>

#include "lexer.h"

namespace ScriptCompile
{
	using std::wstring;
	using std::vector;
	using std::set;
	using std::stack;
	using std::queue;
	using std::map;
	using std::initializer_list;

	enum ASTKind
	{
		AST,
		AST_BINARY,
		AST_UNARY,
		AST_EXPR,
		AST_BREAK,
		AST_WHILE,
		AST_IF,
		AST_STATE,
		AST_RETURN,
		AST_LOGIC,
		AST_CALL,
		AST_VAR,
	};

	//enum ASTOper
	//{
	//	OPER_EQUAL = 0,
	//	OPER_OR,
	//	OPER_AND,
	//	OPER_ADD,
	//	OPER_SUB,
	//	OPER_MUL,
	//	OPER_DIV,
	//	OPER_MOD,
	//	OPER_NOT,
	//};

	struct Operator
	{
		int kind;
		int lever;
		int bit;
	};

	struct Variable
	{
		int kind;
		wstring value;
	};

	//
	//	语法树部分
	//
	class ASTree
	{
	public:
		ASTree() {}
		virtual ~ASTree() {}

		virtual int GetKind() const { return AST; }
	};

	class ASTreeVar : public ASTree
	{
	public:
		ASTreeVar() {}
		virtual ~ASTreeVar() {}

		virtual int GetKind() const { return AST_VAR; }

		int		 GetValueKind() const { return kind; }
		void	 SetValueKind(int k)  { kind = k; }
		void	 SetValue(wstring& t) { temp = t; }
		wstring& GetValue()			  { return temp; }
	private:
		int kind;
		wstring temp;
	};

	class ASTreeCall : public ASTree
	{
	public:
		ASTreeCall() {}
		virtual ~ASTreeCall() {}

		virtual int GetKind() const { return AST_CALL; }

		void	 AddParam(wstring& p) { params.push_back(p); }
		void	 SetName(wstring& n)  { name = n; }
		size_t	 Size()				  { return params.size(); }
		wstring& GetName()			  { return name; }
		wstring& GetParam(unsigned int pos) { return params.at(pos); }
	private:
		wstring name;
		vector<wstring> params;
	};

	class ASTreeUnary : public ASTree
	{
	public:
		ASTreeUnary() : exprssion(NULL) {}
		virtual ~ASTreeUnary() {}

		virtual int GetKind() const { return AST_UNARY; }

		void	SetExprssion(ASTree* l)	{ exprssion = l; }
		void	SetOperator(int o)		{ op = o; }
		int		GetOperator() const		{ return op; }
		ASTree* GetExprssion()			{ return exprssion; }
	private:
		int		op;
		ASTree* exprssion;
	};

	class ASTreeBinary : public ASTree
	{
	public:
		ASTreeBinary() : left(NULL), right(NULL) {}
		virtual ~ASTreeBinary() {}

		virtual int GetKind() const { return AST_BINARY; }

		int		GetOperator() const	{ return op; }
		void	SetLeft(ASTree* l)	{ left = l; }
		void	SetOperator(int o)	{ op = o; }
		void	SetRight(ASTree* r) { right = r; }
		ASTree* GetLeft()			{ return left; }
		ASTree* GetRight()			{ return right; }
	private:
		int		op;
		ASTree* left;
		ASTree* right;
	};

	//class ASTreeLogic : public ASTree
	//{
	//public:
	//	ASTreeLogic() {}
	//	virtual ~ASTreeLogic() {}

	//	virtual int GetKind() const { return AST_LOGIC; }

	//	int		GetOperator() const				{ return op; }
	//	void	SetOperator(int o)				{ op = o; }
	//	void	AddCondition(ASTree* t)			{ conditions.push_back(t); }
	//	size_t	Size()		 					{ return conditions.size(); }
	//	ASTree& GetCondition(unsigned int pos)  { return *conditions.at(pos); }
	//private:
	//	vector<ASTree*> conditions;
	//	int				op;
	//};

	class ASTreeExpression : public ASTree
	{
	public:
		ASTreeExpression() : binary(NULL) {}
		virtual ~ASTreeExpression() {}

		virtual int GetKind() const { return AST_EXPR; }

		void	SetBinary(ASTree* b) { binary = b; }
		ASTree* GetBinary()			 { return binary; }
	private:
		ASTree* binary;
	};

	class ASTreeBreak : public ASTree
	{
	public:
		ASTreeBreak() {}
		virtual ~ASTreeBreak() {}

		virtual int GetKind() const { return AST_BREAK; }
	};

	class ASTreeReturn : public ASTree
	{
	public:
		ASTreeReturn() : expression(NULL) {}
		virtual ~ASTreeReturn() {}

		virtual int GetKind() const { return AST_RETURN; }

		void SetExprssion(ASTreeExpression* e) { expression = e; }
		ASTreeExpression& GetExpression()	   { return *expression; }
	private:
		ASTreeExpression* expression;
	};

	class ASTreeStatements : public ASTree
	{
	public:
		ASTreeStatements() {}
		virtual ~ASTreeStatements() {}

		virtual int GetKind() const { return AST_STATE; }

		void InsertStatement(ASTree* s)	{ statements.push_back(s); }
		ASTree* GetStatement(int pos)	{ return statements.at(pos); }
		size_t GetSize() const			{ return statements.size(); }

		ASTreeStatements& operator += (ASTree* rls)
		{
			if (rls != nullptr)
				statements.push_back(rls);

			return *this;
		}

		ASTreeStatements& operator += (ASTreeStatements& rls)
		{
			for (auto i : rls.statements)
			{
				statements.push_back(i);
			}

			return *this;
		}
	private:
		vector<ASTree*> statements;
	};

	class ASTreeWhile : public ASTree
	{
	public:
		ASTreeWhile() : condition(NULL), statements(NULL) {}
		virtual ~ASTreeWhile() {}

		virtual int GetKind() const { return AST_WHILE; }

		void SetCondition(ASTreeExpression* c)	{ condition = c; }
		void SetStatements(ASTreeStatements* s) { statements = s; }
		ASTreeStatements& GetStatements()		{ return *statements; }
		ASTreeExpression& GetCondition()		{ return *condition; }
	private:
		ASTreeExpression* condition;
		ASTreeStatements* statements;
	};

	class ASTreeIf : public ASTree
	{
	public:
		ASTreeIf() : condition(NULL), hasElse(false), 
			ifStatements(), elseStatements(NULL) {}
		virtual ~ASTreeIf() {}

		virtual int GetKind() const { return AST_IF; }

		void SetCondition(ASTreeExpression* c)		{ condition = c; }
		void SetIfStatements(ASTreeStatements* s)	{ ifStatements = s; }
		void SetElseStatements(ASTreeStatements* s)	{ hasElse = true, elseStatements = s; }
		bool GetHasElse()							{ return hasElse; }
		ASTreeExpression& GetCondition()			{ return *condition; }
		ASTreeStatements& GetIfStatements()			{ return *ifStatements; }
		ASTreeStatements& GetElseStatements()		{ return *elseStatements; }
	private:
		bool			  hasElse;
		ASTreeExpression* condition;
		ASTreeStatements* ifStatements;
		ASTreeStatements* elseStatements;
	};

	// 
	// 程序描述部分
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

	class Function
	{
	public:
		Function() : statements(NULL) {}
		~Function() {}

		void AddParam(wstring& s)				{ params.push_back(s); AddIdentifier(s); }
		void SetStatements(ASTreeStatements* s) { statements = s; }
		void AddIdentifier(wstring& i)			{ identifier.insert(i); }
		bool FindIndetifier(wstring& i)			{ return identifier.find(i) != identifier.end(); }
		void SetFunctionName(wstring& n)		{ name = n; }
		wstring& GetFunctionName()				{ return name; }
		wstring& GetParamName(unsigned int pos) { return params.at(pos); }
		//Variables&	GetVariables()				{ return var; }
		set<wstring>& GetIdentifier()			{ return identifier; }
		unsigned int GetParamLength()			{ return params.size(); }
		ASTreeStatements& GetStatements()		{ return *statements; }
	private:
		wstring				name;
		ASTreeStatements*	statements;
		//Variables			var;
		set<wstring>		identifier;
		vector<wstring>		params;
	};

	class Plugin
	{
	public:
		Plugin() {}

		Variable Call(vector<wstring>& params)
		{
			if (length != -1 && params.size() != length)
				throw wstring(L"调用函数 " + name + L"时参数数量不对");

			return Execute(params);
		}
		
		wstring& GetName()  { return name; }
		virtual Variable Execute(vector<wstring>& params) = 0;
	protected:
		int length;
		wstring name;

	};

	class Program
	{
	public:
		Program() : statements(nullptr) {}
		~Program() {}

		void SetStatements(ASTreeStatements* s) 
		{ 
			if (statements == nullptr)
				statements = s;
			else
				*statements += *s; 
		}

		void AddFunction(Function* f)			
		{ 
			functions[f->GetFunctionName()] = f; 
			AddIdentifier(f->GetFunctionName()); 
		}

		bool FindFunction(wstring& name) const
		{
			return functions.find(name) != functions.end();
		}

		void AddPlugin(Plugin* p)
		{
			AddIdentifier(p->GetName());
			plugin[p->GetName()] = p;
		}

		bool FindPlugin(wstring& name)			{ return plugin.find(name) != plugin.end(); }
		Plugin& GetPlugin(wstring& name)		{ return *plugin.find(name)->second; }
		void AddIdentifier(wstring& i)			{ identifier.insert(i); }
		bool FindIndetifier(wstring& i)	const	{ return identifier.find(i) != identifier.end(); }
		//Function&	GetFunction(unsigned int pos){ return *functions.at(pos); }
		//unsigned int GetFunctionLength()		{ return functions.size(); }
		Function* GetFunction(wstring& name)	{ return functions.find(name)->second; }
		set<wstring>& GetIdentifier()			{ return identifier; }
		ASTreeStatements& GetStatements()		{ return *statements; }

	private:
		set<wstring>			identifier;
		ASTreeStatements*		statements;
		map<wstring, Function*>	functions;
		map<wstring, Plugin*>	plugin;
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
	
	ASTreeExpression*	Expression				(Lexer& lexer);
	ASTreeReturn*		BreakOrReturnStatements	(Lexer& lexer);
	ASTreeWhile*		WhileStatements			(Lexer& lexer, set<wstring>& iden);
	ASTreeIf*			IfStatements			(Lexer& lexer, set<wstring>& iden);
	ASTreeStatements*	Statements				(Lexer& lexer, set<wstring>& iden);
	Function*			Defination				(Lexer& lexer, set<wstring>& iden);
	void				Parser					(wstring& file, Program& program);
}
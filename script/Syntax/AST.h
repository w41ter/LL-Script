#pragma once

#include <string>
#include <vector>
#include <map>

namespace ScriptCompile
{
	using std::wstring;
	using std::vector;
	using std::map;

	enum ASTKind
	{
		AST_BASE,
		AST_UNARY,
		AST_LOGIC,
		AST_VALUE,
		AST_BINARY,
		AST_EXPRESSION,
		AST_STATEMENTS,
		AST_IF_STATEMENT,
		AST_FUNCTION_CALL,
		AST_BREAK_STATEMENT,
		AST_WHILE_STATEMENT,
		AST_RETURN_STATEMENT,
	};

	enum Type
	{
		T_REAL,
		T_STRING,
		T_IDENTIFIER,
	};

	struct Operator
	{
		int kind;
		int lever;
		int bit;
	};

	//
	//	语法树部分
	//
	class ASTree
	{
	public:
		ASTree() {}
		virtual ~ASTree() {}

		virtual int GetKind() const { return AST_BASE; }
	};

	class ASTreeVariable : public ASTree
	{
	public:
		ASTreeVariable() {}
		virtual ~ASTreeVariable() {}

		virtual int GetKind() const { return AST_VALUE; }

		int		 GetValueKind() const { return kind; }
		void	 SetValueKind(int k)  { kind = k; }
		void	 SetValue(const wstring& t) { temp = t; }
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

		virtual int GetKind() const { return AST_FUNCTION_CALL; }

		void	 AddParam(const wstring& p) { params.push_back(p); }
		void	 SetName(const wstring& n)  { name = n; }
		size_t	 Size()				  { return params.size(); }
		wstring& GetName()			  { return name; }
		wstring& GetParam(unsigned int pos) { return params.at(pos); }
		vector<wstring>& GetParams() { return params; }
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
		void	SetOperator(Operator o)		{ op = o; }
		Operator GetOperator() const		{ return op; }
		ASTree* GetExprssion()			{ return exprssion; }
	private:
		Operator op;
		ASTree* exprssion;
	};

	class ASTreeBinary : public ASTree
	{
	public:
		ASTreeBinary() : left(NULL), right(NULL) {}
		virtual ~ASTreeBinary() {}

		virtual int GetKind() const { return AST_BINARY; }

		Operator GetOperator() const	{ return op; }
		void	SetLeft(ASTree* l)	{ left = l; }
		void	SetOperator(Operator o)	{ op = o; }
		void	SetRight(ASTree* r) { right = r; }
		ASTree* GetLeft()			{ return left; }
		ASTree* GetRight()			{ return right; }
	private:
		Operator op;
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

		virtual int GetKind() const { return AST_EXPRESSION; }

		void	SetBinary(ASTree* b) { binary = b; }
		ASTree* GetBinary()			 { return binary; }
	private:
		ASTree* binary;
	};

	class ASTreeStatements : public ASTree
	{
	public:
		ASTreeStatements() {}
		virtual ~ASTreeStatements() {}

		virtual int GetKind() const { return AST_STATEMENTS; }

		void InsertStatement(ASTreeStatements& rls);
		void InsertStatement(ASTree* s)	{ statements.push_back(s); }
		ASTree* GetStatement(int pos)	{ return statements.at(pos); }
		size_t GetSize() const			{ return statements.size(); }

	private:
		vector<ASTree*> statements;
	};

	class ASTreeBreakStatement : public ASTree
	{
	public:
		ASTreeBreakStatement() {}
		virtual ~ASTreeBreakStatement() {}

		virtual int GetKind() const { return AST_BREAK_STATEMENT; }
	};

	// 返回值处应当为expresion
	// 所以没有使用statement
	class ASTreeReturnStatement : public ASTree
	{
	public:
		ASTreeReturnStatement() : expression(NULL) {}
		virtual ~ASTreeReturnStatement() {}

		virtual int GetKind() const { return AST_RETURN_STATEMENT; }

		void SetExprssion(ASTreeExpression* e) { expression = e; }
		ASTreeExpression& GetExpression()	   { return *expression; }
	private:
		ASTreeExpression* expression;
	};

	class ASTreeWhileStatement : public ASTree
	{
	public:
		ASTreeWhileStatement() : condition(nullptr), statements(nullptr) {}
		virtual ~ASTreeWhileStatement() {}

		virtual int GetKind() const { return AST_WHILE_STATEMENT; }

		void SetCondition(ASTreeExpression* c)	{ condition = c; }
		void SetStatements(ASTreeStatements* s) { statements = s; }
		ASTreeStatements& GetStatements()		{ return *statements; }
		ASTreeExpression& GetCondition()		{ return *condition; }
	private:
		ASTreeExpression* condition;
		ASTreeStatements* statements;
	};

	// if statements
	// 管理 if statement 和 else statement
	// 保存condition
	class ASTreeIfStatement : public ASTree
	{
	public:
		ASTreeIfStatement() : condition(nullptr), hasElse(nullptr), elseStatements(nullptr) {}
		virtual ~ASTreeIfStatement() {}

		virtual int GetKind() const { return AST_IF_STATEMENT; }

		void SetCondition(ASTreeExpression* c)		{ condition = c; }
		void SetIfStatements(ASTreeStatements* s)	{ ifStatements = s; }
		void SetElseStatements(ASTreeStatements* s)	{ hasElse = true, elseStatements = s; }
		bool HasElse()								{ return hasElse; }
		ASTreeExpression& GetCondition()			{ return *condition; }
		ASTreeStatements& GetIfStatements()			{ return *ifStatements; }
		ASTreeStatements& GetElseStatements()		{ return *elseStatements; }
	private:
		bool			  hasElse;
		ASTreeExpression* condition;
		ASTreeStatements* ifStatements;
		ASTreeStatements* elseStatements;
	};
}
#include "Interpret.h"
#include <algorithm> 

namespace ScriptCompile
{
	bool Condition(Variable& slove, unordered_map<wstring, wstring>& envir);
	wstring& GetValue(wstring& name, unordered_map<wstring, wstring>& envir);
	Variable Caculate(Variable& left, Variable& right, int kind, Operator op);
	Variable Caculate(Variable left, Variable right, int op, unordered_map<wstring, wstring>& envir);
	Variable EvalExpression(ASTreeExpression& expression, unordered_map<wstring, wstring>& envir);
	ReturnValue RunStatements(ASTreeStatements& statements, unordered_map<wstring, wstring>& envir);
	Variable RunFunction(Function* function, unordered_map<wstring, wstring>& envir);

	unordered_map<wstring, wstring> g_envir, empty;
	Program* g_program;
	int g_InFunction = 0;

	map<int, OpFunction> OperatorEval = {
		{ OP_NOT_EQUAL, [](double l, double r) -> double {
			return l != r;
		} },

		{ OP_NOT_EQUAL, [](double l, double r) -> double {
			return l != r;
		} },

		{ OP_LEFT_RIGHT, [](double l, double r) -> double {
			return l == r;
		} },

		{ OP_LEFT_ARROW, [](double l, double r) -> double {
			return l < r;
		} },

		{ OP_RIGHT_ARROW, [](double l, double r) -> double {
			return l > r;
		} },

		{ OP_LEFT_ARROW_EQUAL, [](double l, double r) -> double {
			return l <= r;
		} },

		{ OP_RIGHT_ARROW_EQUAL, [](double l, double r) -> double {
			return l >= r;
		} },

		{ OP_STAR, [](double l, double r) -> double {
			return l * r;
		} },

		{ OP_PLUS, [](double l, double r) -> double {
			return l + r;
		} },

		{ OP_MINUS, [](double l, double r) -> double {
			return l - r;
		} },

		{ OP_SLASH, [](double l, double r) -> double {
			if (r == 0.0)
				throw wstring(L"除数不能为零");
			return l / r;
		} },
	};

	bool IsInt(const wstring& String)
	{
		const wchar_t* Buffer = String.c_str();
		wcstol(Buffer, const_cast<wchar_t**>(&Buffer), 10);
		return *Buffer == L'\0';
	}

	bool IsReal(const wstring& String)
	{
		const wchar_t* Buffer = String.c_str();
		wcstod(Buffer, const_cast<wchar_t**>(&Buffer));
		return *Buffer == L'\0';
	}

	bool IsStr(const wstring& String)
	{
		const wchar_t* Buffer = String.c_str();
		while (*Buffer != L'\0')
		{
			if (iswalpha(*Buffer))
				return true;
			Buffer++;
		}
		return false;
	}

	int ToInt(const wstring& String)
	{
		const wchar_t* Buffer = String.c_str();
		return wcstol(Buffer, const_cast<wchar_t**>(&Buffer), 10);
	}

	double ToReal(const wstring& String)
	{
		const wchar_t* Buffer = String.c_str();
		return wcstod(Buffer, const_cast<wchar_t**>(&Buffer));
	}

	wstring ToStr(int Value)
	{
		wchar_t Buffer[100];
		_itow_s(Value, Buffer, 10);
		return Buffer;
	}

	wstring ToStr(double Value)
	{
		char AnsiBuffer[100];
		wchar_t Buffer[100];
		sprintf(AnsiBuffer, "%lf", Value);
		mbstowcs(Buffer, AnsiBuffer, 100);
		return Buffer;
	}

	bool Condition(Variable& slove, unordered_map<wstring, wstring>& envir)
	{
		if (slove.kind == TK_STRING)
		{
			throw wstring(L"条件表达式不能为字符串");
		}

		if (slove.kind == TK_IDENTIFIER) 
		{
			wstring value = GetValue(slove.value, envir);

			if (IsStr(value))
			{
				throw wstring(L"条件表达式不能为字符串");
			}
			else// if (IsReal(value))
			{
				slove.kind = TK_REAL;
				return ToReal(value);
			}
			/*else
			{
				slove.kind = TK_INTEGER;
				return ToInt(value);
			}*/
		}

		return ToReal(slove.value);

		//if (slove.kind == TK_REAL)
		//	return ToReal(slove.value);
		//else
		//	return ToInt(slove.value);
	}

	inline Variable ConstructVariable(wstring& value)
	{
		if (IsStr(value))
			return{ TK_STRING, value };
		else //if (IsReal(value))
			return{ TK_REAL, value };
		//else
			//return{ TK_INTEGER, value };
	}

	inline wstring& GetValue(wstring& name, unordered_map<wstring, wstring>& envir)
	{
		if (envir.find(name) != envir.end())
			return envir.find(name)->second;
		else
			return g_envir.find(name)->second;
	}

	Variable Caculate(Variable& left, Variable& right, OpFunction op)
	{
		double slove = op(ToReal(left.value), ToReal(right.value));
		return{ TK_REAL, ToStr(slove) };
	}

	Variable Caculate(Variable left, Variable right, int op, unordered_map<wstring, wstring>& envir)
	{
		if (op == OP_EQUAL)
		{
			if (left.kind != TK_IDENTIFIER)
				throw wstring(L" = 左边必须为变量名");

			if (right.kind == TK_IDENTIFIER)
				right.value = GetValue(right.value, envir);

			return ConstructVariable(GetValue(left.value, envir) = right.value);
		}

		if (left.kind == TK_IDENTIFIER)
			left = ConstructVariable(GetValue(left.value, envir));
		if (right.kind == TK_IDENTIFIER)
			right = ConstructVariable(GetValue(right.value, envir));
		int kind = std::max(right.kind, left.kind);

		if (kind == TK_STRING)
		{
			if (op == OP_PLUS)
				return{ TK_REAL, left.value + right.value };

			bool flag = false;
			if (op == OP_LEFT_RIGHT)
				flag = (left.value == right.value);
			else if (op == OP_NOT_EQUAL)
				flag = !(left.value == right.value);
			else
				throw wstring(L"字符串只支持 +、!=、== 和 = 符号");

			if (flag)
				return{ TK_REAL, L"1" };
			else
				return{ TK_REAL, L"0" };
		}
		else
		{
			if (OperatorEval.find(op) != OperatorEval.end())
				return Caculate(left, right, OperatorEval[op]);
			else
				throw wstring(L"不认识的符号");
		}
	}


	Variable EvalBinary(ASTreeBinary* binary, unordered_map<wstring, wstring>& envir)
	{
		ASTreeExpression binaryExpression;

		if (binary->GetOperator().kind == OP_VERTICAL)
		{
			binaryExpression.SetBinary(binary->GetLeft());
			Variable left = EvalExpression(binaryExpression, envir);
			bool open = Condition(left, envir);
			if (open)
			{
				return{ TK_REAL, L"1" };
			}
			else
			{
				binaryExpression.SetBinary(binary->GetRight());
				left = EvalExpression(binaryExpression, envir);
				open = Condition(left, envir);
				if (open)
					return{ TK_REAL, L"1" };
				else
					return{ TK_REAL, L"0" };
			}
		}
		else if (binary->GetOperator().kind == OP_AND)
		{
			binaryExpression.SetBinary(binary->GetLeft());
			Variable left = EvalExpression(binaryExpression, envir);
			bool open = Condition(left, envir);
			if (open)
			{
				binaryExpression.SetBinary(binary->GetRight());
				left = EvalExpression(binaryExpression, envir);
				open = Condition(left, envir);
				if (open)
					return{ TK_REAL, L"1" };
				else
					return{ TK_REAL, L"0" };
			}
			else
			{
				return{ TK_REAL, L"0" };
			}
		}
		else 
		{
			binaryExpression.SetBinary(binary->GetLeft());
			Variable left = EvalExpression(binaryExpression, envir);
			binaryExpression.SetBinary(binary->GetRight());
			Variable right = EvalExpression(binaryExpression, envir);

			return Caculate(left, right, binary->GetOperator().kind, envir);
		}
	}

	Variable EvalExpression(ASTreeExpression& expression, unordered_map<wstring, wstring>& envir)
	{
		switch (expression.GetBinary()->GetKind())
		{
		case AST_BINARY:
			{
				ASTreeBinary* binary = static_cast<ASTreeBinary*>(expression.GetBinary());

				return EvalBinary(binary, envir);
			}
			
		case AST_UNARY:
			{
				ASTreeUnary* unary = static_cast<ASTreeUnary*>(expression.GetBinary());
				throw wstring(L"暂时不提供单目运算");
				break;
			}
			
		case AST_VALUE:
			{
				ASTreeVariable* var = static_cast<ASTreeVariable*>(expression.GetBinary());
				
				return{ var->GetValueKind(), var->GetValue() };
			}
			
		case AST_FUNCTION_CALL:
			{
				ASTreeCall* call = static_cast<ASTreeCall*>(expression.GetBinary());
			
				if (!g_program->FindFunction(call->GetName()))
				{
					vector<wstring> params;
					for (size_t i = 0; i < call->Size(); i++)
						params.push_back(GetValue(call->GetParam(i), envir));

					return g_program->GetPlugin().Call(call->GetName(), params);
				}
					
				Function* function = g_program->GetFunction(call->GetName());

				unordered_map<wstring, wstring> temp;
				unordered_set<wstring>& identifier = function->GetVariable();
				for (const auto i : identifier)
					temp[i] = L"";

				// 初始化参数值
				for (size_t i = 0; i < call->Size(); i++)
					temp[function->GetParams()[i]] = GetValue(call->GetParam(i), envir);

				return RunFunction(function, temp);
			}
		case AST_EXPRESSION:
			{
				// 特殊情况
				ASTreeExpression* expr = static_cast<ASTreeExpression*>(expression.GetBinary());
				return EvalExpression(*expr, envir);
			}
			
		default:
			throw wstring(L"我也不知道是什么");
		}
	}

	ReturnValue RunStatements(ASTreeStatements& statements, unordered_map<wstring, wstring>& envir)
	{
		for (size_t i = 0; i < statements.GetSize(); i++)
		{
			ASTree* tree = statements.GetStatement(i);
			switch (tree->GetKind())
			{
			case AST_IF_STATEMENT:
				{
					ASTreeIfStatement* astIf = static_cast<ASTreeIfStatement*>(tree);
					Variable slove = EvalExpression(astIf->GetCondition(), envir);
					bool open = Condition(slove, envir);

					if (open)
					{
						ReturnValue var = RunStatements(astIf->GetIfStatements(), envir);
						if (var.kind == TK_RETURN || var.kind == TK_BREAK)
							return var;
					}
					else if (astIf->HasElse())
					{
						ReturnValue var = RunStatements(astIf->GetElseStatements(), envir);
						if (var.kind == TK_RETURN || var.kind == TK_BREAK)
							return var;
					}
					break;
				}

			case AST_WHILE_STATEMENT:
				{
					ASTreeWhileStatement* astWhile = static_cast<ASTreeWhileStatement*>(tree);
					Variable slove = EvalExpression(astWhile->GetCondition(), envir);
					while (Condition(slove, envir))
					{
						ReturnValue var = RunStatements(astWhile->GetStatements(), envir);
						if (var.kind == TK_BREAK)
							break;
						else if (var.kind == TK_RETURN)
							return var;

						slove = EvalExpression(astWhile->GetCondition(), envir);
					}
					break;
				}

			case AST_BREAK_STATEMENT:
				{
					ReturnValue value = { TK_BREAK, { TK_BREAK, L"break" } };
					return value;
				}

			case AST_RETURN_STATEMENT:
				{
					if (g_InFunction == 0)
						throw wstring(L"企图在函数外使用 return");
				
					ASTreeReturnStatement* astReturn = static_cast<ASTreeReturnStatement*>(tree);

					// 将EvalExpression返回的变量转换成对应的值类型
					Variable var = EvalExpression(astReturn->GetExpression(), envir);
					if (var.kind == TK_IDENTIFIER)
					{
						var.value = GetValue(var.value, envir);
						var = ConstructVariable(var.value);
					}
					return{ TK_RETURN, var };
				}

			case AST_EXPRESSION:
				{
					ASTreeExpression* expression = static_cast<ASTreeExpression*>(tree);
					EvalExpression(*expression, envir);
					break;
				}

			default:
				break;
			}
			//2ToStr(123);
		}
		return{ TK_END, { TK_END, L"end" } };
	}

	Variable RunFunction(Function* function, unordered_map<wstring, wstring>& envir)
	{
		++g_InFunction;
		ReturnValue var = RunStatements(function->GetStatements(), envir);
		if (var.kind == TK_BREAK)
			throw wstring(L"break 没有找到对应的 while 循环");
		--g_InFunction;
		return var.var;
	}

	void Run(Program* pro)
	{
		g_program = pro;

		unordered_set<wstring>& identifier = g_program->GetVariable();
		for (auto i : identifier)
			g_envir[i] = L"";
			
		ASTreeStatements& states = g_program->GetStatements();
		ReturnValue var = RunStatements(states, empty);
		if (var.kind == TK_BREAK)
			throw wstring(L"break 没有找到对应的 while 循环");
	}
}
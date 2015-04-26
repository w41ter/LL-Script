
#include "lexer.h"

namespace ScriptCompile
{
	void Lexer::SetProgram(wstring& str)
	{
		program = str;
		position = index = program.begin();
		line = 1;
	}
	
	void Lexer::Begin()
	{
		position = index;
	}
	
	void Lexer::End() 
	{
		index = position;
	}
	
	Token Lexer::Get()
	{
		WhiteSpace();
		
		if (index != program.end())
		{
			if (isalpha(*index) || *index == '_')
			{
				return Identifier();
			}
			else if (isdigit(*index))
			{
				return Number();
			}
			else if (*index == '"')
			{
				return String();
			}
			else
			{
				return Sign();
			}
		}
		else
		{
			Token token = {TK_EOF, line, L"EOF"};
			return token;
		}
	}
	
	void Lexer::WhiteSpace()
	{
		while (index != program.end())
		{
			if (isspace(*index))
			{
				if (*index == '\n')
				{
					line++;
				}
				++index;
			}
			else
			{
				break;
			}
		}
	}

	void Lexer::Comments()
	{
		while (index != program.end())
		{
			if (*index != '\n')
			{
				index++;
			}
			else
			{
				line++;
				break;
			}
		}
	}
	
	Token Lexer::String()
	{
		++index;
		Token token;
		token.kind = TK_STRING;
		token.line = line;
		//token.value += *index++;
		
		while (index != program.end())
		{
			
			if (*index == L'"')
			{
				++index;			// 保证指向下一个位置
				return token;
			}
			else if (*index == L'\n')
			{
				line++;
			}
			token.value += *index;
			++index;
		}
		
		token.kind = TK_ERROR;
		token.value = L"引号没有闭合";
		return token;
	}
	
	Token Lexer::Number()
	{
		Token token;
		token.kind = TK_INTEGER;
		token.line = line;
		
		while (index != program.end() 
			&& (isdigit(*index) || *index == L'.'))
		{
			token.value += *index;
			++index;
		}
		
		int count = 0;
		for (auto c : token.value)
		{
			if (c == L'.')
			{
				count++;
			}
		}
		
		if (count == 1 && *(index - 1) != L'.')
		{
			token.kind = TK_REAL;
		}
		else if (count != 0)
		{
			token.kind = TK_ERROR;
			token.value = L"实数写错了";
		}
		
		return token;
	}
	
	Token Lexer::Identifier()
	{
		Token token;
		token.kind = TK_IDENTIFIER;
		token.line = line;
		token.value += *index;
		
		++index;
		while (index != program.end())
		{
			if (isalnum(*index) || *index == L'_')
			{
				token.value += *index;
				++index;
			}
			else break;
		}
		
		if (wcscmp(token.value.c_str(), L"if") == 0)
		{
			token.kind = TK_IF;
		}
		else if (wcscmp(token.value.c_str(), L"while") == 0)
		{
			token.kind = TK_WHILE;
		}
		else if (wcscmp(token.value.c_str(), L"else") == 0)
		{
			token.kind = TK_ELSE;
		}
		else if (wcscmp(token.value.c_str(), L"break") == 0)
		{
			token.kind = TK_BREAK;
		}
		else if (wcscmp(token.value.c_str(), L"return") == 0)
		{
			token.kind = TK_RETURN;
		}
		else if (wcscmp(token.value.c_str(), L"var") == 0)
		{
			token.kind = TK_VAR;
		}
		else if (wcscmp(token.value.c_str(), L"function") == 0)
		{
			token.kind = TK_FUNCTION;
		}
		
		return token;
	}
	
	Token Lexer::Sign()
	{
		Token token;
		token.line = line;
		token.value = *index;
		
		switch (*index)
		{
		case L'|':
			token.kind = TK_OR;
			break;
		case L'&':
			token.kind = TK_AND;
			break;
		case L'+':
			token.kind = TK_ADD;
			break;
		case L'-':
			token.kind = TK_SUB;
			break;
		case L'*':
			token.kind = TK_MUL;
			break;
		case L'/':
			if (*(index + 1) == L'/')
			{
				Comments();
				return Get();
			}
			token.kind = TK_DIV;
			break;
		case L'%':
			token.kind = TK_MOD;
			break;
		case L'!':
			token.kind = TK_NOT;
			break;
		case L'(':
			token.kind = TK_LBRA;
			break;
		case L')':
			token.kind = TK_RBRA;
			break;
		case L'{':
			token.kind = TK_LBRACE;
			break;
		case L'}':
			token.kind = TK_RBRACE;
			break;
		case L';':
			token.kind = TK_END;
			break;
		case L'=':
			token.kind = TK_EQUAL;
			break;
		case L',':
			token.kind = TK_COMMA;
			break;
		default:
			token.kind = TK_ERROR;
			token.value = L"不认识的符号：";
			token.value += *index;
		}
		 
		++index;

		return token;
	}
}


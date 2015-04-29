
#include "lexer.h"

namespace ScriptCompile
{
	void Lexer::SetProgram(wstring& str)
	{
		program = str;
		position = index = program.begin();
		line = 1;
	}
	
	void Lexer::TakeNotes()
	{
		position = index;
	}
	
	void Lexer::Restore() 
	{
		index = position;
	}
	
	Token Lexer::GetNextToken()
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
		
		if (keywords.find(token.value) != keywords.end())
			token.kind = keywords[token.value];
		
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
			token.kind = TK_VERTICAL;
			break;
		case L'&':
			token.kind = TK_AND;
			break;
		case L'+':
			token.kind = TK_PLUS;
			break;
		case L'-':
			token.kind = TK_MINUS;
			break;
		case L'*':
			token.kind = TK_STAR;
			break;
		case L'/':
			if (*(index + 1) == L'/')
			{
				Comments();
				return GetNextToken();
			}
			token.kind = TK_SLASH;
			break;
		case L'%':
			token.kind = TK_PERCENT;
			break;
		case L'!':
			if (*(index + 1) == L'=')
				token.kind = TK_NOT_EQUAL, ++index;
			else
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

		case L'<':
			if (*(index + 1) == L'=')
				token.kind = TK_LEFT_ARROW_EQUAL, ++index;
			else
				token.kind = TK_LEFT_ARROW;
			break;
		case L'>':
			if (*(index + 1) == L'=')
				token.kind = TK_RIGHT_ARROW_EQUAL, ++index;
			else
				token.kind = TK_RIGHT_ARROW;
			break;
		default:
			token.kind = TK_ERROR;
			token.value = L"不认识的符号：";
			token.value += *index;
		}
		 
		++index;

		return token;
	}

	void Lexer::Initializer()
	{
		keywords[L"if"] = TK_IF;
		keywords[L"while"] = TK_WHILE;
		keywords[L"else"] = TK_ELSE;
		keywords[L"break"] = TK_BREAK;
		keywords[L"return"] = TK_RETURN;
		keywords[L"var"] = TK_VARIABLE;
		keywords[L"function"] = TK_FUNCTION;
	}
}


#pragma once

#include <cctype>
#include <string>

namespace ScriptCompile
{
	using std::wstring;
	enum TK
	{
		TK_NONE,
		TK_INTEGER,
		TK_REAL,
		TK_STRING,
		TK_IDENTIFIER,
		TK_VAR,
		TK_FUNCTION,
		TK_IF,
		TK_ELSE,
		TK_WHILE,
		TK_BREAK,
		TK_RETURN,
		TK_OR,
		TK_AND,
		TK_ADD,
		TK_SUB,
		TK_MUL,
		TK_DIV,
		TK_MOD,
		TK_NOT,
		TK_LBRA,
		TK_RBRA,
		TK_LBRACE,
		TK_RBRACE,
		TK_EQUAL,
		TK_END,
		TK_EOF,
		TK_ERROR,
		TK_COMMA,
	};
	
	struct Token
	{
		int 	kind;
		int 	line;
		wstring value;
	};
	
	class Lexer
	{
	public:
		Lexer() = default;
		Lexer(wstring &str) : program(str) 
		{
			index = position = program.begin();
			line = 1;
		}
		~Lexer() { }
		
		// 对外公开接口
		void 	SetProgram(wstring& str);
		void 	Begin();
		void 	End();
		Token 	Get();
		
	private:
		void 	WhiteSpace();
		void	Comments();
		Token  	String();
		Token 	Number();
		Token 	Identifier();
		Token 	Sign();
		
	private:
		unsigned int 		line;
		wstring 			program;
		wstring::iterator 	position;
		wstring::iterator 	index;
	};
}

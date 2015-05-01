#pragma once

#include <cctype>
#include <string>
#include <unordered_map>

namespace ScriptCompile
{
	using std::wstring;
	using std::unordered_map;

	enum Tokens
	{
		TK_NONE,

		// type
		TK_INTEGER,
		TK_REAL,
		TK_STRING,
		TK_IDENTIFIER,

		// keywords
		TK_VARIABLE,
		TK_FUNCTION,
		TK_IF,
		TK_ELSE,
		TK_WHILE,
		TK_BREAK,
		TK_RETURN,

		// operator
		TK_VERTICAL,
		TK_AND,
		TK_PLUS,
		TK_MINUS,
		TK_STAR,
		TK_SLASH,
		TK_PERCENT,
		TK_NOT,
		TK_EQUAL,
		TK_NOT_EQUAL,
		TK_LEFT_ARROW,
		TK_RIGHT_ARROW,
		TK_LEFT_ARROW_EQUAL,
		TK_RIGHT_ARROW_EQUAL,
		TK_LEFT_RIGHT,

		// others sign
		TK_LBRA,
		TK_RBRA,
		TK_LBRACE,
		TK_RBRACE,
		TK_END,
		TK_COMMA,
		TK_EOF,
		TK_ERROR,
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
		Lexer() { Initializer(); }
		Lexer(wstring &str) : program(str) 
		{
			Initializer();
			index = position = program.begin();
			line = 1;
		}
		~Lexer() { }
		
		// 对外公开接口
		void 	SetProgram(wstring& str);
		void 	TakeNotes();
		void 	Restore();
		Token 	GetNextToken();
		
	private:
		void	Initializer();
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
		unordered_map<wstring, Tokens> keywords;
	};
}

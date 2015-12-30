#include <iostream>

#include "lexer.h"

using std::string;

namespace script
{
    char Lexer::escapeChar(char c)
    {
        switch (c)
        {
        case 'n': c = '\n'; break;
        case 'r': c = '\r'; break;
        case 't': c = '\t'; break;
        case '\\': c = '\\'; break;
        case '"': c = '"';  break;
        case '\'': c = '\''; break;
        default:
            throw std::runtime_error("unexcepted excape char");
            break;
        }
        return c;
    }

    Token Lexer::readIdentifier(char startChar) 
    {
        char c = startChar;
        char id[128] = { 0 };
        size_t len = 0;
        while (isalpha(c) || isdigit(c) || c == '_') 
        {
            if (len > 126)
                throw std::runtime_error("length of identifier must in 1~126");
            id[len++] = c;
            c = lookChar();
        };
        file_.unget();
        unsigned short tok = getKeywordsID(id);
        return Token(tok, coord_, id);
    }
	
    Token Lexer::readToken()
    {
        whiteSpace();
        char ch = lookChar();
        if (!ch)
            return Token();
        if (isalpha(ch) || ch == '_')
            return readIdentifier(ch);
        if (isdigit(ch))
            return readDigit(ch);
        if (ch == '"')
            return readString();
        if (ch == '\'')
            return readChar();
        return readSign(ch);
    }
	
    unsigned short Lexer::getKeywordsID(const char * name)
    {
        if (keywords_.count(name) == 0)
            return TK_Identifier;
        else
            return keywords_[name];
    }

    void Lexer::whiteSpace()
	{
        char startChar = lookChar();
        while (startChar)
        {
            // std::cout << "find char " << startChar <<  std::endl;
            if (!isspace(startChar))
            {
                unget();
                break;
            }
            if (startChar == '\n')
            {
                coord_.lineNum_++;
                coord_.linePos_ = 0;
            }
            startChar = lookChar();
        }
	}

	void Lexer::readComments()
	{
        char c = lookChar();
		while (c)
		{
            if (c == '\n')
            {
                coord_.lineNum_++;
                coord_.linePos_ = 0;
            }
            c = lookChar();
		}
	}
	
    Token Lexer::readChar()
    {
        char c = lookChar();
        if (!c)
            throw std::runtime_error("以外结束文件查找");
        if (c == '\\')
        {
            c = lookChar();
            if (!c)
                throw std::runtime_error("以外结束文件查找");
            c = escapeChar(c);
        }
        char tmp = lookChar();
        if (tmp != '\'')
            throw std::runtime_error("缺少 ' ");
        return Token(TK_LitCharacter, coord_, string(1, c).c_str());
    }

    Token Lexer::readString()
	{
        string value;
        char c = lookChar();
        while (c)
        {
            if (c == '"')
            {
                return Token(TK_LitString, coord_, value.c_str());
            }
            if (c == '\\')
            {
                c = lookChar();
                if (!c) break;
                c = escapeChar(c);
            }
            value += c;
            c = lookChar();
        }
        throw std::runtime_error("以外结束文件查找！");
	}
	
	Token Lexer::readDigit(char startChar)
    {	
        int value = 0;
        while (isdigit(startChar))
        {
            value *= 10;
            value += startChar - '0';
            startChar = lookChar();
        }
        if (startChar != '.')
        {
            unget();
            return Token(coord_, value);
        }
        startChar = lookChar();
        float fnum = value;
        value = 0;
        while (isdigit(startChar))
        {
            value *= 10;
            value += startChar - '0';
            startChar = lookChar();
        }
        unget();
        float tmp = value;
        while (tmp > 1) tmp /= 10;
        fnum += tmp;
        return Token(coord_, fnum);
	}

	Token Lexer::readSign(char startChar)
	{
		switch (startChar)
		{
        case '(': return Token(TK_LParen, coord_);
        case ')': return Token(TK_RParen, coord_);
        case '[': return Token(TK_LSquareBrace, coord_);
        case ']': return Token(TK_RSquareBrace, coord_);
        case '{': return Token(TK_LCurlyBrace, coord_);
        case '}': return Token(TK_RCurlyBrace, coord_);
        case '+': return Token(TK_Plus, coord_);
        case '-': return Token(TK_Sub, coord_);
        case '*': return Token(TK_Mul, coord_);
        case '/': return Token(TK_Div, coord_);
        case ';': return Token(TK_Semicolon, coord_);
        case ',': return Token(TK_Comma, coord_);
        case '.': return Token(TK_Period, coord_);
        case '|': return Token(TK_Or, coord_);
        case '&': return Token(TK_And, coord_);
        case '!': 
        {
            if (lookChar() == '=')
                return Token(TK_NotEqualThan, coord_);
            unget();
            return Token(TK_Not, coord_);
        }
        case '=': 
        {
            if (lookChar() == '=')
                return Token(TK_EqualThan, coord_);
            unget();
            return Token(TK_Assign, coord_);
        }
        case '<': 
        {
            if (lookChar() == '=')
                return Token(TK_LessThan, coord_);
            unget();
            return Token(TK_Less, coord_);
        }
        case '>':
        {
            if (lookChar() == '=')
                return Token(TK_GreatThan, coord_);
            unget();
            return Token(TK_Great, coord_);
        }
		}
        throw std::runtime_error("bad char");
	}

    Token Lexer::getToken()
    {
        if (tokens_.size() == 0)
            return readToken();
        Token tok = tokens_[0];
        tokens_.pop_front();
        return tok;
    }

    Token Lexer::lookAhead(unsigned num)
    {
        if (num > tokens_.size())
        {
            tokens_.push_back(readToken());
        }
        return tokens_[num - 1];
    }

    void Lexer::registerKeyword(const string & str, unsigned tok)
    {
        keywords_.insert(std::pair<string, unsigned>(str, tok));
    }
}


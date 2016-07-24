#include "lexer.h"

#include "../Semantic/DiagnosisConsumer.h"
#include <iostream>
#include <cctype>

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
        {
            Diagnosis diag(DiagType::DT_Error, previousCoord_);
            diag << "unexcepted escape character" << c;
            diag_.diag(diag);
        }
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
            {
                Diagnosis diag(DiagType::DT_Error, previousCoord_);
                diag << "length of identifier must in 1~126";
                diag_.diag(diag);
                break;
            }
            id[len++] = c;
            c = lookChar();
        };
        file_.unget();
        unsigned short tok = getKeywordsID(id);
        return Token(tok, previousCoord_, id);
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
        if (ch == '#')
        {
            readComments();
            return readToken();
        }
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
        previousCoord_ = coord_;
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
                return;
            }
            c = lookChar();
		}
        unget();
	}
	
    Token Lexer::readChar()
    {
        auto endOfFile = [this]() -> Token {
            Diagnosis diag(DiagType::DT_Error, previousCoord_);
            diag << "unexcepted end of file";
            diag_.diag(diag);
            return Token();
        };
        
        char c = lookChar();
        if (!c)
            return endOfFile();

        if (c == '\\')
        {
            c = lookChar();
            if (!c)
                return endOfFile();
            c = escapeChar(c);
        }
        char tmp = lookChar();
        if (tmp != '\'')
        {
            Diagnosis diag(DiagType::DT_Error, previousCoord_);
            diag << "except '";
            diag_.diag(diag);
        }
        return Token(TK_LitCharacter, previousCoord_, string(1, c).c_str());
    }

    Token Lexer::readString()
	{
        string value;
        char c = lookChar();
        while (c)
        {
            if (c == '"')
            {
                return Token(TK_LitString, previousCoord_, value.c_str());
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
        Diagnosis diag(DiagType::DT_Error, previousCoord_);
        diag << "unexcepted end of file";
        diag_.diag(diag);
        return Token();
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
            return Token(previousCoord_, value);
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
        return Token(previousCoord_, fnum);
	}

	Token Lexer::readSign(char startChar)
	{
		switch (startChar)
		{
        case '(': return Token(TK_LParen, previousCoord_);
        case ')': return Token(TK_RParen, previousCoord_);
        case '[': return Token(TK_LSquareBrace, previousCoord_);
        case ']': return Token(TK_RSquareBrace, previousCoord_);
        case '{': return Token(TK_LCurlyBrace, previousCoord_);
        case '}': return Token(TK_RCurlyBrace, previousCoord_);
        case '+': return Token(TK_Plus, previousCoord_);
        case '-': return Token(TK_Sub, previousCoord_);
        case '*': return Token(TK_Mul, previousCoord_);
        case '/': return Token(TK_Div, previousCoord_);
        case ';': return Token(TK_Semicolon, previousCoord_);
        case ',': return Token(TK_Comma, previousCoord_);
        case '.': return Token(TK_Period, previousCoord_);
        case '|': return Token(TK_Or, previousCoord_);
        case '&': return Token(TK_And, previousCoord_);
        case '!': 
        {
            if (lookChar() == '=')
                return Token(TK_NotEqual, previousCoord_);
            unget();
            return Token(TK_Not, previousCoord_);
        }
        case '=': 
        {
            if (lookChar() == '=')
                return Token(TK_EqualThan, previousCoord_);
            unget();
            return Token(TK_Assign, previousCoord_);
        }
        case '<': 
        {
            if (lookChar() == '=')
                return Token(TK_LessThan, previousCoord_);
            unget();
            return Token(TK_Less, previousCoord_);
        }
        case '>':
        {
            if (lookChar() == '=')
                return Token(TK_GreatThan, previousCoord_);
            unget();
            return Token(TK_Great, previousCoord_);
        }
		}
        Diagnosis diag(DiagType::DT_Error, previousCoord_);
        diag << "bad character";
        diag_.diag(diag);
        return readToken();
	}

    Token Lexer::getToken()
    {
        if (tokens_.size() == 0)
            return readToken();
        Token tok = tokens_[0];
        tokens_.pop_front();
        return tok;
    }

    void Lexer::setProgram(std::string & file)
    {
        if (file_)
            file_.close();
        file_.open(file);
        if (!file_)
        {
            throw std::runtime_error("open file failed!");
        }
            
        fileName_ = file;
        coord_ = TokenCoord();
        coord_.fileName_ = fileName_.c_str();
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

    char Lexer::lookChar()
    {
        char ch = 0;
        file_.get(ch);
        coord_.linePos_++;
        return ch;
    }

    void Lexer::unget()
    {
        if (coord_.linePos_ > 0)
            coord_.linePos_--;
        else
            coord_.lineNum_--, coord_.linePos_ = coord_.lastLinePos_;
        file_.unget();
    }

}


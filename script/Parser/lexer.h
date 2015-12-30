#pragma once

#include <deque>
#include <cctype>
#include <string>
#include <fstream>
#include <unordered_map>

namespace script
{
    enum BasicTokenID 
    {
        TK_None = 0,
        TK_EOF,
        TK_Error,
        TK_Newline,
        TK_Whitespace,
        TK_Comment,
        TK_BasicTokenEnd
    };

    enum DefaultTokenIDs 
    {
        TK_Identifier = TK_BasicTokenEnd,

        TK_LitCharacter,
        TK_LitInteger,
        TK_LitFloat,
        TK_LitString,

        TK_Plus,
        TK_Sub,
        TK_Mul,
        TK_Div,
        TK_Not,
        TK_Assign,

        TK_GreatThan,
        TK_LessThan,
        TK_Less,
        TK_Great,
        TK_NotThan,
        TK_EqualThan,
        TK_NotEqualThan,
        TK_And, 
        TK_Or,

        TK_LParen,
        TK_RParen,
        TK_LCurlyBrace,
        TK_RCurlyBrace,
        TK_LSquareBrace,
        TK_RSquareBrace,
        TK_Comma,
        TK_Semicolon,
        TK_Colon,
        TK_Period,

        TK_BeginKeywordIDs
    };

    struct TokenCoord
    {
        unsigned lineNum_;
        unsigned linePos_;
        const char *fileName_;
        TokenCoord() : lineNum_(0), linePos_(0), fileName_(nullptr) {}
    };

    struct Token
    {
        unsigned short kind_;
        int num_;
        float fnum_;
        TokenCoord coord_;
        std::string value_;
        Token(unsigned short kind = TK_EOF) : kind_(kind) {}
        Token(unsigned short kind, TokenCoord coord)
            : kind_(kind)
            , coord_(coord)
        {}
        Token(unsigned short kind, TokenCoord coord, const char *value)
            : kind_(kind)
            , coord_(coord)
            , value_(value)
        {}
        Token(int num, TokenCoord coord) 
            : kind_(TK_LitInteger)
            , coord_(coord)
            , num_(num)
        {}
        Token(float fnum, TokenCoord coord)
            : kind_(TK_LitFloat)
            , coord_(coord)
            , fnum_(fnum)
        {}
    };

    class Lexer
    {
        using KeywordsKind = std::unordered_map<std::string, unsigned>;
    public:
        Lexer() { }

        Token getToken();
        
        void setProgram(std::string &file)
        {
            if (file_)
                file_.close();
            file_.open(file);
            if (!file_)
                throw std::runtime_error("open file is false");
            fileName_ = file;
            coord_ = TokenCoord();
            coord_.fileName_ = fileName_.c_str();
        }

        Token lookAhead(unsigned num);
        void registerKeyword(std::string &str, unsigned tok);

    private:
        char lookChar()
        {
            char ch = 0;
            file_.get(ch);
            coord_.linePos_++;
            return ch;
        }

        void unget()
        {
            if (coord_.linePos_ > 0)
                coord_.linePos_--;
            file_.unget();
        }

        unsigned short getKeywordsID(const char *name);

        void whiteSpace();
        void readComments();

        char escapeChar(char c);

        Token readToken();
        Token readChar();
        Token readString();
        Token readSign(char startChar);
        Token readDigit(char startChar);
        Token readIdentifier(char startChar);

    private:
        std::ifstream file_;
        TokenCoord coord_;

        std::string fileName_;

        KeywordsKind keywords_;

        std::deque<Token> tokens_;
    };
}

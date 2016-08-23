#pragma once
#include <deque>
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
        TK_NotEqual,
        TK_EqualThan,
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
        unsigned lastLinePos_;
        const char *fileName_;
        TokenCoord() : lineNum_(1), linePos_(0), lastLinePos_(0), fileName_(nullptr) {}
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
        Token(TokenCoord coord, int num) 
            : kind_(TK_LitInteger)
            , coord_(coord)
            , num_(num)
        {}
        Token(TokenCoord coord, float fnum)
            : kind_(TK_LitFloat)
            , coord_(coord)
            , fnum_(fnum)
        {}
        bool operator == (const Token &rhs) const
        {
            if (kind_ != rhs.kind_)
                return false;
            if (kind_ == TK_LitInteger)
                return num_ == rhs.num_;
            if (kind_ == TK_LitFloat)
                return fnum_ == rhs.fnum_;
            return value_ == rhs.value_;
        }
    };


    class DiagnosisConsumer;

    class Lexer
    {
        using KeywordsKind = std::unordered_map<std::string, unsigned>;
    public:
        Lexer(DiagnosisConsumer &consumer) : diag_(consumer) {}

        Token getToken();
        TokenCoord getCoord();

        void setProgram(const std::string &file);
        Token lookAhead(unsigned num);
        void registerKeyword(const std::string &str, unsigned tok);

    private:
        char lookChar();
        void unget();

        unsigned short getKeywordsID(const char *name);

        void whiteSpace();
        void readComments();

        char escapeChar( char c);

        Token readToken();
        Token readChar();
        Token readString();
        Token readSign(char startChar);
        Token readDigit(char startChar);
        Token readIdentifier(char startChar);

    private:
        std::ifstream file_;
        TokenCoord coord_;
        TokenCoord previousCoord_;
        DiagnosisConsumer &diag_;
        std::string fileName_;

        KeywordsKind keywords_;

        std::deque<Token> tokens_;
    };
}

#define TEST_MAIN

#include "UnitTest.h"
#include "../script/Parser/Lexer.h"

#include <string>
#include <map>

using std::map;
using std::string;
using namespace script;

    enum KeywordsIDs {
        TK_If = TK_BeginKeywordIDs,
        TK_Let,
        TK_Else,
        TK_TRUE,
        TK_NULL,
        TK_FALSE,
        TK_While,
        TK_Break,
        TK_Return,
        TK_Continue,
        TK_Function,
        TK_Define,

        TK_EndKeywordsIDs
    };
    
#define EXCEPT(a)               \
token_ = lexer.getToken();      \
TEST_CHECK(token_.kind_ == a, token_.kind_, token_.value_)

// define main = function() {
//     let a = 1 + 2 - 0.4 * 5 / 6;
//     if (a | 1 & 2) {
//         return ;
//         return a = 1;
//     }
//     while (true) {
//         break;
//         continue;
//     }
//     "string";
//     'c';
//     '\n';
//     '\r';
//     '\t';
//     '\\';
//     '\"';
//     '\'';
//     a(a, a, a);
//     a[a];
//     let b = [ "good", 123 ];
// };

// define a = main();

TEST_CASE(TestLexer)
{
    string file("test.txt");
    
    map<string, unsigned> keywords = {
        { "if", TK_If },
        { "let", TK_Let },
        { "else", TK_Else },
        { "true", TK_TRUE },
        { "null", TK_NULL },
        { "false", TK_FALSE },
        { "while", TK_While },
        { "break", TK_Break },
        { "return", TK_Return },
        { "continue", TK_Continue },
        { "function", TK_Function },
        { "define", TK_Define },
    };
    
    Token token_;
    Lexer lexer;
    for (auto i : keywords)
    {
        lexer.registerKeyword(i.first, i.second);
    }
   
    lexer.setProgram(file);
    
    EXCEPT(TK_Define);
    EXCEPT(TK_Identifier);
    EXCEPT(TK_Assign);
    EXCEPT(TK_Function);
    EXCEPT(TK_LParen);
    EXCEPT(TK_RParen);
    EXCEPT(TK_LCurlyBrace);
    
    EXCEPT(TK_Let);
    EXCEPT(TK_Identifier);
    EXCEPT(TK_Assign);
    EXCEPT(TK_LitInteger);
    EXCEPT(TK_Plus);
    EXCEPT(TK_LitInteger);
    EXCEPT(TK_Sub);
    EXCEPT(TK_LitFloat);
    EXCEPT(TK_Mul);
    EXCEPT(TK_LitInteger);
    EXCEPT(TK_Div);
    EXCEPT(TK_LitInteger);
    EXCEPT(TK_Semicolon);
    
    EXCEPT(TK_If);
    EXCEPT(TK_LParen);
    EXCEPT(TK_Identifier);
    EXCEPT(TK_Or);
    EXCEPT(TK_LitInteger);
    EXCEPT(TK_And);
    EXCEPT(TK_LitInteger);
    EXCEPT(TK_RParen);
    EXCEPT(TK_LCurlyBrace);
    EXCEPT(TK_Return);
    EXCEPT(TK_Semicolon);
    EXCEPT(TK_Return);
    EXCEPT(TK_Identifier);
    EXCEPT(TK_Assign);
    EXCEPT(TK_LitInteger);
    EXCEPT(TK_Semicolon);
    EXCEPT(TK_RCurlyBrace);
    
    EXCEPT(TK_While);
    EXCEPT(TK_LParen);
    EXCEPT(TK_TRUE);
    EXCEPT(TK_RParen);
    EXCEPT(TK_LCurlyBrace);
    EXCEPT(TK_Break);
    EXCEPT(TK_Semicolon);
    EXCEPT(TK_Continue);
    EXCEPT(TK_Semicolon);
    EXCEPT(TK_RCurlyBrace);
    
    
    EXCEPT(TK_LitString);
    EXCEPT(TK_Semicolon);
    
    EXCEPT(TK_LitCharacter);
    EXCEPT(TK_Semicolon);
    
    EXCEPT(TK_LitCharacter);
    EXCEPT(TK_Semicolon);
    
    EXCEPT(TK_LitCharacter);
    EXCEPT(TK_Semicolon);
    
    EXCEPT(TK_LitCharacter);
    EXCEPT(TK_Semicolon);
    
    EXCEPT(TK_LitCharacter);
    EXCEPT(TK_Semicolon);
    
    EXCEPT(TK_LitCharacter);
    EXCEPT(TK_Semicolon);
    
    EXCEPT(TK_LitCharacter);
    EXCEPT(TK_Semicolon);
    
    EXCEPT(TK_Identifier);
    EXCEPT(TK_LParen);
    EXCEPT(TK_Identifier);
    EXCEPT(TK_Comma);
    EXCEPT(TK_Identifier);
    EXCEPT(TK_Comma);
    EXCEPT(TK_Identifier);
    EXCEPT(TK_RParen);
    EXCEPT(TK_Semicolon);
    
    EXCEPT(TK_Identifier);
    EXCEPT(TK_LSquareBrace);
    EXCEPT(TK_Identifier);
    EXCEPT(TK_RSquareBrace);
    EXCEPT(TK_Semicolon);
    
    EXCEPT(TK_Let);
    EXCEPT(TK_Identifier);
    EXCEPT(TK_Assign);
    EXCEPT(TK_LSquareBrace);
    EXCEPT(TK_LitString);
    EXCEPT(TK_Comma);
    EXCEPT(TK_LitInteger);
    EXCEPT(TK_RSquareBrace);
    EXCEPT(TK_Semicolon);
    
    EXCEPT(TK_RCurlyBrace);
    EXCEPT(TK_Semicolon);
    
    EXCEPT(TK_Define);
    EXCEPT(TK_Identifier);
    EXCEPT(TK_Assign);
    EXCEPT(TK_Identifier);
    EXCEPT(TK_LParen);
    EXCEPT(TK_RParen);
    EXCEPT(TK_Semicolon);
}
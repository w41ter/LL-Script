#include "Diagnosis.h"

#include <sstream>

using std::string;
using std::stringstream;

namespace script
{
    static char *tokens[TK_BeginKeywordIDs] = {
        "[None]",
        "[EOF]",
        "[Error]",
        "[NewLine]",
        "[WhiteSpace]",
        "[Comment]",
        "[BasicTokenEnd]",
        "[character]",
        "[LitInteger]",
        "[LitFloat]",
        "[LitString]",
        "+",
        "-",
        "*",
        "/",
        "!",
        "=",
        ">=",
        "<=",
        "<",
        ">",
        "!=",
        "==",
        "&",
        "|",
        "{",
        "}",
        "(",
        ")",
        "[",
        "]",
        ",",
        ";",
        ":",
        "."
    };
    const char * Diagnosis::TokenToStirng(unsigned kind)
    {
        if (kind >= TK_BeginKeywordIDs)
            return " ";
        return tokens[kind];
    }

    const char * Diagnosis::DiagTypeToString(DiagType type)
    {
        switch (type)
        {
        case script::DiagType::DT_Error:
            return "Error";
        case script::DiagType::DT_Warning:
            return "Warning";
        case script::DiagType::DT_Tips:
            return "Tips";
        default:
            return "";
        }
    }

    string Diagnosis::format()
    {
        string message = "";
        stringstream stream;
        stream << coord_.fileName_
            << " (" << coord_.lineNum_ << ',' << coord_.linePos_ + 1 << ") "
            << DiagTypeToString(type_) << ": " << message_;
        std::getline(stream, message);
        return message;
    }
}
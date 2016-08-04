#include "OperatorSystem.h"

#include <iostream>
#include <string>

#include "../Runtime/Runtime.h"

using std::string;
using std::cout;
using std::endl;
using std::cin;

string Input()
{
    string str;
    cin >> str;
    return str;
}

string Output(script::Pointer p)
{
    if (script::IsFixnum(p)) 
    {
        cout << script::GetFixnum(p);
    }
    else if (script::IsReal(p))
    {
        cout << script::GetReal(p);
    }
    else if (script::IsChar(p))
    {
        cout << script::GetChar(p);
    }
    else if (script::IsString(p))
    {
        cout << script::GetString(p);
    }
    else if (script::IsClosure(p))
    {
        cout << "<closure>";
    }
    else if (script::IsArray(p))
    {
        cout << "[";
        script::Pointer *array = script::ArrayData(p);
        size_t length = script::ArrayLength(p);
        for (size_t i = 0; i < length; ++i)
        {
            Output(*(array + i));
            if (i + 1 == length)
                break;
            cout << ", ";
        }
        cout << "]";
    }
    else if (script::IsNil(p))
    {
        cout << "nil";
    }
    return "";
}

namespace buildin
{
    OS & OS::getInstance()
    {
        static OS os;
        return os;
    }

    void OS::map(Map m)
    {
    }

    OS::OS()
    {
        initializer();
    }

    void OS::initializer()
    {
    }
}

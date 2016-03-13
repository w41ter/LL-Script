#include "BuildIn.h"

#include <iostream>
#include <string>
#include <vector>
#include <assert.h>

#include "lexical_cast.h"
#include "OperatorSystem.h"
#include "../Runtime/GC.h"
#include "../Runtime/Runtime.h"

using std::string;
using std::vector;
using std::cin;
using std::cout;
using std::endl;
using namespace script;

#define FUNC_OUTPUT 1
#define FUNC_INPUT (FUNC_OUTPUT + 1)

bool getInteger(const string &str, int &val)
{
    try {
        val = lexical_cast<int>(str.c_str());
        return true;
    }
    catch (const exception &e) {
        return false;
    }
}

bool getFloat(const string &str, float &fval)
{
    try {
        fval = lexical_cast<float>(str.c_str());
        return true;
    }
    catch (const exception &e) {
        return false;
    }
}

namespace buildin
{

    BuildIn & BuildIn::getInstance()
    {
        static BuildIn buildin;
        return buildin;
    }

    void BuildIn::map(Map m)
    {
        for (auto &i : functions_)
        {
            string name = m(i.first, i.second);
            auto p = std::pair<string, int>(name, functionMap_[i.first]);
            mapName_.insert(std::move(p));
        }
    }

    // if not found , return -1
    int BuildIn::getFunctionIndex(const std::string & name)
    {
        if (mapName_.find(name) == mapName_.end())
            return -1;
        return mapName_[name];
    }

    Pointer BuildIn::excute(int index, std::vector<Pointer> params)
    {
        switch (index)
        {
        case FUNC_OUTPUT:
            return output(std::move(params));
            break;
        case FUNC_INPUT:
            return input(std::move(params));
            break;
        }

        return Pointer();
    }

    void BuildIn::bindGC(GarbageCollector * gc)
    {
        gc_ = gc;
    }

    BuildIn::BuildIn()
    {
        initializer();
    }

    BuildIn::~BuildIn()
    {
    }

    void BuildIn::initializer()
    {
        functions_["output"] = 1;
        functionMap_["output"] = FUNC_OUTPUT;
        functions_["input"] = 0;
        functionMap_["input"] = FUNC_INPUT;
    }

    Pointer BuildIn::input(std::vector<Pointer> params)
    {
        assert(params.size() == 0);
        string str;
        cin >> str;
        // TODO: there have some bug...
        {
            int val = 0;
            if (getInteger(str, val))
                return MakeFixnum(val);
        }
        {
            float val = 0.f;
            if (getFloat(str, val))
                return MakeReal(val);
        }
        Pointer p = gc_->allocate(STRING_SIZE(str.length()));
        return MakeString(p, str.c_str(), str.size());
    }

    Pointer BuildIn::output(std::vector<Pointer> params)
    {
        assert(params.size() == 1);
        output(params[0]);
        return MakeFixnum(0);
    }

    void BuildIn::output(Pointer p)
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
            for (int i = 0; i < length; ++i)
            {
                output(*(array + i));
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
    }

}
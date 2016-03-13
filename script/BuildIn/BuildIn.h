#pragma once

#include <functional>
#include <string>
#include <vector>
#include <map>

namespace script
{
    class GarbageCollector;
}

namespace buildin
{
    typedef int32_t Pointer;
    using Map = std::function<std::string(const std::string&, int)>;


    class BuildIn
    {
    public:
        static BuildIn &getInstance();

        void map(Map m);

        int getFunctionIndex(const std::string &name);
        Pointer excute(int index, std::vector<Pointer> params);

        void bindGC(script::GarbageCollector *gc);

    private:
        BuildIn();
        ~BuildIn();

        void initializer();

        Pointer input(std::vector<Pointer> params);
        Pointer output(std::vector<Pointer> params);
        void output(Pointer p);
    private:
        std::map<std::string, int> functions_;
        std::map<std::string, int> functionMap_;
        std::map<std::string, int> mapName_;

        script::GarbageCollector *gc_;
    };

    class BuildInMoudle
    {
    public:
        virtual void map(Map m) = 0;
    };
}
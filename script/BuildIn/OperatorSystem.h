#pragma once

#include "BuildIn.h"

namespace buildin
{
    class OS : public BuildInMoudle
    {
    public:
        static OS &getInstance();

        virtual void map(Map m);

    private:
        OS();
        ~OS() = default;

        void initializer();

    private:
        std::map<std::string, std::vector<std::string>> functions_;
    };
}

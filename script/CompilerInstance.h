#pragma once

#include "Semantic\DiagnosisConsumer.h"
#include "driver.h"

namespace script
{
    class CompilerInstance
    {
    public:

        DiagnosisConsumer &getDiagnosisConsumer()
        {
            return diagConsumer_;
        }

        Driver &getDriver()
        {
            return driver_;
        }

    private:
        Driver driver_;
        DiagnosisConsumer diagConsumer_;
    };
}
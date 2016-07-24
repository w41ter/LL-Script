#pragma once

#include "Diagnosis.h"
#include <vector>

namespace script
{
    class DiagnosisConsumer
    {
    public:
        void diag(Diagnosis &diag);

    private:
        std::vector<Diagnosis> diags_;
    };
}
#pragma once

#include "Diagnosis.h"
#include <vector>

namespace script
{
    class DiagnosisConsumer
    {
    public:
        DiagnosisConsumer() : mode_(false), errors_(0), warnings_(0) {}

        void setMode(bool mode);
        void diag(Diagnosis &diag);
        void show();
        int errors() const { return errors_; }
        int warnings() const { return warnings_; }
    private:
        int errors_;
        int warnings_;
        bool mode_;
        std::vector<Diagnosis> diags_;
    };
}
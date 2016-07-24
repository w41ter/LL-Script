#include "DiagnosisConsumer.h"

namespace script
{
    void DiagnosisConsumer::diag(Diagnosis &diag)
    {
        diags_.push_back(diag);
    }
}
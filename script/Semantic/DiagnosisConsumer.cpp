#include "DiagnosisConsumer.h"

#include <iostream>

using std::cout;
using std::endl;

namespace script
{
    void DiagnosisConsumer::setMode(bool mode)
    {
        mode_ = mode;
    }

    void DiagnosisConsumer::diag(Diagnosis &diag)
    {
        if (diag.type_ == DiagType::DT_Error)
            errors_++;
        else if (diag.type_ == DiagType::DT_Warning)
            warnings_++;

        if (mode_)
            cout << diag.format() << endl;
        else 
            diags_.push_back(diag);
    }

    void DiagnosisConsumer::show()
    {
        for (auto diag : diags_)
        {
            cout << diag.format() << endl;
        }
    }
}
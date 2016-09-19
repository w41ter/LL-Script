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

    void DiagnosisConsumer::undefineID(const std::string &name, TokenCoord coord)
    {
        Diagnosis diag(DiagType::DT_Error, coord);
        diag << "Using undefine identifier : " << name;
        this->diag(diag);
    }

    void DiagnosisConsumer::unexceptedToken(unsigned kind, TokenCoord coord)
    {
        Diagnosis diag(DiagType::DT_Error, coord);
        diag << "Unexcepted token : " << Diagnosis::TokenToStirng(kind);
        this->diag(diag);
    }

    void DiagnosisConsumer::outOfScopeBreakOrContinue(TokenCoord coord)
    {
        Diagnosis diag(DiagType::DT_Error, coord);
        diag << "break/continue need in while statement";
        this->diag(diag);
    }

    void DiagnosisConsumer::unknowTableDecl(TokenCoord coord)
    {
        Diagnosis diag(DiagType::DT_Error, coord);
        diag << "Unknow table declare!";
        this->diag(diag);
    }

    void DiagnosisConsumer::indexLessThanZero(TokenCoord coord)
    {
        Diagnosis diag(DiagType::DT_Error, coord);
        diag << "table index can't be less than zero";
        this->diag(diag);
    }

    void DiagnosisConsumer::redefineAs(const std::string & name, TokenCoord coord)
    {
        Diagnosis diag(DiagType::DT_Error, coord);
        diag << "redefined \"" << name << "\" as " << name;
        this->diag(diag);
    }

    void DiagnosisConsumer::except(unsigned tok1, unsigned tok2, TokenCoord coord)
    {
        Diagnosis diag(DiagType::DT_Error, coord);
        diag << Diagnosis::TokenToStirng(tok1)
            << " except in file but find "
            << Diagnosis::TokenToStirng(tok2);
        this->diag(diag);
    }

}
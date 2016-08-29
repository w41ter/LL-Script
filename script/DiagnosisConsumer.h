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

        // helper
        void undefineID(const std::string &name, TokenCoord coord);
        void unexceptedToken(unsigned token, TokenCoord coord);
        void outOfScopeBreakOrContinue(TokenCoord coord);
        void unknowTableDecl(TokenCoord coord);
        void indexLessThanZero(TokenCoord coord);
        void redefineAs(const std::string &name, TokenCoord coord);
        void except(unsigned tok1, unsigned tok2, TokenCoord coord);
    private:
        int errors_;
        int warnings_;
        bool mode_;
        std::vector<Diagnosis> diags_;
    };
}
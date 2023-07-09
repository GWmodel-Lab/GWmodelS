#ifndef GWMMONOVARIABLEANALYSIS_H
#define GWMMONOVARIABLEANALYSIS_H

#include "Model/gwmvariableitemmodel.h"

struct IMonovariableAnalysis
{
    virtual GwmVariable variable() const = 0;
    virtual void setVariable(const GwmVariable& variable) = 0;
};

#endif // GWMMONOVARIABLEANALYSIS_H

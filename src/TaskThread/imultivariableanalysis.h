#ifndef GWMMULTIVARIABLEANALYSIS_H
#define GWMMULTIVARIABLEANALYSIS_H

#include "Model/gwmvariableitemmodel.h"

struct IGwmMultivariableAnalysis
{
    virtual QList<GwmVariable> variables() const = 0;
    virtual void setVariables(const QList<GwmVariable>& variables) = 0;
};

#endif // GWMMULTIVARIABLEANALYSIS_H

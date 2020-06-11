#ifndef GWMMULTIVARIABLEANALYSIS_H
#define GWMMULTIVARIABLEANALYSIS_H

#include "Model/gwmvariableitemmodel.h"

interface IMultivariableAnalysis
{
    virtual QList<GwmVariable> getVariables() const = 0;
    virtual void setVariables(const QList<GwmVariable>& variables) = 0;
    virtual void setVariables(const QList<GwmVariable>&& variables) = 0;
};

#endif // GWMMULTIVARIABLEANALYSIS_H

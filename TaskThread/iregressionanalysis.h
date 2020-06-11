#ifndef GWMREGRESSIONANALYSIS_H
#define GWMREGRESSIONANALYSIS_H

#include "Model/gwmvariableitemmodel.h"
#include <armadillo>

interface IRegressionAnalysis
{
    virtual GwmVariable getDependentVariable() const = 0;
    virtual void setDependentVariable(const GwmVariable& variable) = 0;
    virtual void setDependentVariable(const GwmVariable&& variable) = 0;

    virtual QList<GwmVariable> getIndependentVariables() const = 0;
    virtual void setIndependentVariables(const QList<GwmVariable>& variables) = 0;
    virtual void setIndependentVariables(const QList<GwmVariable>&& variables) = 0;

    virtual arma::mat regression() = 0;
};

#endif // GWMREGRESSIONANALYSIS_H

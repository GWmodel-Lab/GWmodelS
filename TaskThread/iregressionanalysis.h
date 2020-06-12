#ifndef GWMREGRESSIONANALYSIS_H
#define GWMREGRESSIONANALYSIS_H

#include "Model/gwmvariableitemmodel.h"
#include <armadillo>

struct GwmDiagnostic
{
    double RSS;
    double AIC;
    double AICc;
    double ENP;
    double EDF;
    double RSquare;
    double RSquareAdjust;
};


struct IRegressionAnalysis
{
    virtual GwmVariable dependentVariable() const = 0;
    virtual void setDependentVariable(const GwmVariable& variable) = 0;
    virtual void setDependentVariable(const GwmVariable&& variable) = 0;

    virtual QList<GwmVariable> independentVariables() const = 0;
    virtual void setIndependentVariables(const QList<GwmVariable>& variables) = 0;
    virtual void setIndependentVariables(const QList<GwmVariable>&& variables) = 0;

    virtual GwmDiagnostic diagnostic() const = 0;

    virtual arma::mat regression(const arma::mat& x, const arma::vec& y) = 0;
};

#endif // GWMREGRESSIONANALYSIS_H

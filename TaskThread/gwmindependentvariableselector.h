#ifndef GWMINDEPENDENTVARIABLESELECTOR_H
#define GWMINDEPENDENTVARIABLESELECTOR_H

#include <Model/gwmvariableitemmodel.h>
#include <armadillo>

struct IIndependentVariableSelectable
{
    virtual double criterion(QList<GwmVariable> variables) = 0;
};

class GwmIndependentVariableSelector
{
public:
    GwmIndependentVariableSelector();

    QList<GwmVariable> indepVars() const;
    void setIndepVars(const QList<GwmVariable> &indepVars);

    double getThreshold() const;
    void setThreshold(double threshold);

public:
    QList<GwmVariable> optimize(IIndependentVariableSelectable* instance);

private:
    QList<GwmVariable> getVariables(QList<int> index);
    QList<QPair<QList<int>, double> > sort(QList<QPair<QList<int>, double> > models);
    QPair<QList<int>, double> select(QList<QPair<QList<int>, double> > models);

private:
    QList<GwmVariable> mIndepVars;
    QList<QPair<QList<int>, double> > mIndepVarsCriterion;
    double mThreshold;
};

#endif // GWMINDEPENDENTVARIABLESELECTOR_H

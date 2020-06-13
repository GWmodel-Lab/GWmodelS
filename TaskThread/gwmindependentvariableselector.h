#ifndef GWMINDEPENDENTVARIABLESELECTOR_H
#define GWMINDEPENDENTVARIABLESELECTOR_H

#include <QMetaType>
#include <Model/gwmvariableitemmodel.h>
#include <armadillo>
#include <qwt_plot.h>

struct IIndependentVariableSelectable
{
    virtual double criterion(QList<GwmVariable> variables) = 0;
};

typedef  QList<QPair<QList<GwmVariable>, double> > IndepVarsCriterionList;
Q_DECLARE_METATYPE(IndepVarsCriterionList)

class GwmIndependentVariableSelector
{
public:
    static void PlotModelOrder(QVariant data, QwtPlot* plot);
    static void PlotModelAICcs(QVariant data, QwtPlot* plot);

public:
    GwmIndependentVariableSelector();

    QList<GwmVariable> indepVars() const;
    void setIndepVars(const QList<GwmVariable> &indepVars);

    double threshold() const;
    void setThreshold(double threshold);

public:
    QList<GwmVariable> optimize(IIndependentVariableSelectable* instance);

    IndepVarsCriterionList indepVarsCriterion() const;

private:
    QList<GwmVariable> getVariables(QList<int> index);
    QList<QPair<QList<int>, double> > sort(QList<QPair<QList<int>, double> > models);
    QPair<QList<int>, double> select(QList<QPair<QList<int>, double> > models);

private:
    QList<GwmVariable> mIndepVars;
    QList<QPair<QList<int>, double> > mIndepVarsCriterion;
    double mThreshold;
};

inline QList<GwmVariable> GwmIndependentVariableSelector::indepVars() const
{
    return mIndepVars;
}

inline void GwmIndependentVariableSelector::setIndepVars(const QList<GwmVariable> &indepVars)
{
    mIndepVars = indepVars;
}

inline double GwmIndependentVariableSelector::threshold() const
{
    return mThreshold;
}

inline void GwmIndependentVariableSelector::setThreshold(double threshold)
{
    mThreshold = threshold;
}

#endif // GWMINDEPENDENTVARIABLESELECTOR_H

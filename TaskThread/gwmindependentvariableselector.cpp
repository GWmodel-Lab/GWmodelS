#include "gwmindependentvariableselector.h"

using namespace arma;

GwmIndependentVariableSelector::GwmIndependentVariableSelector()
{

}

QList<GwmVariable> GwmIndependentVariableSelector::indepVars() const
{
    return mIndepVars;
}

void GwmIndependentVariableSelector::setIndepVars(const QList<GwmVariable> &indepVars)
{
    mIndepVars = indepVars;
}

QList<GwmVariable> GwmIndependentVariableSelector::optimize(IIndependentVariableSelectable *instance)
{
    QList<int> curIndex, restIndex;
    QList<QPair<QList<int>, double> > modelCriterions;
    for (int i = 0; i < mIndepVars.size(); i++)
    {
        vec AICcs = vec(mIndepVars.size() - i);
        for (int j = 0; j < mIndepVars.size() - i; j++)
        {
            curIndex.append(j);
            double aic = instance->criterion(getVariables(curIndex));
            AICcs(j) = aic;
            modelCriterions.append(qMakePair(curIndex, aic));
            curIndex.removeLast();
        }
        int iBestVar = AICcs.index_min();
        curIndex.append(restIndex[iBestVar]);
        restIndex.removeAt(iBestVar);
    }
    mIndepVarsCriterion = sort(modelCriterions);
    return getVariables(select(mIndepVarsCriterion).first);
}

QList<GwmVariable> GwmIndependentVariableSelector::getVariables(QList<int> index)
{
    QList<GwmVariable> variables;
    for (int i : index)
        variables.append(mIndepVars[i]);
    return variables;
}

QList<QPair<QList<int>, double> > GwmIndependentVariableSelector::sort(QList<QPair<QList<int>, double> > models)
{
    int tag = 0;
    QList<int> sortIndex;
    for (int i = mIndepVars.size(); i > 0; i--)
    {
        vec tmpList(i, fill::zeros);
        for (int j = tag; j < tag + i; j++)
        {
            tmpList.row(j - tag) = models[j].second;
        }
        for (int j = 0; j < i; j++)
        {
            int index = tmpList.index_max();
            sortIndex.append(index + tag);
            tmpList.row(index) = 0;
        }
        tag = tag + i;
    }
    QList<QPair<QList<int>, double> > sorted;
    for (int i : sortIndex)
    {
        sorted.append(models[i]);
    }
    return sorted;
}

QPair<QList<int>, double> GwmIndependentVariableSelector::select(QList<QPair<QList<int>, double> > models)
{
    for (int i = models.size() - 1; i >= 0; i--)
    {
        if (models[i - 1].second - models[i].second >= mThreshold)
        {
            return models[i];
        }
    }
}

double GwmIndependentVariableSelector::getThreshold() const
{
    return mThreshold;
}

void GwmIndependentVariableSelector::setThreshold(double threshold)
{
    mThreshold = threshold;
}

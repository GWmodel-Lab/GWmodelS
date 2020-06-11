#ifndef GWMGWSSTASKTHREAD_H
#define GWMGWSSTASKTHREAD_H

#include <QObject>

#include "TaskThread/gwmspatialmonoscalealgorithm.h"
#include "TaskThread/imultivariableanalysis.h"

class GwmGWSSTaskThread : public GwmSpatialMonoscaleAlgorithm, public IMultivariableAnalysis
{
    Q_OBJECT
public:
    GwmGWSSTaskThread();
    bool quantile() const;
    void setQuantile(bool quantile);

public:
    QList<GwmVariable> getVariables() const;
    void setVariables(const QList<GwmVariable> &variables);
    void setVariables(const QList<GwmVariable> &&variables);

protected:
    void createResultLayer();

private:
    QList<GwmVariable> mVariables;
    bool mQuantile = false;
};

#endif // GWMGWSSTASKTHREAD_H

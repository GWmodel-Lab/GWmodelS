#ifndef GWMGWSSTASKTHREAD_H
#define GWMGWSSTASKTHREAD_H

#include <QObject>

#include "TaskThread/gwmspatialmonoscalealgorithm.h"
#include "TaskThread/imultivariableanalysis.h"
#include "TaskThread/iparallelable.h"

class GwmGWSSTaskThread : public GwmSpatialMonoscaleAlgorithm, public IMultivariableAnalysis, public IOpenmpParallelable
{
    Q_OBJECT
public:
    GwmGWSSTaskThread();
    bool quantile() const;
    void setQuantile(bool quantile);

public:  // IMultivariableAnalysis interface
    QList<GwmVariable> getVariables() const;
    void setVariables(const QList<GwmVariable> &variables);
    void setVariables(const QList<GwmVariable> &&variables);

public:  // IParallelalbe interface
    int parallelAbility() const;
    virtual ParallelType parallelType() const;
    virtual void setParallelType(const ParallelType& type);

public:  // IOpenmpParallelable interface
    void setThreadNum(const int threadNum);

protected:  // GwmSpatialMonoscaleAlgorithm interface
    void createResultLayer();

private:
    QList<GwmVariable> mVariables;
    bool mQuantile = false;

};

#endif // GWMGWSSTASKTHREAD_H

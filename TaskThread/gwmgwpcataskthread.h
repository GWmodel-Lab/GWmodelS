#ifndef GWMGWPCATASKTHREAD_H
#define GWMGWPCATASKTHREAD_H

#include <QObject>
#include "TaskThread/gwmspatialmonoscalealgorithm.h"
#include "TaskThread/imultivariableanalysis.h"
#include "TaskThread/iparallelable.h"

class GwmGWPCATaskThread : public GwmSpatialMonoscaleAlgorithm, public IMultivariableAnalysis, public IOpenmpParallelable
{
    Q_OBJECT
public:
    GwmGWPCATaskThread();

public:
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
    bool isValid();

private:
    QList<GwmVariable> mVariables;

};

#endif // GWMGWPCATASKTHREAD_H

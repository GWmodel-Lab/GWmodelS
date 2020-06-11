#ifndef GWMGWPCATASKTHREAD_H
#define GWMGWPCATASKTHREAD_H

#include <QObject>
#include "TaskThread/gwmspatialmonoscalealgorithm.h"
#include "TaskThread/imultivariableanalysis.h"

class GwmGWPCATaskThread : public GwmSpatialMonoscaleAlgorithm, public IMultivariableAnalysis
{
    Q_OBJECT
public:
    GwmGWPCATaskThread();

public:
    QList<GwmVariable> getVariables() const;
    void setVariables(const QList<GwmVariable> &variables);
    void setVariables(const QList<GwmVariable> &&variables);

protected:
    void createResultLayer();
    bool isValid();

private:
    QList<GwmVariable> mVariables;
};

#endif // GWMGWPCATASKTHREAD_H

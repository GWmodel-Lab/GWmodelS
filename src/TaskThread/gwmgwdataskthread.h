#ifndef GWMGWDATASKTHREAD_H
#define GWMGWDATASKTHREAD_H

#include <QObject>
#include <armadillo>
#include <gwmodel.h>

#include "TaskThread/gwmspatialmonoscalealgorithm.h"
#include "TaskThread/imultivariableanalysis.h"
#include "TaskThread/iparallelable.h"
#include "TaskThread/gwmbandwidthsizeselector.h"

#include "Model/gwmalgorithmmetavariable.h"

class GwmGWDATaskThread: public GwmSpatialMonoscaleAlgorithm, public IGwmMultivariableAnalysis, public IOpenmpParallelable
{

public:
    GwmGWDATaskThread();

};

#endif // GWMGWDATASKTHREAD_H

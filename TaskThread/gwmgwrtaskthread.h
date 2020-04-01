#ifndef GWMGWRTASKTHREAD_H
#define GWMGWRTASKTHREAD_H

#include "gwmtaskthread.h"
#include <qgsvectorlayer.h>

class GwmGWRTaskThread : public GwmTaskThread
{
    Q_OBJECT

public:

    enum BandwidthType
    {
        Adaptive,
        Fixed
    };

    enum ParallelMethod
    {
        None,
        Multithread,
        GPU
    };

public:
    GwmGWRTaskThread(QgsVectorLayer* layer, int depVarIndex, QList<int> indepVarIndex);

private:
    QgsVectorLayer* mLayer = nullptr;
    int mDepVarIndex;
    QList<int> mIndepVarIndex;
    bool isEnableIndepVarAutosel = false;

    BandwidthType mBandwidthType = BandwidthType::Adaptive;
    QVariant mBandwidthSize = 0.0;
    bool isBandwidthSizeAutoSel = true;

    double mCRSRotateTheta = 0.0;
    double mCRSRotateP = 0.0;

    ParallelMethod mParallelMethodType = ParallelMethod::None;
    QVariant mParallelParameter = 0;
};

#endif // GWMGWRTASKTHREAD_H

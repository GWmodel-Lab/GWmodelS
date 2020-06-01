#ifndef GWMMULTISCALEGWRTASKTHREAD_H
#define GWMMULTISCALEGWRTASKTHREAD_H

#include "TaskThread/gwmgwrtaskthread.h"
#include "gwmbandwidthselecttaskthread.h"

class GwmMultiscaleGWRTaskThread : public GwmGWRTaskThread
{
    Q_OBJECT

public:
    enum BandwidthSeledType
    {
        Null,
        Initial,
        Specified
    };

    enum CriterionType
    {
        CVR,
        dCVR
    };

public:
    GwmMultiscaleGWRTaskThread();

public:
    void run() override;

protected:
    double selectOptimizedBandwidth(GwmBandwidthSelectTaskThread& bwSelThread);

    vec regressionVar(const mat& x, const vec& y, bool hatmatrix, mat& S);

protected:
    vec mBandwidthSizePS;
    vec mInitialBandwidthSize;
    QList<BandwidthSeledType> mBandwidthSeled;
    QList<bool> mPreditorCentered;
    vec mBandwidthSelectThreshold;
    int mBandwidthSelectRetryTimes;
    int mMaxIteration;
    CriterionType mCriterionType;
    double mCriterionThreshold;

    mat mX0;
    vec mY0;
};

#endif // GWMMULTISCALEGWRTASKTHREAD_H

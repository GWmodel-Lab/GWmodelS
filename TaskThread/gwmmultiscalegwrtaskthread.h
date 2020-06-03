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

    vec initialBandwidthSize() const;
    void setInitialBandwidthSize(const vec &initialBandwidthSize);

    QList<BandwidthSeledType> bandwidthSeled() const;
    void setBandwidthSeled(const QList<BandwidthSeledType> &bandwidthSeled);

    QList<bool> preditorCentered() const;
    void setPreditorCentered(const QList<bool> &preditorCentered);

    vec bandwidthSelectThreshold() const;
    void setBandwidthSelectThreshold(const vec &bandwidthSelectThreshold);

    int bandwidthSelectRetryTimes() const;
    void setBandwidthSelectRetryTimes(int bandwidthSelectRetryTimes);

    CriterionType criterionType() const;
    void setCriterionType(const CriterionType &criterionType);

    double criterionThreshold() const;
    void setCriterionThreshold(double criterionThreshold);

protected:
    double selectOptimizedBandwidth(GwmBandwidthSelectTaskThread& bwSelThread, bool verbose = true);

    vec regressionVar(const mat& x, const vec& y, bool hatmatrix, mat& S);

protected:
    vec mBandwidthSizePS;
    vec mInitialBandwidthSize;
    QList<BandwidthSeledType> mBandwidthSeled;
    QList<bool> mPreditorCentered;
    vec mBandwidthSelectThreshold;
    int mBandwidthSelectRetryTimes = 5;
    int mMaxIteration = 500;
    CriterionType mCriterionType = CriterionType::CVR;
    double mCriterionThreshold = 1e-6;

    mat mX0;
    vec mY0;
};

#endif // GWMMULTISCALEGWRTASKTHREAD_H

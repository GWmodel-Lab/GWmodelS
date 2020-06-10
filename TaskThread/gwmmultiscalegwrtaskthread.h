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

    void setIndepVars(const QList<GwmLayerAttributeItem *> &indepVars) override;

    void diagnostic(double RSS);

    void createResultLayer() override;

    vec initialBandwidthSize() const;
    void setInitialBandwidthSize(const vec &initialBandwidthSize);
    void setInitialBandwidthSize(const int index, const double value);

    QList<BandwidthSeledType> bandwidthSeled() const;
    void setBandwidthSeled(const QList<BandwidthSeledType> &bandwidthSeled);
    void setBandwidthSeled(const int index, const BandwidthSeledType value);

    QList<bool> preditorCentered() const;
    void setPreditorCentered(const QList<bool> &preditorCentered);
    void setPreditorCentered(const int index, const bool value);

    vec bandwidthSelectThreshold() const;
    void setBandwidthSelectThreshold(const vec &bandwidthSelectThreshold);
    void setBandwidthSelectThreshold(const int index, const double value);

    int bandwidthSelectRetryTimes() const;
    void setBandwidthSelectRetryTimes(int bandwidthSelectRetryTimes);

    CriterionType criterionType() const;
    void setCriterionType(const CriterionType &criterionType);

    double criterionThreshold() const;
    void setCriterionThreshold(double criterionThreshold);

    int maxIteration() const;
    void setMaxIteration(int maxIteration);

    QList<QString> bandwidthUnit() const;
    void setBandwidthUnit(const QList<QString> &bandwidthUnit);
    void setBandwidthUnit(const int index, const QString value);

    QList<GwmGWRTaskThread::BandwidthType> bandwidthType() const;
    void setBandwidthType(const QList<GwmGWRTaskThread::BandwidthType> &bandwidthType);
    void setBandwidthType(const int index, const GwmGWRTaskThread::BandwidthType value);

    QList<GwmGWRTaskThread::BandwidthSelectionApproach> bandwidthSelectionApproach() const;
    void setBandwidthSelectionApproach(const QList<GwmGWRTaskThread::BandwidthSelectionApproach> &bandwidthSelectionApproach);
    void setBandwidthSelectionApproach(const int index, const GwmGWRTaskThread::BandwidthSelectionApproach value);

    QList<GwmGWRTaskThread::KernelFunction> bandwidthKernel() const;
    void setBandwidthKernel(const QList<GwmGWRTaskThread::KernelFunction> &bandwidthKernel);
    void setBandwidthKernel(const int index, const GwmGWRTaskThread::KernelFunction value);

    QList<GwmGWRTaskThread::DistanceSourceType> distanceSource() const;
    void setDistanceSource(const QList<GwmGWRTaskThread::DistanceSourceType> &distanceSource);
    void setDistanceSource(const int index, const GwmGWRTaskThread::DistanceSourceType value);

    QList<QVariant> distanceParameter() const;
    void setDistanceParameter(const QList<QVariant> &distanceParameter);
    void setDistanceParameter(const int index, const QVariant value);

    int adaptiveLower() const;
    void setAdaptiveLower(int adaptiveLower);

protected:
    double selectOptimizedBandwidth(GwmBandwidthSelectTaskThread& bwSelThread, bool verbose = true);

    vec distance(int focus, int variable);
    vec distanceCRS(int focus, int variable);
    vec distanceMinkowski(int focus, int variable);
    vec distanceDmat(int focus, int variable);

    bool regressionAllSerial();
//    bool regressionAllOmp(bool hatmatrix, mat &S) override;
    vec regressionVar(const mat& x, const vec& y, const int var, double bw, mat& S);

protected:
    vec mBandwidthSizePS;
    vec mInitialBandwidthSize;
    QList<QString> mBandwidthUnit;
    QList<BandwidthSeledType> mBandwidthSeled;
    QList<GwmGWRTaskThread::BandwidthType> mBandwidthType;
    QList<GwmGWRTaskThread::BandwidthSelectionApproach> mBandwidthSelectionApproach;
    QList<GwmGWRTaskThread::KernelFunction> mBandwidthKernelFunction;
    QList<GwmGWRTaskThread::DistanceSourceType> mDistanceSource;
    QList<QVariant> mDistSrcParameters;
    QList<bool> mPreditorCentered;
    vec mBandwidthSelectThreshold;
    int mBandwidthSelectRetryTimes = 5;
    int mMaxIteration = 500;
    CriterionType mCriterionType = CriterionType::CVR;
    double mCriterionThreshold = 1e-6;
    int mAdaptiveLower = 10;

    mat mS0;
    cube mSArray;
    cube mC;

    mat mX0;
    vec mY0;
};

#endif // GWMMULTISCALEGWRTASKTHREAD_H

#ifndef GWMLCRGWRTASKTHREAD_H
#define GWMLCRGWRTASKTHREAD_H


#include "GWmodel/GWmodel.h"
#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"
#include "TaskThread/gwmbandwidthsizeselector.h"
#include "TaskThread/gwmindependentvariableselector.h"

#include "TaskThread/iparallelable.h"

class GwmLcrGWRTaskThread:public GwmGeographicalWeightedRegressionAlgorithm, public IBandwidthSizeSelectable,public IOpenmpParallelable
{
public:

    enum BandwidthSelectionCriterionType
    {
        CV
    };

    typedef QList<QPair<QString, const mat> > CreateResultLayerData;

    static GwmDiagnostic CalcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);

    typedef double (GwmLcrGWRTaskThread::*BandwidthSelectCriterionFunction)(GwmBandwidthWeight*);

    typedef mat (GwmLcrGWRTaskThread::*Regression)(const mat&, const vec&);
public:
    GwmLcrGWRTaskThread();

    double cnThresh() const;
    void setCnThresh(double cnThresh);

    double lambda() const;
    void setLambda(double lambda);

    bool hasHatmatix() const;
    void setHasHatmatix(bool value);

    bool lambdaAdjust() const;
    void setLambdaAdjust(bool lambdaAdjust);

    GwmDiagnostic dialnostic() const{
        return mDiagnostic;
    }
    bool isAutoselectBandwidth() const;

    BandwidthCriterionList GwmLcrGWRTaskThread::bandwidthSelectorCriterions() const
    {
        return selector.bandwidthCriterion();
    }

    void GwmLcrGWRTaskThread::setIsAutoselectBandwidth(bool value)
    {
        mIsAutoselectBandwidth = value;
    }

    bool GwmLcrGWRTaskThread::autoselectBandwidth() const
    {
        return mIsAutoselectBandwidth;
    }

    BandwidthSelectionCriterionType bandwidthSelectionCriterionType() const;
    void setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType);
public:
    void run() override;

    bool isValid() override;

    arma::mat regression(const arma::mat &x, const arma::vec &y);

    double criterion(GwmBandwidthWeight *weight)
    {
        return (this->*mBandwidthSelectCriterionFunction)(weight);
    };
protected:
    //返回cv的函数
    double LcrCV(double bw,int kernel, bool adaptive,double lambda,bool lambdaAdjust,double cnThresh);
    //ridge.lm函数
    vec ridgelm(const vec& w,double lambda);

    void createResultLayer(CreateResultLayerData data);
private:
    double mLambda;

    bool mLambdaAdjust;

    double mCnThresh;

    GwmBandwidthSizeSelector selector;

    bool mHasHatmatix = false;

    double mTrS = 0;

    double mTrStS = 0;

    bool mIsAutoselectBandwidth = false;

    double bandwidthSizeCriterionCVSerial(GwmBandwidthWeight* weight);
    double bandwidthSizeCriterionCVOmp(GwmBandwidthWeight* weight);

    BandwidthSelectionCriterionType mBandwidthSelectionCriterionType = BandwidthSelectionCriterionType::CV;
    BandwidthSelectCriterionFunction mBandwidthSelectCriterionFunction = &GwmLcrGWRTaskThread::bandwidthSizeCriterionCVSerial;

    mat regressionSerial(const mat& x, const vec& y);
    mat regressionOmp(const mat& x, const vec& y);
    Regression mRegressionFunction = &GwmLcrGWRTaskThread::regressionSerial;

    IParallelalbe::ParallelType mParallelType = IParallelalbe::ParallelType::SerialOnly;
    int mOmpThreadNum = 8;
    int mGpuId = 0;
    int mGroupSize = 64;
    // IParallelalbe interface
public:
    int parallelAbility() const;
    ParallelType parallelType() const;

    // IOpenmpParallelable interface
public:
    void setOmpThreadNum(const int threadNum);

    // IParallelalbe interface
public:
    void setParallelType(const ParallelType &type);
};

inline int GwmLcrGWRTaskThread::parallelAbility() const
{
    return IParallelalbe::SerialOnly | IParallelalbe::OpenMP | IParallelalbe::CUDA;
}

inline IParallelalbe::ParallelType GwmLcrGWRTaskThread::parallelType() const
{
    return mParallelType;
}

inline void GwmLcrGWRTaskThread::setOmpThreadNum(const int threadNum)
{
    mOmpThreadNum = threadNum;
}

#endif // GWMLCRGWRTASKTHREAD_H

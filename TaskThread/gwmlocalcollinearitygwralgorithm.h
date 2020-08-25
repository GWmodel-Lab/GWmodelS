#ifndef GWMLCRGWRTASKTHREAD_H
#define GWMLCRGWRTASKTHREAD_H

#include <armadillo>

#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"
#include "TaskThread/gwmbandwidthsizeselector.h"
#include "TaskThread/gwmindependentvariableselector.h"

#include "TaskThread/iparallelable.h"

class GwmLocalCollinearityGWRAlgorithm:public GwmGeographicalWeightedRegressionAlgorithm, public IBandwidthSizeSelectable,public IOpenmpParallelable
{
public:

    enum BandwidthSelectionCriterionType
    {
        CV
    };

    typedef QList<QPair<QString, const mat> > CreateResultLayerData;

    static GwmDiagnostic CalcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat);

    typedef double (GwmLocalCollinearityGWRAlgorithm::*BandwidthSelectCriterionFunction)(GwmBandwidthWeight*);

    typedef mat (GwmLocalCollinearityGWRAlgorithm::*Regression)(const mat&, const vec&);
public:
    GwmLocalCollinearityGWRAlgorithm();

    QString name() const override { return tr("Local-collinearity GWR"); }

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

    void setIsAutoselectBandwidth(bool value)
    {
        mIsAutoselectBandwidth = value;
    }

    bool autoselectBandwidth() const
    {
        return mIsAutoselectBandwidth;
    }

    BandwidthCriterionList bandwidthSelectorCriterions() const
    {
        return selector.bandwidthCriterion();
    }

    BandwidthSelectionCriterionType bandwidthSelectionCriterionType() const;
    void setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType);
public:
    bool isValid() override;

    double criterion(GwmBandwidthWeight *weight)
    {
        return (this->*mBandwidthSelectCriterionFunction)(weight);
    };
protected:
    void run() override;

    mat regression(const mat &x, const mat &y);
    //返回cv的函数
    double LcrCV(double bw,int kernel, bool adaptive,double lambda,bool lambdaAdjust,double cnThresh);
    //ridge.lm函数
    vec ridgelm(const vec& w,double lambda);

    void createResultLayer(CreateResultLayerData data);
public:
    int parallelAbility() const;
    ParallelType parallelType() const;

    void setParallelType(const ParallelType &type);

    // IOpenmpParallelable interface
public:
    void setOmpThreadNum(const int threadNum);
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
    BandwidthSelectCriterionFunction mBandwidthSelectCriterionFunction = &GwmLocalCollinearityGWRAlgorithm::bandwidthSizeCriterionCVSerial;

    mat regressionSerial(const mat& x, const vec& y);
    mat regressionOmp(const mat& x, const vec& y);
    Regression mRegressionFunction = &GwmLocalCollinearityGWRAlgorithm::regressionSerial;

    IParallelalbe::ParallelType mParallelType = IParallelalbe::ParallelType::SerialOnly;
    int mOmpThreadNum = 8;
    int mGpuId = 0;
    int mGroupSize = 64;
};

inline int GwmLocalCollinearityGWRAlgorithm::parallelAbility() const
{
    return IParallelalbe::SerialOnly | IParallelalbe::OpenMP;
}

inline IParallelalbe::ParallelType GwmLocalCollinearityGWRAlgorithm::parallelType() const
{
    return mParallelType;
}

inline void GwmLocalCollinearityGWRAlgorithm::setOmpThreadNum(const int threadNum)
{
    mOmpThreadNum = threadNum;
}

#endif // GWMLCRGWRTASKTHREAD_H

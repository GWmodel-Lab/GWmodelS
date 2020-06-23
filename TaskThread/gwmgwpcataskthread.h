#ifndef GWMGWPCATASKTHREAD_H
#define GWMGWPCATASKTHREAD_H

#include <QObject>
#include "TaskThread/gwmspatialmonoscalealgorithm.h"
#include "TaskThread/imultivariableanalysis.h"
#include "TaskThread/iparallelable.h"

#include "TaskThread/gwmbandwidthsizeselector.h"

class GwmGWPCATaskThread : public GwmSpatialMonoscaleAlgorithm, public IBandwidthSizeSelectable, public IMultivariableAnalysis, public IOpenmpParallelable
{
    Q_OBJECT
    enum BandwidthSelectionCriterionType
    {
        CV
    };

    typedef QList<QPair<QString, const mat> > CreateResultLayerData;

    typedef double (GwmGWPCATaskThread::*BandwidthSelectCriterionFunction)(GwmBandwidthWeight*);

    typedef mat (GwmGWPCATaskThread::*PcaFunction)(const mat& , cube& , mat& , cube& );

public:
    GwmGWPCATaskThread();

public:
    QList<GwmVariable> variables() const{return QList<GwmVariable>();};
    void setVariables(const QList<GwmVariable> &variables);
    void setVariables(const QList<GwmVariable> &&variables){
        mVariables = variables;
    };

    bool isAutoselectBandwidth() const;
    void setIsAutoselectBandwidth(bool isAutoselectBandwidth);
    BandwidthSelectionCriterionType bandwidthSelectionCriterionType() const;
    void setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType);

    void setOmpThreadNum(const int threadNum){
        mOmpThreadNum = threadNum;
    };

    BandwidthCriterionList GwmGWPCATaskThread::bandwidthSelectorCriterions() const
    {
        return mSelector.bandwidthCriterion();
    }

    double k() const;
    void setK(double k);

    mat localPV() const;

    mat variance() const;

    cube loadings() const;

    cube scores() const;

public:     // QThread interface
    void run() override;

public:     // GwmSpatialMonoscaleAlgorithm interface
    bool isValid() override;

public:  // IParallelalbe interface
    int parallelAbility() const;;
    virtual ParallelType parallelType() const;;
    virtual void setParallelType(const ParallelType& type);

public:  // IOpenmpParallelable interface
    void setThreadNum(const int threadNum){};
    //void setOmpThreadNum(const int threadNum){};


private:
    void initPoints();
    void initXY(mat& x, const QList<GwmVariable>& indepVars);
    //void wpca(const mat &x, const vec &wt, double nu, double nv, mat &V, vec &S);
    void wpca(const mat &x, const vec &wt, mat &V, vec &S);
    mat rwpca(const mat &x, const vec &wt, double nu, double nv);
    void createResultLayer(CreateResultLayerData data,QList<QString> winvar);

    mat pca(const mat& x, cube& loadings, mat& sdev, cube& scores)
    {
        return (this->*mPcaFunction)(x , loadings, sdev, scores);
    };

    mat pcaSerial(const mat& x, cube& loadings, mat& variance, cube& scores);
    mat pcaOmp(const mat& x, cube& loadings, mat& variance, cube& scores);

    double bandwidthSizeCriterionCVSerial(GwmBandwidthWeight* weight);
    double bandwidthSizeCriterionCVOmp(GwmBandwidthWeight* weight);
    double criterion(GwmBandwidthWeight *weight)
    {
        return (this->*mBandwidthSelectCriterionFunction)(weight);
    }

private:
    QList<GwmVariable> mVariables;
    mat mDataPoints;

    int mK = 2;
    bool mRobust=false;
    mat mX;
    vec mLatestWt;

    GwmBandwidthSizeSelector mSelector;
    BandwidthSelectionCriterionType mBandwidthSelectionCriterionType = BandwidthSelectionCriterionType::CV;
    BandwidthSelectCriterionFunction mBandwidthSelectCriterionFunction = &GwmGWPCATaskThread::bandwidthSizeCriterionCVSerial;
    bool mIsAutoselectBandwidth = false;

    // IOpenmpParallelable interface
    IParallelalbe::ParallelType mParallelType = IParallelalbe::ParallelType::SerialOnly;
    int mOmpThreadNum = 8;

    PcaFunction mPcaFunction = &GwmGWPCATaskThread::pcaSerial;

    mat mLocalPV;
    mat mVariance;
    cube mLoadings;
    cube mScores;
};

inline mat GwmGWPCATaskThread::variance() const
{
    return mVariance;
}

inline int GwmGWPCATaskThread::parallelAbility() const
{
    return IParallelalbe::SerialOnly | IParallelalbe::OpenMP;
}

inline IParallelalbe::ParallelType GwmGWPCATaskThread::parallelType() const
{
    return mParallelType;
}

inline mat GwmGWPCATaskThread::localPV() const
{
    return mLocalPV;
}

inline double GwmGWPCATaskThread::k() const
{
    return mK;
}

inline cube GwmGWPCATaskThread::scores() const
{
    return mScores;
}

inline cube GwmGWPCATaskThread::loadings() const
{
    return mLoadings;
}

inline void GwmGWPCATaskThread::setK(double k)
{
    mK = k;
}

#endif // GWMGWPCATASKTHREAD_H

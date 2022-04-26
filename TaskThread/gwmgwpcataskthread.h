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

    typedef QList<QPair<QString, mat> > CreateResultLayerData;
    typedef QList<QPair<QString, vec> > CreatePlotLayerData;

    typedef double (GwmGWPCATaskThread::*BandwidthSelectCriterionFunction)(GwmBandwidthWeight*);

    typedef mat (GwmGWPCATaskThread::*PcaLoadingsSdevScoresFunction)(const mat& , cube& , mat& , cube& );
    typedef mat (GwmGWPCATaskThread::*PcaLoadingsSdev)(const mat&, cube&, mat&);

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

    BandwidthCriterionList bandwidthSelectorCriterions() const
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


public:     // GwmTaskThread interface
    QString name() const override { return tr("GWPCA"); }


public:     // GwmSpatialMonoscaleAlgorithm interface
    bool isValid() override;


public:  // IParallelalbe interface
    int parallelAbility() const;;
    virtual ParallelType parallelType() const;;
    virtual void setParallelType(const ParallelType& type);

public:  // IOpenmpParallelable interface
    void setThreadNum(const int threadNum){};
    //void setOmpThreadNum(const int threadNum){};

    bool zscore() const;
    void setZscore(bool zscore);

    bool scoresCal() const;
    void setScoresCal(bool scoresCal);

    bool Robust() const;
    void setRobust(bool robust);

    bool getPlot() const;
    void setPlot(bool plot);

    void setCanceled(bool canceled);

private:
    void initPoints();
    void initXY(mat& x, const QList<GwmVariable>& indepVars);
    void variableZscore(mat& x);
    //void wpca(const mat &x, const vec &wt, double nu, double nv, mat &V, vec &S);
    void wpca(const mat &x, const vec &wt, mat &V, vec &S);
    void rwpca(const mat &x, const vec &wt, mat &coeff, vec &latent, double nu, double nv);
    void createResultLayer(CreateResultLayerData data,QList<QString> winvar);
    void createPlotLayer(CreatePlotLayerData data, QList<QString> varpc);

    mat pca(const mat& x, cube& loadings, mat& sdev, cube& scores)
    {
        return (this->*mPcaLoadingsSdevScoresFunction)(x , loadings, sdev, scores);
    };

    //实现不计算scores的pca
    mat pca(const mat& x, cube& loadings, mat& variance)
    {
        return (this->*mPcaLoadingsSdevFunction)(x,loadings,variance);
    }

    mat pcaLoadingsSdevScoresSerial(const mat& x, cube& loadings, mat& stddev, cube& scores);
#ifdef ENABLE_OpenMP
    mat pcaLoadingsSdevScoresOmp(const mat& x, cube& loadings, mat& stddev, cube& scores);
#endif
    mat pcaLoadingsSdevSerial(const mat& x, cube& loadings, mat& stddev);
#ifdef ENABLE_OpenMP
    mat pcaLoadingsSdevOmp(const mat& x, cube& loadings, mat& stddev);
#endif

    double bandwidthSizeCriterionCVSerial(GwmBandwidthWeight* weight);
#ifdef ENABLE_OpenMP
    double bandwidthSizeCriterionCVOmp(GwmBandwidthWeight* weight);
#endif
    double criterion(GwmBandwidthWeight *weight)
    {
        return (this->*mBandwidthSelectCriterionFunction)(weight);
    }

private:
    QList<GwmVariable> mVariables;
    mat mDataPoints;

    int mK = 2;

    mat mX;
    vec mLatestWt;

    GwmBandwidthSizeSelector mSelector;
    BandwidthSelectionCriterionType mBandwidthSelectionCriterionType = BandwidthSelectionCriterionType::CV;
    BandwidthSelectCriterionFunction mBandwidthSelectCriterionFunction = &GwmGWPCATaskThread::bandwidthSizeCriterionCVSerial;
    bool mIsAutoselectBandwidth = false;

    // IOpenmpParallelable interface
    IParallelalbe::ParallelType mParallelType = IParallelalbe::ParallelType::SerialOnly;
    int mOmpThreadNum = 8;

    PcaLoadingsSdevScoresFunction mPcaLoadingsSdevScoresFunction = &GwmGWPCATaskThread::pcaLoadingsSdevScoresSerial;
    PcaLoadingsSdev mPcaLoadingsSdevFunction = &GwmGWPCATaskThread::pcaLoadingsSdevSerial;

    mat mLocalPV;
    mat mVariance;
    cube mLoadings;
    cube mScores;

    //用户选择是否Z-score标准化
    bool mZscore;
    //用户选择是否计算scores
    bool mScoresCal;


public:
    static int treeChildCount;

private:
    //用户选择RobustGWPCA
    bool mRobust;

    //用户选择GlyphPlot
    bool mPlot;

};

inline mat GwmGWPCATaskThread::variance() const
{
    return mVariance;
}

inline int GwmGWPCATaskThread::parallelAbility() const
{
    return IParallelalbe::SerialOnly
        #ifdef ENABLE_OpenMP
            | IParallelalbe::OpenMP
        #endif
            ;
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

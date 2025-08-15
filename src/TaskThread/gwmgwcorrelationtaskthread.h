    #ifndef GWMGWCORRELATIONTASKTHREAD_H
#define GWMGWCORRELATIONTASKTHREAD_H

#include <QObject>

#include "TaskThread/gwmspatialmultiscalealgorithm.h"
#include "TaskThread/imultivariableanalysis.h"
#include "TaskThread/iparallelable.h"
#include "TaskThread/gwmmultiscalegwralgorithm.h"

#include "SpatialWeight/gwmbandwidthweight.h"
#include "gwmbandwidthsizeselector.h"
#include "gwmspatialmultiscalealgorithm.h"
#include "iregressionanalysis.h"
#include "iparallelable.h"
#include "TaskThread/gwmbandwidthsizeselector.h"

class GwmGWcorrelationTaskThread;
//typedef double (GwmGWcorrelationTaskThread::*pfGwmCVApproach)(const mat& , GwmBandwidthWeight*);


class GwmGWcorrelationTaskThread : public GwmSpatialMultiscaleAlgorithm, public IBandwidthSizeSelectable,public IGwmMultivariableAnalysis, public IOpenmpParallelable
{
    Q_OBJECT

public:

    enum BackFittingCriterionType
    {
        CVR,
        dCVR
    };

    typedef double (GwmGWcorrelationTaskThread::*BandwidthSizeCriterionFunction)(GwmBandwidthWeight*);
//    typedef mat (GwmGWcorrelationTaskThread::*RegressionAllFunction)(const arma::mat&, const arma::vec&);
//    typedef vec (GwmGWcorrelationTaskThread::*RegressionVarFunction)(const arma::vec&, const arma::vec&, int, mat&);
//    typedef QPair<QString, const mat> CreateResultLayerDataItem;

public:
    static double covwt(const mat &x1, const mat &x2, const vec &w){
        return sum((sqrt(w) % (x1 - sum(x1 % w))) % (sqrt(w) % (x2 - sum(x2 % w)))) / (1 - sum(w % w));
    }

    static double corwt(const mat &x1, const mat &x2, const vec &w)
    {
        return covwt(x1,x2,w)/sqrt(covwt(x1,x1,w)*covwt(x2,x2,w));
    }

    static vec del(vec x,int rowcount);

    static vec rank(vec x)
    {
        vec n = linspace(0,x.n_rows-1,x.n_rows);
        vec res = n(sort_index(x));
        return n(sort_index(res));
    }

    static double AICc(const mat& x, const mat& y, const mat& betas, const vec& shat)
    {
        double ss = RSS(x, y, betas), n = x.n_rows;
        return n * log(ss / n) + n * log(2 * datum::pi) + n * ((n + shat(0)) / (n - 2 - shat(0)));
    }

    static double RSS(const mat& x, const mat& y, const mat& betas)
    {
        vec r = y - Fitted(x, betas);
        return sum(r % r);
    }

    static vec Fitted(const mat& x, const mat& betas)
    {
        return sum(betas % x, 1);
    }

public:
    static GwmEnumValueNameMapper<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> BandwidthInitilizeTypeNameMapper;

    enum GwmCVType{
        mean,
        median
    };

    typedef double (GwmGWcorrelationTaskThread::*CVFunction)(const mat& ,GwmBandwidthWeight*);
    typedef QList<QPair<QString, mat> > CreateResultLayerData;

    typedef bool (GwmGWcorrelationTaskThread::*CalFunction)();

    static GwmEnumValueNameMapper<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> BandwidthSelectionCriterionTypeNameMapper;


protected:
    GwmBandwidthSizeSelector selector;
    bool CalculateSerial();
#ifdef ENABLE_OpenMP
    bool CalculateOmp();
#endif
public:
    GwmGWcorrelationTaskThread();

    bool quantile() const;
    void setQuantile(bool quantile);

protected:  // QThread interface
    void run() override;

public:  // IGwmMultivariableAnalysis interface
    QList<GwmVariable> variables() const  override;
    void setVariables(const QList<GwmVariable> &variables)  override;
    void setVariables(const QList<GwmVariable> &&variables);

    QList<GwmVariable> variablesY() const{
        return mVariablesY;
    };
    void setVariablesY(const QList<GwmVariable> &variables);
    void setVariablesY(const QList<GwmVariable> &&variables);


public:  // IParallelalbe interface
    int parallelAbility() const override;
    virtual ParallelType parallelType() const override;
    virtual void setParallelType(const ParallelType& type) override;

public:  // IOpenmpParallelable interface
    void setOmpThreadNum(const int threadNum) override;

public:  // GwmSpatialAlgorithm interface
    bool isValid() override;

public:     // GwmTaskThread interface
    QString name() const override { return tr("GWCorrelations"); };


public:     // GwmSpatialMultiscaleAlgorithm interface
    virtual void setSpatialWeights(const QList<GwmSpatialWeight> &spatialWeights) override;

public:     // IBandwidthSizeSelectable interface
    double criterion(GwmBandwidthWeight* bandwidthWeight) override
    {
        return (this->*mBandwidthSelectCriterionFunction)(bandwidthWeight);
    }

    BandwidthSizeCriterionFunction mBandwidthSelectCriterionFunction = &GwmGWcorrelationTaskThread::bandwidthSizeCriterionVarCVSerial;
    GwmBandwidthSizeSelector mBandwidthSizeSelector;
    QList<bool> mIsAutoselectBandwidth;

    QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> bandwidthSelectionApproach() const;
    void setBandwidthSelectionApproach(const QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> &bandwidthSelectionApproach);
    QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> bandwidthInitilize() const;
    void setBandwidthInitilize(const QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> &bandwidthInitilize);

    GwmBandwidthWeight* bandwidth(int i)
    {
        return static_cast<GwmBandwidthWeight*>(mSpatialWeights[i].weight());
    }

    GwmGWcorrelationTaskThread::BandwidthSizeCriterionFunction bandwidthSizeCriterionVar(GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType type);
    double bandwidthSizeCriterionVarCVSerial(GwmBandwidthWeight* bandwidthWeight);
    double bandwidthSizeCriterionVarAICSerial(GwmBandwidthWeight* bandwidthWeight);
    int mBandwidthSelectionCurrentIndex = 0;

public:
    mat mDataPoints;
    GwmBandwidthWeight* bandwidth() const;
    void setBandwidth(GwmBandwidthWeight* bandwidth);
    mat localmean() const{return mLocalMean;}
    mat standarddev() const{return mStandardDev;}
    mat localskewness() const{return mLocalSkewness;}
    mat lcv() const{return mLCV;}
    mat lvar() const{return mLVar;}
    mat covmat() const{return mCovmat;}
    mat corrmat() const{return mCorrmat;}
    mat scorrmat() const{return mSCorrmat;}

    CreateResultLayerData resultlist() const{return mResultList;}

    int dataPointsSize() const
    {
        return mDataPoints.n_rows;
    }
protected:
    void initPoints();
    void initXY(mat& x,mat& y, const QList<GwmVariable>& indepVars,QList<GwmVariable>& indepVars2);

protected:  // GwmSpatialMonoscaleAlgorithm interface
    void createResultLayer(CreateResultLayerData data);

private:
    QList<GwmVariable> mVariables;
    QList<GwmVariable> mVariablesY;
    bool mQuantile = false;

    QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> mBandwidthSelectionApproach;
    QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> mBandwidthInitilize;

public:

protected:
//    mat mDataPoints;
    CalFunction mCalFunciton;

    mat mX;
    mat mY;
    mat mXi;
    mat mYi;
    IParallelalbe::ParallelType mParallelType = IParallelalbe::ParallelType::SerialOnly;
    int mOmpThreadNum = 8;

protected:
    mat mLocalMean;
    mat mStandardDev;
    mat mLocalSkewness;
    mat mLCV;
    mat mLVar;

    mat mLocalMedian;
    mat mIQR;
    mat mQI;

    mat mCovmat;
    mat mCorrmat;
    mat mSCorrmat;

    CreateResultLayerData mResultList;

public:
    static int treeChildCount;

};



inline bool GwmGWcorrelationTaskThread::quantile() const
{
    return mQuantile;
}

inline void GwmGWcorrelationTaskThread::setQuantile(bool quantile)
{
    mQuantile = quantile;
}

inline QList<GwmVariable> GwmGWcorrelationTaskThread::variables() const{
    return mVariables;
}

inline void GwmGWcorrelationTaskThread::setVariables(const QList<GwmVariable> &variables)
{
    mVariables = variables;
}

inline void GwmGWcorrelationTaskThread::setVariables(const QList<GwmVariable> &&variables)
{
    mVariables = variables;
}

inline void GwmGWcorrelationTaskThread::setVariablesY(const QList<GwmVariable> &variables)
{
    mVariablesY = variables;
}

inline void GwmGWcorrelationTaskThread::setVariablesY(const QList<GwmVariable> &&variables)
{
    mVariablesY = variables;
}


//inline void GwmGWcorrelationTaskThread::setBandwidth(GwmBandwidthWeight *bandwidth)
//{
//    mSpatialWeight.setWeight(bandwidth);
//}

inline int GwmGWcorrelationTaskThread::parallelAbility() const
{
    return IParallelalbe::SerialOnly
        #ifdef ENABLE_OpenMP
            | IParallelalbe::OpenMP
        #endif
            ;
}

inline IParallelalbe::ParallelType GwmGWcorrelationTaskThread::parallelType() const
{
    return mParallelType;
}

inline void GwmGWcorrelationTaskThread::setOmpThreadNum(const int threadNum)
{
    mOmpThreadNum = threadNum;
}

inline QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> GwmGWcorrelationTaskThread::bandwidthSelectionApproach() const
{
    return GwmGWcorrelationTaskThread::mBandwidthSelectionApproach;
}

inline void GwmGWcorrelationTaskThread::setBandwidthSelectionApproach(const QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> &bandwidthSelectionApproach)
{
    mBandwidthSelectionApproach = bandwidthSelectionApproach;
}

inline QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> GwmGWcorrelationTaskThread::bandwidthInitilize() const
{
    return GwmGWcorrelationTaskThread::mBandwidthInitilize;
}

inline void GwmGWcorrelationTaskThread::setBandwidthInitilize(const QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> &bandwidthInitilize)
{
    mBandwidthInitilize = bandwidthInitilize;
}


#endif // GWMGWCORRELATIONTASKTHREAD_H

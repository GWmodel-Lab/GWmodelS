    #ifndef GWMGWSSTASKTHREAD_H
#define GWMGWSSTASKTHREAD_H

#include <QObject>

#include "TaskThread/gwmspatialmonoscalealgorithm.h"
#include "TaskThread/imultivariableanalysis.h"
#include "TaskThread/iparallelable.h"

#include "SpatialWeight/gwmbandwidthweight.h"

#include "TaskThread/gwmbandwidthsizeselector.h"

class GwmGWSSTaskThread;
//typedef double (GwmGWSSTaskThread::*pfGwmCVApproach)(const mat& , GwmBandwidthWeight*);


class GwmGWSSTaskThread : public GwmSpatialMonoscaleAlgorithm, public IMultivariableAnalysis, public IOpenmpParallelable
{
    Q_OBJECT

public:
    static double covwt(const mat &x1, const mat &x2, const vec &w){
    //    vec wi = w/sum(w);
    //    double center1 = sum(x1 % wi);
    //    double center2 = sum(x2 % wi);
    //    vec n1 = sqrt(wi) % (x1 - center1);
    //    vec n2 = sqrt(wi) % (x2 - center2);
    //    double res = sum(n1 % n2) / (1 - sum(square(wi)));
    //    return res;
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

public:
    enum GwmCVType{
        mean,
        median
    };

    typedef double (GwmGWSSTaskThread::*CVFunction)(const mat& ,GwmBandwidthWeight*);
    typedef QList<QPair<QString, const mat> > CreateResultLayerData;

    typedef bool (GwmGWSSTaskThread::*CalFunction)();

protected:
    static vec findq(const mat& x, const vec& w);

    bool CalculateSerial();
    bool CalculateOmp();

public:
    GwmGWSSTaskThread();

    bool quantile() const;
    void setQuantile(bool quantile);

protected:  // QThread interface
    void run() override;

public:  // IMultivariableAnalysis interface
    QList<GwmVariable> variables() const;
    void setVariables(const QList<GwmVariable> &variables);
    void setVariables(const QList<GwmVariable> &&variables);

public:  // IParallelalbe interface
    int parallelAbility() const override;
    virtual ParallelType parallelType() const override;
    virtual void setParallelType(const ParallelType& type) override;

public:  // IOpenmpParallelable interface
    void setOmpThreadNum(const int threadNum) override;

public:  // GwmSpatialAlgorithm interface
    bool isValid() override;

public:     // GwmTaskThread interface
    QString name() const override { return tr("GWSS"); };

public:

    GwmBandwidthWeight* bandwidth() const;
    void setBandwidth(GwmBandwidthWeight* bandwidth);


    mat localmean() const{return mLocalMean;}
    mat standarddev() const{return mStandardDev;}
    mat localskewness() const{return mLocalSkewness;}
    mat lcv() const{return mLCV;}
    mat lvar() const{return mLVar;}

    mat localmedian() const{return mLocalMedian;}
    mat iqr() const{return mIQR;}
    mat qi() const{return mQI;}

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
    void initXY(mat& x, const QList<GwmVariable>& indepVars);

protected:  // GwmSpatialMonoscaleAlgorithm interface
    void createResultLayer(CreateResultLayerData data);

private:
    QList<GwmVariable> mVariables;
    bool mQuantile = false;

protected:
    mat mDataPoints;

//    GwmBandwidthWeight* mBandwidth;

    CalFunction mCalFunciton;

    mat mX;

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

};

inline bool GwmGWSSTaskThread::quantile() const
{
    return mQuantile;
}

inline void GwmGWSSTaskThread::setQuantile(bool quantile)
{
    mQuantile = quantile;
}

inline QList<GwmVariable> GwmGWSSTaskThread::variables() const{
    return mVariables;
}

inline void GwmGWSSTaskThread::setVariables(const QList<GwmVariable> &variables)
{
    mVariables = variables;
}

inline void GwmGWSSTaskThread::setVariables(const QList<GwmVariable> &&variables)
{
    mVariables = variables;
}

inline GwmBandwidthWeight *GwmGWSSTaskThread::bandwidth() const
{
    return static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight());
}

inline void GwmGWSSTaskThread::setBandwidth(GwmBandwidthWeight *bandwidth)
{
    mSpatialWeight.setWeight(bandwidth);
}

inline int GwmGWSSTaskThread::parallelAbility() const
{
    return IParallelalbe::SerialOnly | IParallelalbe::OpenMP;
}

inline IParallelalbe::ParallelType GwmGWSSTaskThread::parallelType() const
{
    return mParallelType;
}

inline void GwmGWSSTaskThread::setOmpThreadNum(const int threadNum)
{
    mOmpThreadNum = threadNum;
}


#endif // GWMGWSSTASKTHREAD_H

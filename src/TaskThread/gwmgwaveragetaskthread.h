    #ifndef GWMGWAVERAGETASKTHREAD_H
#define GWMGWAVERAGETASKTHREAD_H

#include <QObject>

#include "TaskThread/gwmspatialmonoscalealgorithm.h"
#include "TaskThread/imultivariableanalysis.h"
#include "TaskThread/iparallelable.h"

#include "SpatialWeight/gwmbandwidthweight.h"

#include "TaskThread/gwmbandwidthsizeselector.h"

class GwmGWaverageTaskThread;
//typedef double (GwmGWaverageTaskThread::*pfGwmCVApproach)(const mat& , GwmBandwidthWeight*);


class GwmGWaverageTaskThread :public GwmSpatialMonoscaleAlgorithm, public IGwmMultivariableAnalysis, public IOpenmpParallelable
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

    typedef double (GwmGWaverageTaskThread::*CVFunction)(const mat& ,GwmBandwidthWeight*);
    typedef QList<QPair<QString, mat> > CreateResultLayerData;

    typedef bool (GwmGWaverageTaskThread::*CalFunction)();

protected:
    static vec findq(const mat& x, const vec& w);

    bool CalculateSerial();
#ifdef ENABLE_OpenMP
    bool CalculateOmp();
#endif
public:
    GwmGWaverageTaskThread();

    bool quantile() const;
    void setQuantile(bool quantile);

protected:  // QThread interface
    void run() override;

public:  // IGwmMultivariableAnalysis interface
    QList<GwmVariable> variables() const override;
    void setVariables(const QList<GwmVariable> &variables) override;
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

    // mat covmat() const{return mCovmat;}
    // mat corrmat() const{return mCorrmat;}
    // mat scorrmat() const{return mSCorrmat;}

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

public:
    static int treeChildCount;

};



inline bool GwmGWaverageTaskThread::quantile() const
{
    return mQuantile;
}

inline void GwmGWaverageTaskThread::setQuantile(bool quantile)
{
    mQuantile = quantile;
}

inline QList<GwmVariable> GwmGWaverageTaskThread::variables() const{
    return mVariables;
}

inline void GwmGWaverageTaskThread::setVariables(const QList<GwmVariable> &variables)
{
    mVariables = variables;
}

inline void GwmGWaverageTaskThread::setVariables(const QList<GwmVariable> &&variables)
{
    mVariables = variables;
}

inline GwmBandwidthWeight *GwmGWaverageTaskThread::bandwidth() const
{
    return static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight());
}

inline void GwmGWaverageTaskThread::setBandwidth(GwmBandwidthWeight *bandwidth)
{
    mSpatialWeight.setWeight(bandwidth);
}

inline int GwmGWaverageTaskThread::parallelAbility() const
{
    return IParallelalbe::SerialOnly
        #ifdef ENABLE_OpenMP
            | IParallelalbe::OpenMP
        #endif
            ;
}

inline IParallelalbe::ParallelType GwmGWaverageTaskThread::parallelType() const
{
    return mParallelType;
}

inline void GwmGWaverageTaskThread::setOmpThreadNum(const int threadNum)
{
    mOmpThreadNum = threadNum;
}


#endif // GWMGWAVERAGETASKTHREAD_H

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


class GwmGWSSTaskThread : public GwmSpatialMonoscaleAlgorithm, public IMultivariableAnalysis//, public IOpenmpParallelable
{
    Q_OBJECT

    enum GwmCVType{
        mean,
        median
    };

    typedef double (GwmGWSSTaskThread::*CVFunction)(const mat& ,GwmBandwidthWeight*);
    typedef QList<QPair<QString, const mat> > CreateResultLayerData;

public:
    inline vec del(vec x,int rowcount){
        vec res;
        if(rowcount == 0)
            res = x.rows(rowcount+1,x.n_rows-1);
        else if(rowcount == x.n_rows-1)
            res = x.rows(0,x.n_rows-2);
        else
            res = join_cols(x.rows(0,rowcount - 1),x.rows(rowcount+1,x.n_rows-1));
        return res;
    }

    inline vec rank(vec x)
    {
        vec n = linspace(0,x.n_rows-1,x.n_rows);
        vec res = n(sort_index(x));
        return n(sort_index(res));
    }

public:
    GwmGWSSTaskThread();
    bool quantile() const;

    void setQuantile(bool quantile);
    mat getBWS() const{
        return mBWS;
    }

protected:
    double meanCV(const mat& x,GwmBandwidthWeight* bandwidthWeight);
    double medianCV(const mat& x,GwmBandwidthWeight* bandwidthWeight);

    double findmedian(const mat& x, const mat& w);

    mat findq(const mat& x, const mat& w);

    double gold(GwmCVType cvType,double xL, double xU, const mat& x);

    double covwt(mat x1, mat x2, mat w);

    double corwt(mat x1, mat x2, mat w);

protected:  // QThread interface
    void run() override;

public:  // IMultivariableAnalysis interface
    QList<GwmVariable> variables() const{
        return mVariables;
    }

    void setVariables(const QList<GwmVariable> &variables)
    {
        mVariables = variables;
    }

    void setVariables(const QList<GwmVariable> &&variables)
    {
        mVariables = variables;
    }

//public:  // IParallelalbe interface
//    int parallelAbility() const;
//    virtual ParallelType parallelType() const;
//    virtual void setParallelType(const ParallelType& type);

//public:  // IOpenmpParallelable interface
//    void setThreadNum(const int threadNum);

public:  // GwmSpatialAlgorithm interface
    bool isValid() override;

public:     // GwmTaskThread interface
    QString name() const override { return tr("GWSS"); };

public:

    GwmBandwidthWeight* bandwidth() const
    {
        return mBandwidth;
    }

    void setBandwidth(GwmBandwidthWeight* bandwidth)
    {
        mBandwidth = bandwidth;
    }

    inline QgsVectorLayer *regressionLayer() const
    {
        return mRegressionLayer;
    }

    inline void setRegressionLayer(QgsVectorLayer *layer)
    {
        mRegressionLayer = layer;
    }

    inline void setHasRegressionLayer(bool hasregressionlayer){
        mHasRegressionLayer = hasregressionlayer;
    }

    inline bool hasRegressionLayer()
    {
        return mHasRegressionLayer;
    }

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
    QgsVectorLayer * mRegressionLayer;
    mat mRegressionPoints;

    bool mHasRegressionLayer;

    GwmBandwidthWeight* mBandwidth;

    CVFunction mCVFunction;

    mat mX;

    mat mBWS;

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
    mat mSCorrnms;
    mat mSCorrmat;

};



#endif // GWMGWSSTASKTHREAD_H

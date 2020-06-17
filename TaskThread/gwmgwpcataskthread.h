#ifndef GWMGWPCATASKTHREAD_H
#define GWMGWPCATASKTHREAD_H

#include <QObject>
#include "TaskThread/gwmspatialmonoscalealgorithm.h"
#include "TaskThread/imultivariableanalysis.h"
#include "TaskThread/iparallelable.h"

#include "TaskThread/gwmbandwidthsizeselector.h"

class GwmGWPCATaskThread;
typedef double (GwmGWPCATaskThread::*pfGwmGWPCABandwidthSelectionApproach)(double ,const mat &, int , bool, int, bool);

class GwmGWPCATaskThread : public GwmSpatialMonoscaleAlgorithm, public IMultivariableAnalysis, public IOpenmpParallelable, public IBandwidthSizeSelectable
{
    Q_OBJECT
public:
    GwmGWPCATaskThread();

public:
    QList<GwmVariable> variables() const{return QList<GwmVariable>();};
    void setVariables(const QList<GwmVariable> &variables){};
    void setVariables(const QList<GwmVariable> &&variables){};

public:  // IParallelalbe interface
    int parallelAbility() const{return 0;};
    virtual ParallelType parallelType() const{return ParallelType::SerialOnly;};
    virtual void setParallelType(const ParallelType& type){};

public:  // IOpenmpParallelable interface
    void setThreadNum(const int threadNum){};

protected:  // GwmSpatialMonoscaleAlgorithm interface
    void createResultLayer();
    bool isValid(){return true;};

    void run() override;

private:
    QList<GwmVariable> mVariables;

    mat mDataPoints;
    void initPoints();
    void initXY(mat& x, const QList<GwmVariable>& indepVars);

public:
    //wpca函数
    void wpca(const mat &x, const vec &wt, double nu, double nv, mat &V, vec &S);
    //rwpca函数
    mat rwpca(const mat &x, const vec &wt, double nu, double nv);
    //gwpca.cv函数(k=2)
    double gwpcaCV(double bw, const mat &x, int k, bool robust, int kernel, bool adaptive){return 0;};
    //黄金分割函数
    double gold(pfGwmGWPCABandwidthSelectionApproach p,double xL, double xU, bool adaptBw, const mat &x, double k, bool robust, int kernel, bool adaptive);
    //带宽选择函数? 怎样加vars?
    double bwGWPCA(double k, bool robust, int kernel, bool adaptive);
    //
    double mk;
    bool mRobust;
    mat mX;
    double findMaxDistance()
    {
        int nDp = mDataPoints.n_rows;
        double maxD = 0.0;
        for (int i = 0; i < nDp; i++)
        {
            double d = max(mSpatialWeight.distance()->distance(mDataPoints.row(i), mDataPoints));
            maxD = d > maxD ? d : maxD;
        }
        return maxD;
    }
public:
    double criterion(GwmBandwidthWeight *weight);
};

#endif // GWMGWPCATASKTHREAD_H

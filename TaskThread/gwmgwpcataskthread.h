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
    typedef QList<QPair<QString, const mat> > CreateResultLayerData;

public:
    GwmGWPCATaskThread();

    QList<GwmVariable> variables() const{return QList<GwmVariable>();};
    void setVariables(const QList<GwmVariable> &variables);
    //void setVariables(const QList<GwmVariable> &&variables){};

    bool isAutoselectBandwidth() const;
    void setIsAutoselectBandwidth(bool isAutoselectBandwidth);

    BandwidthCriterionList GwmGWPCATaskThread::bandwidthSelectorCriterions() const
    {
        return mSelector.bandwidthCriterion();
    }

    double k() const;

    mat localPV() const;

    mat dResult1() const;

public:     // QThread interface
    void run() override;

public:     // GwmSpatialMonoscaleAlgorithm interface
    bool isValid(){return true;};

public:  // IParallelalbe interface
    int parallelAbility() const{return 0;};
    virtual ParallelType parallelType() const{return ParallelType::SerialOnly;};
    virtual void setParallelType(const ParallelType& type){};

public:  // IOpenmpParallelable interface
    void setThreadNum(const int threadNum){};
    void setOmpThreadNum(const int threadNum){};

public:     // IBandwidthSizeSelectable interface
    double criterion(GwmBandwidthWeight *weight);

private:
    void initPoints();
    void initXY(mat& x, const QList<GwmVariable>& indepVars);
    void wpca(const mat &x, const vec &wt, double nu, double nv, mat &V, vec &S);
    mat rwpca(const mat &x, const vec &wt, double nu, double nv);
    void createResultLayer(CreateResultLayerData data,QList<QString> winvar);
    void createResultLayer();

private:
    QList<GwmVariable> mVariables;
    mat mDataPoints;

    double mK=2;
    bool mRobust=false;
    mat mX;

    GwmBandwidthSizeSelector mSelector;

    bool mIsAutoselectBandwidth = false;


    mat mLocalPV;
    mat mDResult1;

};

inline mat GwmGWPCATaskThread::dResult1() const
{
    return mDResult1;
}

inline mat GwmGWPCATaskThread::localPV() const
{
    return mLocalPV;
}

inline double GwmGWPCATaskThread::k() const
{
    return mK;
}

#endif // GWMGWPCATASKTHREAD_H

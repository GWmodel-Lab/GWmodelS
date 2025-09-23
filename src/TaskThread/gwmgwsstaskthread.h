#ifndef GWMGWSSTASKTHREAD_H
#define GWMGWSSTASKTHREAD_H

#include <QObject>

#include <armadillo>
#include <gwmodel.h>

#include "TaskThread/gwmspatialmonoscalealgorithm.h"
#include "TaskThread/imultivariableanalysis.h"
#include "TaskThread/iparallelable.h"
#include "TaskThread/gwmbandwidthsizeselector.h"

#include "Model/gwmalgorithmmetagwss.h"

class GwmGWSSTaskThread;
//typedef double (GwmGWSSTaskThread::*pfGwmCVApproach)(const mat& , GwmBandwidthWeight*);


class GwmGWSSTaskThread : public GwmSpatialMonoscaleAlgorithm, public IMultivariableAnalysis, public IOpenmpParallelable
{
    Q_OBJECT

public:
    typedef QList<QPair<QString, mat> > CreateResultLayerData;

public:
    GwmGWSSTaskThread();

    GwmGWSSTaskThread(const GwmAlgorithmMetaGWSS& meta);

    GwmAlgorithmMetaGWSS meta() const { return mMeta; }

    bool quantile() const { return mAlgorithmAverage.quantile(); }

    QList<GwmVariable> variables() const override { return mVariables; }
    void setVariables(const QList<GwmVariable>& variables) override { mVariables = variables; }

    int parallelAbility() const override { return mAlgorithmAverage.parallelAbility(); }
    ParallelType parallelType() const override { return ParallelType(mAlgorithmAverage.parallelType()); }
    void setParallelType(const ParallelType& type) override { mAlgorithmAverage.setParallelType(gwm::ParallelType(type)); }
    void setOmpThreadNum(const int threadNum) override { mAlgorithmAverage.setOmpThreadNum(threadNum); }

    QString name() const override { return tr("GWSS"); };

    mat localmean() const{return mAlgorithmAverage.localMean();}
    mat standarddev() const{return mAlgorithmAverage.localSDev();}
    mat localskewness() const{return mAlgorithmAverage.localSkewness();}
    mat lcv() const{return mAlgorithmAverage.localCV();}
    mat lvar() const{return mAlgorithmAverage.localVar();}

    mat localmedian() const{return mAlgorithmAverage.localMedian();}
    mat iqr() const{return mAlgorithmAverage.iqr();}
    mat qi() const{return mAlgorithmAverage.qi();}

    mat covmat() const{return mAlgorithmCorrelation.localCov();}
    mat corrmat() const{return mAlgorithmCorrelation.localCorr();}
    mat scorrmat() const{return mAlgorithmCorrelation.localSCorr();}

    bool isValid() override { return mAlgorithmAverage.isValid() && mAlgorithmCorrelation.isValid(); }

    CreateResultLayerData resultlist() const { return mResultList; }
    
protected:  // QThread interface
    void run() override;
protected:
    mat initPoints(QgsVectorLayer* layer);
    mat initXY(QgsVectorLayer* layer, const QList<GwmVariable>& indepVars);

protected:  // GwmSpatialMonoscaleAlgorithm interface
    void createResultLayer(CreateResultLayerData data);

protected:
    GwmAlgorithmMetaGWSS mMeta;
    gwm::GWAverage mAlgorithmAverage;
    gwm::GWCorrelation mAlgorithmCorrelation;
    QgsVectorLayer* mLayer = nullptr;
    QList<GwmVariable> mVariables;
    CreateResultLayerData mResultList;

public:
    static int treeChildCount;

};

#endif // GWMGWSSTASKTHREAD_H

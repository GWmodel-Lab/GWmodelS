#ifndef GWMGWSSTASKTHREAD_H
#define GWMGWSSTASKTHREAD_H

#include <QObject>

#include <armadillo>
#include <gwmodel.h>

#include "TaskThread/gwmspatialmonoscalealgorithm.h"
#include "TaskThread/imultivariableanalysis.h"
#include "TaskThread/iparallelable.h"
#include "TaskThread/gwmbandwidthsizeselector.h"

#include "Model/gwmalgorithmmetagtdr.h"

class GwmGWSSTaskThread;
//typedef double (GwmGWSSTaskThread::*pfGwmCVApproach)(const mat& , GwmBandwidthWeight*);


class GwmGWSSTaskThread : public GwmSpatialMonoscaleAlgorithm, public IMultivariableAnalysis, public IOpenmpParallelable
{
    Q_OBJECT

public:
    typedef QList<QPair<QString, mat> > CreateResultLayerData;

public:
    GwmGWSSTaskThread();

    GwmGWSSTaskThread(const GwmAlgorithmMetaGTDR& meta);

    GwmAlgorithmMetaGTDR meta() const { return mMeta; }

    bool quantile() const { return mAlgorithm.quantile(); }

    QList<GwmVariable> variables() const override { return mVariables; }
    void setVariables(const QList<GwmVariable>& variables) override { mVariables = variables; }

    int parallelAbility() const override { return mAlgorithm.parallelAbility(); }
    ParallelType parallelType() const override { return ParallelType(mAlgorithm.parallelType()); }
    void setParallelType(const ParallelType& type) override { mAlgorithm.setParallelType(gwm::ParallelType(type)); }
    void setOmpThreadNum(const int threadNum) override { mAlgorithm.setOmpThreadNum(threadNum); }

    QString name() const override { return tr("GWSS"); };

    mat localmean() const{return mAlgorithm.localMean();}
    mat standarddev() const{return mAlgorithm.localSDev();}
    mat localskewness() const{return mAlgorithm.localSkewness();}
    mat lcv() const{return mAlgorithm.localCV();}
    mat lvar() const{return mAlgorithm.localVar();}

    mat localmedian() const{return mAlgorithm.localMedian();}
    mat iqr() const{return mAlgorithm.iqr();}
    mat qi() const{return mAlgorithm.qi();}

    mat covmat() const{return mAlgorithm.localCov();}
    mat corrmat() const{return mAlgorithm.localCorr();}
    mat scorrmat() const{return mAlgorithm.localSCorr();}

    bool isValid() override { return mAlgorithm.isValid(); }

    CreateResultLayerData resultlist() const { return mResultList; }
    
protected:  // QThread interface
    void run() override;
protected:
    mat initPoints(QgsVectorLayer* layer);
    mat initXY(QgsVectorLayer* layer, const QList<GwmVariable>& indepVars);

protected:  // GwmSpatialMonoscaleAlgorithm interface
    void createResultLayer(CreateResultLayerData data);

protected:
    GwmAlgorithmMetaGTDR mMeta;
    gwm::GWSS mAlgorithm;
    QgsVectorLayer* mLayer = nullptr;
    QList<GwmVariable> mVariables;
    CreateResultLayerData mResultList;

public:
    static int treeChildCount;

};

#endif // GWMGWSSTASKTHREAD_H

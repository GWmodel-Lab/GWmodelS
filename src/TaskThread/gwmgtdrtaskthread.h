#ifndef GWMGTDRTASKTHREAD_H
#define GWMGTDRTASKTHREAD_H

#include <QObject>

#include <armadillo>
#include <gwmodel.h>

#include "TaskThread/iregressionanalysis.h"
#include "TaskThread/gwmspatialalgorithm.h"
#include "TaskThread/iparallelable.h"
#include "TaskThread/gwmbandwidthsizeselector.h"

#include "Model/gwmalgorithmmetagtdr.h"

class GwmGTDRTaskThread;
//typedef double (GwmGTDRTaskThread::*pfGwmCVApproach)(const mat& , GwmBandwidthWeight*);

class GwmGTDRTaskThread : public GwmSpatialAlgorithm, public IOpenmpParallelable /*, public IRegressionAnalysis*/
{
    Q_OBJECT

public:
    typedef QList<QPair<QString, mat> > CreateResultLayerData;

public:
    GwmGTDRTaskThread();

    GwmGTDRTaskThread(const GwmAlgorithmMetaGTDR& meta);

    GwmAlgorithmMetaGTDR meta() const { return mMeta; }

    //QList<GwmVariable> independentVariables() const  { return mIndepVars; }
    //void setIndependentVariables(const QList<GwmVariable>& indepVars)  { mIndepVars = indepVars; }

    //GwmVariable dependentVariable() const  { return mDepVar; }
    //void setDependentVariable(const GwmVariable& depVar)  { mDepVar = depVar; }

    int parallelAbility() const override { return mAlgorithm.parallelAbility(); }
    ParallelType parallelType() const override { return ParallelType(mAlgorithm.parallelType()); }
    void setParallelType(const ParallelType& type) override { mAlgorithm.setParallelType(gwm::ParallelType(type)); }
    void setOmpThreadNum(const int threadNum) override { mAlgorithm.setOmpThreadNum(threadNum); }

    const gwm::GTDR& algorithm() const { return mAlgorithm; }
    std::vector<gwm::SpatialWeight> spatialWeights() const { return mAlgorithm.spatialWeights(); }

    // QString name() const override { return tr("GTDR"); };

    mat betas() const { return mAlgorithm.betas(); }

    mat betasSE() { return mAlgorithm.betasSE(); }
    vec sHat() { return mAlgorithm.sHat(); }
    vec qDiag() { return mAlgorithm.qDiag(); }
    mat s() { return mAlgorithm.s(); }
    bool hasHatMatrix() const { return mAlgorithm.hasHatMatrix(); }

    bool isValid() override { return mAlgorithm.isValid(); }

    GwmDiagnostic diagnostic() const
    {
        return toGwmDiagnostic(mDiagnostic);
    }

    CreateResultLayerData resultlist() const { return mResultList; }
    
protected:  // QThread interface
    void run() override;
protected:
    mat initPoints(QgsVectorLayer* layer);
    void initXY(mat &x, mat &y, const GwmVariable &depVar, const QList<GwmVariable> &indepVars);

protected:  // GwmSpatialMonoscaleAlgorithm interface
    void createResultLayer(CreateResultLayerData data);

private:
    inline GwmDiagnostic toGwmDiagnostic(const gwm::RegressionDiagnostic& diag) const
    {
        return GwmDiagnostic{diag.RSS, diag.AIC, diag.AICc,
            diag.ENP, diag.EDF, diag.RSquare, diag.RSquareAdjust};
    }

protected:
    GwmAlgorithmMetaGTDR mMeta;

    mat mX;
    vec mY;
    mat mBetas;
    mat mBetasSE;

    // GwmDiagnostic mDiagnostic;
    gwm::RegressionDiagnostic mDiagnostic;
    std::unique_ptr<gwm::GTDR> mGTDRCore;

    gwm::GTDR mAlgorithm;
    QgsVectorLayer* mLayer = nullptr;
    QList<GwmVariable> mIndepVars;
    GwmVariable mDepVar;
    CreateResultLayerData mResultList;

public:
    static int treeChildCount;
private:
    std::vector<std::unique_ptr<gwm::BandwidthWeight>> mBandwidthHolders;
    std::vector<std::unique_ptr<gwm::OneDimDistance>> mDistanceHolders;

    gwm::BandwidthSelector mBandwidthSizeSelector;
    //gwm::GTDRBandwidthOptimizer mGTDRBandwidthOptimizer;
    int mCurrentOptimizingDim = -1;

    std::vector<gwm::SpatialWeight> mSpatialWeights;

    // 带宽优化准则方法（实现 IBandwidthSizeSelectable 接口）
    double criterion(GwmBandwidthWeight* weight);

    // 为特定维度优化带宽
    void optimizeBandwidthForDimension(size_t dimIndex);

    // 计算 AIC 准则（串行版本）
    double bandwidthSizeCriterionAICSerial(GwmBandwidthWeight* bandwidthWeight, size_t dimIndex);

    // 计算 CV 准则（串行版本）
    double bandwidthSizeCriterionCVSerial(GwmBandwidthWeight* bandwidthWeight, size_t dimIndex);

};

#endif // GWMGTDRTASKTHREAD_H

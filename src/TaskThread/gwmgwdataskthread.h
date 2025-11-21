#ifndef GWMGWDATASKTHREAD_H
#define GWMGWDATASKTHREAD_H

#include <QObject>
#include <armadillo>
#include <gwmodel.h>

#include "TaskThread/gwmspatialmonoscalealgorithm.h"
#include "TaskThread/imultivariableanalysis.h"
#include "TaskThread/iparallelable.h"
#include "TaskThread/gwmbandwidthsizeselector.h"

#include "Model/gwmalgorithmmetavariable.h"
#include "Model/gwmvariableitemmodel.h"
#include <qgsvectorlayer.h>

class GwmGWDATaskThread: public GwmSpatialMonoscaleAlgorithm, public IGwmMultivariableAnalysis, public IOpenmpParallelable
{
    Q_OBJECT

public:
    GwmGWDATaskThread();

    // 实现 IGwmMultivariableAnalysis 接口
    QList<GwmVariable> variables() const override { return mVariables; }
    void setVariables(const QList<GwmVariable>& variables) override { mVariables = variables; }

    // 实现 IOpenmpParallelable 接口
    int parallelAbility() const override { return mAlgorithm.parallelAbility(); }
    ParallelType parallelType() const override { return ParallelType(mAlgorithm.parallelType()); }
    void setParallelType(const ParallelType& type) override { mAlgorithm.setParallelType(gwm::ParallelType(type)); }
    void setOmpThreadNum(const int threadNum) override { mAlgorithm.setOmpThreadNum(threadNum); }

    // 实现 GwmSpatialAlgorithm 接口
    bool isValid() override { return mAlgorithm.isValid(); }

    QString name() const override { return tr("GWDA"); }

protected:  // QThread interface
    void run() override;

protected:
    // 添加必要的成员变量
    gwm::GWDA mAlgorithm;  // 底层算法对象
    GwmVariable mGroupVariable;
    QList<GwmVariable> mVariables;  // 变量列表
    QgsVectorLayer* mLayer = nullptr;  // 数据图层

public:
    void setDataLayer(QgsVectorLayer* layer);
    void setGroupVariable(const GwmVariable& groupVar);
    void setIndependentVariables(const QList<GwmVariable>& indepVars);
    void setSpatialWeight(const GwmSpatialWeight& spatialWeight);
    void setIsWqda(bool isWqda);
    void setHascov(bool hasCov);
    void setHasmean(bool hasMean);
    void setHasprior(bool hasPrior);

    GwmVariable groupVariable() const { return mGroupVariable; }
    QList<GwmVariable> independentVariables() const { return mVariables; }
    QgsVectorLayer* dataLayer() const { return mLayer; }
    double correctRate() const { return mAlgorithm.correctRate(); }
    bool isWqda() const { return mAlgorithm.isWqda(); }
    bool hasCov() const { return mAlgorithm.hasCov(); }
    bool hasMean() const { return mAlgorithm.hasMean(); }
    bool hasPrior() const { return mAlgorithm.hasPrior(); }

    static int treeChildCount;
};

#endif // GWMGWDATASKTHREAD_H

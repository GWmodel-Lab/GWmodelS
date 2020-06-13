#ifndef GWMLAYERBASICGWRITEM_H
#define GWMLAYERBASICGWRITEM_H

#include "prefix.h"

#include <QObject>
#include <qgsvectorlayer.h>
#include <armadillo>
#include "gwmlayervectoritem.h"
#include "Model/gwmlayerattributeitem.h"
#include "TaskThread/gwmgwrtaskthread.h"
#include <TaskThread/gwmbasicgwralgorithm.h>

class GwmLayerBasicGWRItem : public GwmLayerVectorItem
{
    Q_OBJECT
public:
    explicit GwmLayerBasicGWRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmBasicGWRAlgorithm* taskThread = nullptr);

    virtual int childNumber() override;

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::GWR; }

    int dataPointsSize() const;

    GwmDiagnostic diagnostic() const;

    arma::mat betas() const;

    bool modelOptimized() const;

    bool bandwidthOptimized() const;

    bool hatmatrix() const;

    bool fTest() const;

    bool regressionPointGiven() const;

    GwmVariable depVar() const;

    QList<GwmVariable> indepVars() const;

    GwmBasicGWRAlgorithm::FTestResultPack fTestResults() const;

    QList<QPair<QList<GwmVariable>, double> > modelSelModels() const;

    QList<QPair<double, double> > bandwidthSelScores() const;

    GwmBandwidthWeight weight() const;

protected:
    int mDataPointsSize;
    GwmVariable mDepVar;
    QList<GwmVariable> mIndepVars;
    GwmBandwidthWeight mWeight;
    GwmDiagnostic mDiagnostic;
    arma::mat mBetas;
    QList<QPair<QList<GwmVariable>, double> > mModelSelModels;
    QList<QPair<double, double> > mBandwidthSelScores;
    GwmBasicGWRAlgorithm::FTestResultPack mFTestResults;

    bool isRegressionPointGiven;
    bool isModelOptimized;
    bool isBandwidthOptimized;
    bool hasHatmatrix;
    bool hasFTest;
};

#endif // GWMLAYERBASICGWRITEM_H

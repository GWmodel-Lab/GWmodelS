#ifndef GWMLAYERGTDRITEM_H
#define GWMLAYERGTDRITEM_H

#include "gwmlayervectoritem.h"
#include "TaskThread/gwmgtdrtaskthread.h"
#include "TaskThread/gwmgwaveragetaskthread.h"
#include "TaskThread/gwmgwcorrelationtaskthread.h"
#include "TaskThread/gwmmultiscalegwralgorithm.h"

class GwmLayerGTDRItem : public GwmLayerVectorItem
{
public:

    explicit GwmLayerGTDRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmGTDRTaskThread* taskThread = nullptr);

    virtual int childNumber() override;

    inline virtual GwmLayerItemType itemType() override { return GwmLayerItemType::GTDR; }

    virtual bool readXml(QDomNode &node) override;
    virtual bool writeXml(QDomNode &node, QDomDocument &doc) override;

    int dataPointsSize() const;

    GwmDiagnostic diagnostic() const {return mDiagnostic;};

    arma::mat betas() const {return mBetas;};

    bool modelOptimized() const {return isModelOptimized;};

    bool bandwidthOptimized() const {return isBandwidthOptimized;};

    bool isBandwidthOptimizationSuccessful() const {return mIsBandwidthOptimizationSuccessful;};

    bool hatmatrix() const {return hasHatmatrix;};

    // bool fTest() const;

    // bool regressionPointGiven() const;

    // bool ols() const;

    // GwmGTDRAlgorithm::FTestResultPack fTestResults() const;

    QList<QPair<QList<GwmVariable>, double> > modelSelModels() const;

    QList<QPair<double, double> > bandwidthSelScores() const;

    GwmBandwidthWeight weight() const;

    // GwmGTDRAlgorithm::OLSVar OLSResults() const;

    // GwmLayerGTDRItem(GwmLayerItem* parentItem, QgsVectorLayer* vector, const GwmGTDRTaskThread* taskThread);
    ~GwmLayerGTDRItem();

    GwmGTDRTaskThread::CreateResultLayerData resultlist() const{return mResultList;}


    GwmVariable depVar() const
    {
        return mDepVar;
    }

    QList<GwmVariable> indepVar() const
    {
        return mIndepVars;
    }

    QList<GwmVariable> weightingVar() const
    {
        return mWeightingVars;
    }

    GwmBandwidthWeight* bandwidth() const
    {
        return mBandwidth;
    }

    QList<GwmBandwidthWeight*> bandwidths() const
    {
        return mBandwidths;
    }

protected:

    int mDataPointsSize;
    GwmVariable mDepVar;
    QList<GwmVariable> mIndepVars;
    QList<GwmVariable> mWeightingVars;
    GwmBandwidthWeight mWeight;
    GwmDiagnostic mDiagnostic;
    arma::mat mBetas;

    GwmBandwidthWeight* mBandwidth;
    QList<GwmBandwidthWeight*> mBandwidths;//因为gtdr带宽是多维的，理论上只用它而不用mBandwidth

    QList<QPair<QList<GwmVariable>, double> > mModelSelModels;
    QList<QPair<double, double> > mBandwidthSelScores;
    // GwmGTDRAlgorithm::FTestResultPack mFTestResults;
    // GwmGTDRAlgorithm::OLSVar mOLSVar;
    bool isRegressionPointGiven;
    bool isModelOptimized;
    bool isBandwidthOptimized;
    bool hasHatmatrix;
    bool mIsBandwidthOptimizationSuccessful;
    // bool hasFTest;
    // bool hasols;

    GwmGTDRTaskThread::CreateResultLayerData mResultList;
};

#endif // GWMLAYERGTDRITEM_H

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

    arma::mat betas() const;

    bool modelOptimized() const {return isModelOptimized;};

    bool bandwidthOptimized() const {return isBandwidthOptimized;};

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

    GwmBandwidthWeight* bandwidth() const
    {
        return mBandwidth;
    }

protected:

    int mDataPointsSize;
    GwmVariable mDepVar;
    QList<GwmVariable> mIndepVars;
    GwmBandwidthWeight mWeight;
    GwmDiagnostic mDiagnostic;
    arma::mat mBetas;

    GwmBandwidthWeight* mBandwidth;

    QList<QPair<QList<GwmVariable>, double> > mModelSelModels;
    QList<QPair<double, double> > mBandwidthSelScores;
    // GwmGTDRAlgorithm::FTestResultPack mFTestResults;
    // GwmGTDRAlgorithm::OLSVar mOLSVar;
    bool isRegressionPointGiven;
    bool isModelOptimized;
    bool isBandwidthOptimized;
    bool hasHatmatrix;
    // bool hasFTest;
    // bool hasols;

    GwmGTDRTaskThread::CreateResultLayerData mResultList;
};

#endif // GWMLAYERGTDRITEM_H

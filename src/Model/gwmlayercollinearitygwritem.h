#ifndef GWMLAYERCOLLINEARUTYGWRITEM_H
#define GWMLAYERCOLLINEARUTYGWRITEM_H

#include "prefix.h"

#include <QObject>
#include <qgsvectorlayer.h>
#include <armadillo>
#include "gwmlayervectoritem.h"
#include "Model/gwmlayerattributeitem.h"
//#include "TaskThread/gwmgwrtaskthread.h"
#include "TaskThread/gwmlocalcollinearitygwralgorithm.h"

class GwmLayerCollinearityGWRItem : public GwmLayerVectorItem
{
    Q_OBJECT
public:
    explicit GwmLayerCollinearityGWRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmLocalCollinearityGWRAlgorithm* taskThread = nullptr);

    virtual int childNumber() override;

    inline virtual GwmLayerItemType itemType() override { return GwmLayerItemType::CollinearityGWR; }

    virtual bool readXml(QDomNode &node) override;
    virtual bool writeXml(QDomNode &node, QDomDocument &doc) override;

    int dataPointsSize() const;

    GwmVariable depVar() const;

    QList<GwmVariable> indepVars() const;

    gwm::BandwidthWeight weight() const;

    GwmDiagnostic diagnostic() const;

    gwm::RegressionDiagnostic diagnostic0() const;

    arma::mat betas() const;

    BandwidthCriterionList bandwidthSelScores() const;

    bool getIsRegressionPointGiven() const;

    bool getIsBandwidthOptimized() const;

    bool getHasHatmatrix() const;



    double getLambda() const;

    double getMcnThresh() const;

protected:
    int mDataPointsSize;
    GwmVariable mDepVar;
    QList<GwmVariable> mIndepVars;
    gwm::BandwidthWeight mWeight;
    GwmDiagnostic mDiagnostic;
    gwm::RegressionDiagnostic mDiagnostic0;
    arma::mat mBetas;
    //QList<QPair<QList<GwmVariable>, double> > mModelSelModels;
    BandwidthCriterionList mBandwidthSelScores;
    //GwmBasicGWRAlgorithm::FTestResultPack mFTestResults;

    bool isRegressionPointGiven;
    //bool isModelOptimized;
    bool isBandwidthOptimized;
    bool hasHatmatrix;
    //bool hasFTest;
    double mcnThresh;

    double mLambda;
};

#endif // GWMLAYERGWRITEM_H

#ifndef GWMLAYERCOLLINEARUTYGWRITEM_H
#define GWMLAYERCOLLINEARUTYGWRITEM_H

#include "prefix.h"

#include <QObject>
#include <qgsvectorlayer.h>
#include <armadillo>
#include "gwmlayervectoritem.h"
#include "Model/gwmlayerattributeitem.h"
//#include "TaskThread/gwmgwrtaskthread.h"
#include "TaskThread/gwmlcrgwrtaskthread.h"

class GwmLayerCollinearityGWRItem : public GwmLayerVectorItem
{
    Q_OBJECT
public:
    explicit GwmLayerCollinearityGWRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmLcrGWRTaskThread* taskThread = nullptr);

    virtual int childNumber() override;

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::CollinearityGWR; }

    int dataPointsSize() const;

    GwmVariable depVar() const;

    QList<GwmVariable> indepVars() const;

    GwmBandwidthWeight weight() const;

    GwmDiagnostic diagnostic() const;

    arma::mat betas() const;

    QList<QPair<double, double> > bandwidthSelScores() const;

    bool getIsRegressionPointGiven() const;

    bool getIsBandwidthOptimized() const;

    bool getHasHatmatrix() const;



    double getLambda() const;

    double getMcnThresh() const;

protected:
    int mDataPointsSize;
    GwmVariable mDepVar;
    QList<GwmVariable> mIndepVars;
    GwmBandwidthWeight mWeight;
    GwmDiagnostic mDiagnostic;
    arma::mat mBetas;
    //QList<QPair<QList<GwmVariable>, double> > mModelSelModels;
    QList<QPair<double, double> > mBandwidthSelScores;
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

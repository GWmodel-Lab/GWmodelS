#ifndef GWMLAYERGTWRITEM_H
#define GWMLAYERGTWRITEM_H

#include "prefix.h"

#include <QObject>
#include <qgsvectorlayer.h>
#include <armadillo>
#include "gwmlayervectoritem.h"
#include "Model/gwmlayerattributeitem.h"
#include "TaskThread/gwmgwrtaskthread.h"
#include "TaskThread/gwmgtwralgorithm.h"

class GwmLayerGTWRItem : public GwmLayerVectorItem
{
    Q_OBJECT
public:
    explicit GwmLayerGTWRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmGTWRAlgorithm* taskThread = nullptr);

    virtual int childNumber() override;

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::GTWR; }

    virtual bool readXml(QDomNode &node) override;
    virtual bool writeXml(QDomNode &node, QDomDocument &doc) override;

    int dataPointsSize() const;

    GwmDiagnostic diagnostic() const;

    arma::mat betas() const;

    bool bandwidthOptimized() const;

    bool hatmatrix() const;

    bool regressionPointGiven() const;

    GwmVariable depVar() const;

    QList<GwmVariable> indepVars() const;

    QList<QPair<double, double> > bandwidthSelScores() const;

    GwmBandwidthWeight weight() const;

protected:
    int mDataPointsSize;
    GwmVariable mDepVar;
    QList<GwmVariable> mIndepVars;
    GwmBandwidthWeight mWeight;
    GwmDiagnostic mDiagnostic;
    arma::mat mBetas;
    QList<QPair<double, double> > mBandwidthSelScores;

    bool isRegressionPointGiven;
    bool isBandwidthOptimized;
    bool hasHatmatrix;
};

#endif // GWMLAYERGTWRITEM_H

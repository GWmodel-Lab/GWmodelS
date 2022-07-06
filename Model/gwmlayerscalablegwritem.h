#ifndef GWMLAYERSCALABLEGWRITEM_H
#define GWMLAYERSCALABLEGWRITEM_H

#include "Model/gwmlayerbasicgwritem.h"
#include "TaskThread/gwmscalablegwralgorithm.h"

#include <SpatialWeight/gwmbandwidthweight.h>

class GwmLayerScalableGWRItem : public GwmLayerVectorItem
{
    Q_OBJECT
public:
    GwmLayerScalableGWRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmScalableGWRAlgorithm* taskThread = nullptr);

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::ScalableGWR; }

    virtual bool readXml(QDomNode &node) override;
    virtual bool writeXml(QDomNode &node, QDomDocument &doc) override;

    double cv() const;

    double scale() const;

    double penalty() const;

    int polynomial() const;

    int dataPointsSize() const;

    GwmVariable depVar() const;

    QList<GwmVariable> indepVars() const;

    GwmBandwidthWeight weight() const;

    GwmDiagnostic diagnostic() const;

    arma::mat betas() const;

    GwmDistance::DistanceType distanceType() const;

    GwmScalableGWRAlgorithm::ParameterOptimizeCriterionType parameterOptimizeCriterionType() const;

    bool hasRegressionLayer() const;

    bool hasPredict() const;

private:
    int mDataPointsSize;
    GwmVariable mDepVar;
    QList<GwmVariable> mIndepVars;
    GwmBandwidthWeight mWeight;
    GwmDistance::DistanceType mDistanceType;
    GwmDiagnostic mDiagnostic;
    arma::mat mBetas;
    GwmScalableGWRAlgorithm::ParameterOptimizeCriterionType mParameterOptimizeCriterionType;
    bool mHasRegressionLayer = false;
    bool mHasPredict = false;

    int mPolynomial;
    double mCV;
    double mScale;
    double mPenalty;
};

#endif // GWMLAYERSCALABLEGWRITEM_H

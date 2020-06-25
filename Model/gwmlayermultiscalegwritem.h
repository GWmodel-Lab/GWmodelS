#ifndef GWMLAYERMULTISCALEGWRITEM_H
#define GWMLAYERMULTISCALEGWRITEM_H

#include "prefix.h"

#include <QObject>
#include <qgsvectorlayer.h>
#include <armadillo>
#include "gwmlayervectoritem.h"
#include "Model/gwmlayerattributeitem.h"
#include "TaskThread/gwmmultiscalegwralgorithm.h"

class GwmLayerMultiscaleGWRItem : public GwmLayerVectorItem
{
    Q_OBJECT
public:
    explicit GwmLayerMultiscaleGWRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmMultiscaleGWRAlgorithm* taskThread = nullptr);

    virtual int childNumber() override;

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::MultiscaleGWR; }

    int dataPointsSize() const;

    QList<bool> preditorCentered() const;

    GwmDiagnostic diagnostic() const;

    arma::mat betas() const;

    bool hasHatmatrix() const;

    QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> bandwidthInitilize() const;

    QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> bandwidthSelectionApproach() const;

    QList<double> bandwidthSelectThreshold() const;

    GwmMultiscaleGWRAlgorithm::BackFittingCriterionType criterionType() const;

    QList<GwmBandwidthWeight> bandwidthWeights() const;

    QList<GwmDistance::DistanceType> distaneTypes() const;

    GwmVariable depVar() const;

    QList<GwmVariable> indepVars() const;

private:
    int mDataPointsSize;
    GwmVariable mDepVar;
    QList<GwmVariable> mIndepVars;
    QList<GwmBandwidthWeight> mBandwidthWeights;
    QList<GwmDistance::DistanceType> mDistaneTypes;
    QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> mBandwidthInitilize;
    QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> mBandwidthSelectionApproach;
    QList<bool> mPreditorCentered;
    QList<double> mBandwidthSelectThreshold;
    GwmMultiscaleGWRAlgorithm::BackFittingCriterionType mCriterionType;

    GwmDiagnostic mDiagnostic;
    arma::mat mBetas;

    bool mHasHatmatrix;
};

#endif // GWMLAYERGWRITEM_H

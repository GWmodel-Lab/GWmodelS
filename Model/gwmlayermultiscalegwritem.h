#ifndef GWMLAYERMULTISCALEGWRITEM_H
#define GWMLAYERMULTISCALEGWRITEM_H

#include "prefix.h"

#include <QObject>
#include <qgsvectorlayer.h>
#include <armadillo>
#include "gwmlayervectoritem.h"
#include "Model/gwmlayerattributeitem.h"
#include "TaskThread/gwmmultiscalegwrtaskthread.h"

class GwmLayerMultiscaleGWRItem : public GwmLayerVectorItem
{
    Q_OBJECT
public:
    explicit GwmLayerMultiscaleGWRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmMultiscaleGWRTaskThread* taskThread = nullptr);

    virtual int childNumber() override;

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::MultiscaleGWR; }

    int dataPointsSize() const;

    QList<bool> preditorCentered() const;

    GwmDiagnostic diagnostic() const;

    arma::mat betas() const;

    bool hasHatmatrix() const;

    QList<GwmMultiscaleGWRTaskThread::BandwidthInitilizeType> bandwidthInitilize() const;

    QList<GwmMultiscaleGWRTaskThread::BandwidthSelectionCriterionType> bandwidthSelectionApproach() const;

    QList<double> bandwidthSelectThreshold() const;

    GwmMultiscaleGWRTaskThread::BackFittingCriterionType criterionType() const;

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
    QList<GwmMultiscaleGWRTaskThread::BandwidthInitilizeType> mBandwidthInitilize;
    QList<GwmMultiscaleGWRTaskThread::BandwidthSelectionCriterionType> mBandwidthSelectionApproach;
    QList<bool> mPreditorCentered;
    QList<double> mBandwidthSelectThreshold;
    GwmMultiscaleGWRTaskThread::BackFittingCriterionType mCriterionType;

    GwmDiagnostic mDiagnostic;
    arma::mat mBetas;

    bool mHasHatmatrix;
};

#endif // GWMLAYERGWRITEM_H

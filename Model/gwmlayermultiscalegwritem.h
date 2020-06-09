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

    int depVarIndex() const;

    QList<int> indepVarsIndex() const;

    QList<GwmLayerAttributeItem *> indepVarsOrigin() const;

    vec bandwidthSize() const;

    QList<QString> bandwidthUnit() const;

    QList<GwmMultiscaleGWRTaskThread::BandwidthSeledType> bandwidthSeled() const;

    QList<GwmGWRTaskThread::BandwidthType> bandwidthType() const;

    QList<GwmGWRTaskThread::BandwidthSelectionApproach> bandwidthSelectionApproach() const;

    QList<GwmGWRTaskThread::KernelFunction> bandwidthKernelFunction() const;

    QList<GwmGWRTaskThread::DistanceSourceType> distanceSource() const;

    QList<QVariant> distSrcParameters() const;

    QList<bool> preditorCentered() const;

    vec bandwidthSelectThreshold() const;

    GwmGWRDiagnostic diagnostic() const;

    arma::mat betas() const;

    bool getHasHatmatrix() const;

private:
    int mDataPointsSize;
    int mDepVarIndex;
    QList<int> mIndepVarsIndex;
    QList<GwmLayerAttributeItem*> mIndepVarsOrigin;
    vec mBandwidthSize;
    QList<QString> mBandwidthUnit;
    QList<GwmMultiscaleGWRTaskThread::BandwidthSeledType> mBandwidthSeled;
    QList<GwmGWRTaskThread::BandwidthType> mBandwidthType;
    QList<GwmGWRTaskThread::BandwidthSelectionApproach> mBandwidthSelectionApproach;
    QList<GwmGWRTaskThread::KernelFunction> mBandwidthKernelFunction;
    QList<GwmGWRTaskThread::DistanceSourceType> mDistanceSource;
    QList<QVariant> mDistSrcParameters;
    QList<bool> mPreditorCentered;
    vec mBandwidthSelectThreshold;

    GwmGWRDiagnostic mDiagnostic;
    arma::mat mBetas;

    bool hasHatmatrix;
};

#endif // GWMLAYERGWRITEM_H

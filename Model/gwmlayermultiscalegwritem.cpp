#include "gwmlayermultiscalegwritem.h"
#include "gwmlayergroupitem.h"

GwmLayerMultiscaleGWRItem::GwmLayerMultiscaleGWRItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmMultiscaleGWRTaskThread* taskThread)
    : GwmLayerVectorItem(parent, vector)
{
    if (taskThread)
    {
        mDataPointsSize = taskThread->getFeatureList().size();
        mDepVarIndex = taskThread->getDepVarIndex();
        mIndepVarsIndex = taskThread->getIndepVarsIndex();
        mIndepVarsOrigin = taskThread->indepVars();
        mBandwidthSize = taskThread->initialBandwidthSize();
        mBandwidthUnit = taskThread->bandwidthUnit();
        mBandwidthSeled = taskThread->bandwidthSeled();
        mBandwidthType = taskThread->bandwidthType();
        mBandwidthSelectionApproach = taskThread->bandwidthSelectionApproach();
        mBandwidthKernelFunction = taskThread->bandwidthKernel();
        mDistanceSource = taskThread->distanceSource();
        mDistSrcParameters = taskThread->distanceParameter();
        mPreditorCentered = taskThread->preditorCentered();
        mDiagnostic = taskThread->getDiagnostic();
        mBetas = mat(taskThread->getBetas());
        hasHatmatrix = taskThread->getHasHatMatrix();
    }
}

int GwmLayerMultiscaleGWRItem::childNumber()
{
    if (mParentItem)
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this);
    return 0;
}

int GwmLayerMultiscaleGWRItem::dataPointsSize() const
{
    return mDataPointsSize;
}

int GwmLayerMultiscaleGWRItem::depVarIndex() const
{
    return mDepVarIndex;
}

QList<int> GwmLayerMultiscaleGWRItem::indepVarsIndex() const
{
    return mIndepVarsIndex;
}

QList<GwmLayerAttributeItem *> GwmLayerMultiscaleGWRItem::indepVarsOrigin() const
{
    return mIndepVarsOrigin;
}

vec GwmLayerMultiscaleGWRItem::bandwidthSize() const
{
    return mBandwidthSize;
}

QList<QString> GwmLayerMultiscaleGWRItem::bandwidthUnit() const
{
    return mBandwidthUnit;
}

QList<GwmMultiscaleGWRTaskThread::BandwidthSeledType> GwmLayerMultiscaleGWRItem::bandwidthSeled() const
{
    return mBandwidthSeled;
}

QList<GwmGWRTaskThread::BandwidthType> GwmLayerMultiscaleGWRItem::bandwidthType() const
{
    return mBandwidthType;
}

QList<GwmGWRTaskThread::BandwidthSelectionApproach> GwmLayerMultiscaleGWRItem::bandwidthSelectionApproach() const
{
    return mBandwidthSelectionApproach;
}

QList<GwmGWRTaskThread::KernelFunction> GwmLayerMultiscaleGWRItem::bandwidthKernelFunction() const
{
    return mBandwidthKernelFunction;
}

QList<GwmGWRTaskThread::DistanceSourceType> GwmLayerMultiscaleGWRItem::distanceSource() const
{
    return mDistanceSource;
}

QList<QVariant> GwmLayerMultiscaleGWRItem::distSrcParameters() const
{
    return mDistSrcParameters;
}

QList<bool> GwmLayerMultiscaleGWRItem::preditorCentered() const
{
    return mPreditorCentered;
}

vec GwmLayerMultiscaleGWRItem::bandwidthSelectThreshold() const
{
    return mBandwidthSelectThreshold;
}

GwmGWRDiagnostic GwmLayerMultiscaleGWRItem::diagnostic() const
{
    return mDiagnostic;
}

arma::mat GwmLayerMultiscaleGWRItem::betas() const
{
    return mBetas;
}

bool GwmLayerMultiscaleGWRItem::getHasHatmatrix() const
{
    return hasHatmatrix;
}

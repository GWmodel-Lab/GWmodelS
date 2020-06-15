#include "gwmlayermultiscalegwritem.h"
#include "gwmlayergroupitem.h"

GwmLayerMultiscaleGWRItem::GwmLayerMultiscaleGWRItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmMultiscaleGWRAlgorithm* taskThread)
    : GwmLayerVectorItem(parent, vector)
{
    if (taskThread)
    {
        mDataPointsSize = taskThread->dataLayer()->featureCount();
        mDepVar = taskThread->dependentVariable();
        mIndepVars = taskThread->independentVariables();
        for (const GwmSpatialWeight& sp : taskThread->spatialWeights())
        {
            GwmBandwidthWeight* pBw = static_cast<GwmBandwidthWeight*>(sp.weight()->clone());
            GwmBandwidthWeight bw(pBw);
            mBandwidthWeights.append(bw);
            mDistaneTypes.append(sp.distance()->type());
        }
        mBandwidthInitilize = taskThread->bandwidthInitilize();
        mBandwidthSelectionApproach = taskThread->bandwidthSelectionApproach();
        mPreditorCentered = taskThread->preditorCentered();
        mBandwidthSelectThreshold = taskThread->bandwidthSelectThreshold();
        mCriterionType = taskThread->criterionType();
        mDiagnostic = taskThread->diagnostic();
        mBetas = mat(taskThread->betas());
        mHasHatmatrix = taskThread->hasHatMatrix();
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

QList<bool> GwmLayerMultiscaleGWRItem::preditorCentered() const
{
    return mPreditorCentered;
}

GwmDiagnostic GwmLayerMultiscaleGWRItem::diagnostic() const
{
    return mDiagnostic;
}

arma::mat GwmLayerMultiscaleGWRItem::betas() const
{
    return mBetas;
}

bool GwmLayerMultiscaleGWRItem::hasHatmatrix() const
{
    return mHasHatmatrix;
}

QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> GwmLayerMultiscaleGWRItem::bandwidthInitilize() const
{
    return mBandwidthInitilize;
}

QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> GwmLayerMultiscaleGWRItem::bandwidthSelectionApproach() const
{
    return mBandwidthSelectionApproach;
}

QList<double> GwmLayerMultiscaleGWRItem::bandwidthSelectThreshold() const
{
    return mBandwidthSelectThreshold;
}

GwmMultiscaleGWRAlgorithm::BackFittingCriterionType GwmLayerMultiscaleGWRItem::criterionType() const
{
    return mCriterionType;
}

QList<GwmBandwidthWeight> GwmLayerMultiscaleGWRItem::bandwidthWeights() const
{
    return mBandwidthWeights;
}

QList<GwmDistance::DistanceType> GwmLayerMultiscaleGWRItem::distaneTypes() const
{
    return mDistaneTypes;
}

GwmVariable GwmLayerMultiscaleGWRItem::depVar() const
{
    return mDepVar;
}

QList<GwmVariable> GwmLayerMultiscaleGWRItem::indepVars() const
{
    return mIndepVars;
}

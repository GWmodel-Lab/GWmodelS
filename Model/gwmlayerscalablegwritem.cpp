#include "gwmlayerscalablegwritem.h"

GwmLayerScalableGWRItem::GwmLayerScalableGWRItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmScalableGWRTaskThread* taskThread)
    : GwmLayerVectorItem(parent, vector)
{
    mDataPointsSize = taskThread->dataLayer()->featureCount();
    mDepVar = taskThread->dependentVariable();
    mIndepVars = taskThread->independentVariables();
    mWeight = GwmBandwidthWeight(*static_cast<GwmBandwidthWeight*>(taskThread->spatialWeight().weight()));
    mDistanceType = taskThread->spatialWeight().distance()->type();
    mDiagnostic = taskThread->diagnostic();
    mBetas = mat(taskThread->betas());
    mPolynomial = taskThread->getPolynomial();
    mCV = taskThread->getCV();
    mScale = taskThread->getScale();
    mPenalty = taskThread->getPenalty();
}

double GwmLayerScalableGWRItem::cv() const
{
    return mCV;
}

double GwmLayerScalableGWRItem::scale() const
{
    return mScale;
}

double GwmLayerScalableGWRItem::penalty() const
{
    return mPenalty;
}

int GwmLayerScalableGWRItem::polynomial() const
{
    return mPolynomial;
}

int GwmLayerScalableGWRItem::dataPointsSize() const
{
    return mDataPointsSize;
}

GwmVariable GwmLayerScalableGWRItem::depVar() const
{
    return mDepVar;
}

QList<GwmVariable> GwmLayerScalableGWRItem::indepVars() const
{
    return mIndepVars;
}

GwmBandwidthWeight GwmLayerScalableGWRItem::weight() const
{
    return mWeight;
}

GwmDiagnostic GwmLayerScalableGWRItem::diagnostic() const
{
    return mDiagnostic;
}

arma::mat GwmLayerScalableGWRItem::betas() const
{
    return mBetas;
}

GwmDistance::DistanceType GwmLayerScalableGWRItem::distanceType() const
{
    return mDistanceType;
}

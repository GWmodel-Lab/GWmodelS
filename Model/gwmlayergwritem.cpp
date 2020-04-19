#include "gwmlayergwritem.h"
#include "gwmlayergroupitem.h"

GwmLayerGWRItem::GwmLayerGWRItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmGWRTaskThread* taskThread)
    : GwmLayerVectorItem(parent, vector)
{
    if (taskThread)
    {
        mDataPointsSize = taskThread->getFeatureList().size();
        mDepVar = taskThread->depVar();
        mIndepVars = taskThread->indepVars();
        mBandwidthType = taskThread->bandwidthType();
        mBandwidthSize = (mBandwidthType == GwmGWRTaskThread::BandwidthType::Fixed ?
                              taskThread->getBandwidthSizeOrigin() :
                              taskThread->getBandwidthSize());
        mBandwidthUnit = taskThread->getBandwidthUnit();
        mBandwidthKernelFunction = taskThread->getBandwidthKernelFunction();
        mDiagnostic = taskThread->getDiagnostic();
    }
}

int GwmLayerGWRItem::childNumber()
{
    if (mParentItem)
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this);
    return 0;
}

int GwmLayerGWRItem::dataPointsSize() const
{
    return mDataPointsSize;
}

GwmLayerAttributeItem *GwmLayerGWRItem::depVar() const
{
    return mDepVar;
}

QList<GwmLayerAttributeItem *> GwmLayerGWRItem::indepVars() const
{
    return mIndepVars;
}

double GwmLayerGWRItem::bandwidthSize() const
{
    return mBandwidthSize;
}

QString GwmLayerGWRItem::bandwidthUnit() const
{
    return mBandwidthUnit;
}

GwmGWRTaskThread::BandwidthType GwmLayerGWRItem::bandwidthType() const
{
    return mBandwidthType;
}

GwmGWRDiagnostic GwmLayerGWRItem::diagnostic() const
{
    return mDiagnostic;
}

GwmGWRTaskThread::KernelFunction GwmLayerGWRItem::bandwidthKernelFunction() const
{
    return mBandwidthKernelFunction;
}

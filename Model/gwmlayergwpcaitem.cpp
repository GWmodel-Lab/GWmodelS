#include "gwmlayergwpcaitem.h"
#include "gwmlayergroupitem.h"

GwmLayerGWPCAItem::GwmLayerGWPCAItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmGWPCATaskThread *taskThread)
    : GwmLayerVectorItem(parent, vector)
{
    if (taskThread)
    {
        mdResult1 = taskThread->variance();
        mLocalPV = taskThread->localPV();
        mk = taskThread->k();
        mWeight = taskThread->spatialWeight().weight<GwmBandwidthWeight>();
        mBandwidthSelScores = taskThread->bandwidthSelectorCriterions();

        isBandwidthOptimized = taskThread->isAutoselectBandwidth();
        //
        mLoadings = taskThread->loadings();
        mScores = taskThread->scores();
        mVariance = taskThread->variance();
    }
}

GwmBandwidthWeight GwmLayerGWPCAItem::weight() const
{
    return mWeight;
}

QList<QPair<double, double> > GwmLayerGWPCAItem::bandwidthSelScores() const
{
    return mBandwidthSelScores;
}

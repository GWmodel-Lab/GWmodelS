#include "gwmlayergwpcaitem.h"
#include "gwmlayergroupitem.h"

GwmLayerGWPCAItem::GwmLayerGWPCAItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmGWPCATaskThread *taskThread)
    : GwmLayerVectorItem(parent, vector)
{
    if (taskThread)
    {
        mdResult1 = taskThread->dResult1();
        mLocalPV = taskThread->localPV();
        mk = taskThread->k();
        mWeight = taskThread->spatialWeight().weight<GwmBandwidthWeight>();
        mBandwidthSelScores = taskThread->bandwidthSelectorCriterions();

        isBandwidthOptimized = taskThread->isAutoselectBandwidth();
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

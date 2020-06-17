#include "gwmlayergwssitem.h"
#include "gwmlayergroupitem.h"


GwmLayerGWSSItem::GwmLayerGWSSItem(GwmLayerItem* parentItem, QgsVectorLayer* vector, const GwmGWSSTaskThread* taskThread)
    : GwmLayerVectorItem(parentItem, vector)
{
    mBWS = taskThread->getBWS();
    mVariables = taskThread->variables();
    mBandwidth = taskThread->bandwidth();
}

int GwmLayerGWSSItem::childNumber()
{
    if (mParentItem)
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this);
    return 0;
}


#include "gwmlayergwritem.h"
#include "gwmlayergroupitem.h"

GwmLayerGWRItem::GwmLayerGWRItem(GwmLayerItem* parent, QgsVectorLayer* vector)
    : GwmLayerVectorItem(parent, vector)
{

}

int GwmLayerGWRItem::childNumber()
{
    if (mParentItem)
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this);
    return 0;
}

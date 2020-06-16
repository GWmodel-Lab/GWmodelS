#ifndef GWMLAYERGWSSITEM_H
#define GWMLAYERGWSSITEM_H

#include "gwmlayervectoritem.h"
#include "TaskThread/gwmgwsstaskthread.h"

class GwmLayerGWSSItem : public GwmLayerVectorItem
{
public:
    GwmLayerGWSSItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmGWSSTaskThread* taskThread = nullptr);

    virtual int childNumber() override;

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::GWSS; }

    mat bws() const
    {
        return mBWS;
    }

    QList<GwmVariable> variables() const
    {
        return mVariables;
    }

    GwmBandwidthWeight* bandwidth() const
    {
        return mBandwidth;
    }

protected:

    mat mBWS;
    QList<GwmVariable> mVariables;
    GwmBandwidthWeight* mBandwidth;
};

#endif // GWMLAYERGWSSITEM_H

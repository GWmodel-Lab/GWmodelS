#ifndef GWMLAYERORIGINITEM_H
#define GWMLAYERORIGINITEM_H

#include <QObject>
#include "gwmlayervectoritem.h"

class GwmLayerOriginItem : public GwmLayerVectorItem
{
    Q_OBJECT
public:
    GwmLayerOriginItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr);

    inline virtual QString text() override;

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::Origin; }

    virtual int childNumber() override { return 0; }
};

inline QString GwmLayerOriginItem::text()
{
    return "Origin";
}

#endif // GWMLAYERORIGINITEM_H

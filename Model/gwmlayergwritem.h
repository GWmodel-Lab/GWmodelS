#ifndef GWMLAYERGWRITEM_H
#define GWMLAYERGWRITEM_H

#include "prefix.h"

#include <QObject>
#include <qgsvectorlayer.h>
#include "gwmlayervectoritem.h"

class GwmLayerGWRItem : public GwmLayerVectorItem
{
    Q_OBJECT
public:
    explicit GwmLayerGWRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr);

    virtual int childNumber() override;
};

#endif // GWMLAYERGWRITEM_H

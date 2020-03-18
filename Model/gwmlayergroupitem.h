#ifndef GWMLAYERGROUPITEM_H
#define GWMLAYERGROUPITEM_H

#include "prefix.h"
#include <QObject>
#include <QVariant>
#include <qgsvectorlayer.h>
#include "gwmlayeritem.h"
#include "gwmlayervectoritem.h"

class GwmLayerGroupItem : public GwmLayerItem
{
    Q_OBJECT
public:
    explicit GwmLayerGroupItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr);
    ~GwmLayerGroupItem();

public:
    virtual QString text() override;
    virtual QVariant data(int col, int role) override;
    virtual GwmLayerItem * child(int row) override;
    virtual int childCount() override;
    virtual int childNumber() override;

    virtual bool insertChildren(int position, int count) override;
    virtual bool removeChildren(int position, int count) override;

    inline void setOriginItem(GwmLayerVectorItem* item) { mOriginChild = item; }

    inline GwmLayerVectorItem* originChild() const;
    inline void setOriginChild(GwmLayerVectorItem *originChild);

    inline QList<GwmLayerVectorItem*>& analyseChildren() const;
    inline void setAnalyseChildren(const QList<GwmLayerVectorItem *> &analyseChildren);

private:
    GwmLayerVectorItem* mOriginChild;
    QList<GwmLayerVectorItem*> mAnalyseChildren;
};

#endif // GWMLAYERGROUPITEM_H

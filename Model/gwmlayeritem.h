#ifndef GWMLAYERITEM_H
#define GWMLAYERITEM_H

#include "prefix.h"
#include <QObject>
#include <QList>
#include <QVariant>
#include <qgsvectorlayer.h>

class GwmLayerGroupItem;

class GwmLayerItem : public QObject
{
    Q_OBJECT
public:
    explicit GwmLayerItem(GwmLayerItem* parentItem = nullptr);
    ~GwmLayerItem();

public:
    virtual QString text();
    virtual QVariant data(int col, int role);

    virtual GwmLayerItem* parentItem() const;
    inline void setParentItem(GwmLayerItem *parentItem);

    virtual GwmLayerItem* child(int row);
    virtual int childCount();
    virtual int childNumber();
    virtual bool insertChildren(int position, int count);
    virtual bool removeChildren(int position, int count);

    inline QList<GwmLayerGroupItem*>& children();
    inline void setChildren(const QList<GwmLayerGroupItem *> &children);
    inline void appendChildren(GwmLayerGroupItem* item) { mChildren.append(item); }

protected:
    GwmLayerItem* mParentItem;

private:
    QList<GwmLayerGroupItem*> mChildren;

};

#endif // GWMLAYERITEM_H

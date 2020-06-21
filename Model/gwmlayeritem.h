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
    enum GwmLayerItemType
    {
        Base,
        Group,
        Vector,
        Origin,
        GWR,
        Symbol,
        ScalableGWR,
        GGWR,
        MultiscaleGWR,
        GWPCA
    };

public:
    explicit GwmLayerItem(GwmLayerItem* parentItem = nullptr);
    ~GwmLayerItem();

public:
    virtual QString text();
    virtual QVariant data(int col, int role);
    virtual Qt::ItemFlags flags();
    virtual bool setData(int col, int role, QVariant value);

    virtual GwmLayerItem* parentItem() const;
    inline void setParentItem(GwmLayerItem *parentItem);

    virtual GwmLayerItem* child(int row);
    virtual int childCount();
    virtual int childNumber();
    virtual bool insertChildren(int position, int count);
    virtual bool removeChildren(int position, int count);

    virtual bool insertChildren(int position, QList<GwmLayerItem*> items);
    virtual bool appendChildren(QList<GwmLayerItem*> items);
    virtual QList<GwmLayerItem*> takeChildren(int position, int count);
    virtual bool moveChildren(int position, int count, int destination);


    inline QList<GwmLayerGroupItem *> children() const { return mChildren; }
    inline void setChildren(const QList<GwmLayerGroupItem *> &children) { mChildren = children; }

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::Base; }

    Qt::CheckState checkState() const;
    void setCheckState(const Qt::CheckState &checkState);

protected:
    GwmLayerItem* mParentItem;
    Qt::CheckState mCheckState = Qt::CheckState::Checked;

private:
    QList<GwmLayerGroupItem*> mChildren;

};

#endif // GWMLAYERITEM_H

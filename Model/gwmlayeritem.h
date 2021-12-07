#ifndef GWMLAYERITEM_H
#define GWMLAYERITEM_H

#include "prefix.h"
#include <QObject>
#include <QList>
#include <QVariant>
#include <qgsvectorlayer.h>
#include <QDomDocument>

#include "gwmenumvaluenamemapper.h"

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
        GeneralizedGWR,
        MultiscaleGWR,
        GWSS,
        CollinearityGWR,
        GWPCA,
        GTWR
    };

    static GwmEnumValueNameMapper<GwmLayerItemType> LayerItemTypeNameMapper;

public:
    explicit GwmLayerItem(GwmLayerItem* parentItem = nullptr);
    ~GwmLayerItem();

public:
    virtual QString text();
    virtual QVariant data(int col, int role);
    virtual Qt::ItemFlags flags();
    virtual bool setData(int col, int role, QVariant value);

    virtual GwmLayerItem* parentItem() const;
    virtual void setParentItem(GwmLayerItem *parentItem);

    virtual GwmLayerItem* child(int row);
    virtual int childCount();
    virtual int childNumber();
    virtual bool insertChildren(int position, int count);
    virtual bool removeChildren(int position, int count);

    virtual bool insertChildren(int position, QList<GwmLayerItem*> items);
    virtual bool appendChildren(QList<GwmLayerItem*> items);
    virtual QList<GwmLayerItem*> takeChildren(int position, int count);
    virtual bool moveChildren(int position, int count, int destination);

    virtual bool readXml(QDomNode& node);
    virtual bool writeXml(QDomNode& node, QDomDocument& doc);

    inline QList<GwmLayerGroupItem *> children() const { return mChildren; }
    inline void setChildren(const QList<GwmLayerGroupItem *> &children) { mChildren = children; }

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::Base; }

    Qt::CheckState checkState() const;
    void setCheckState(const Qt::CheckState &checkState);
    QWidget* tabWidget = nullptr;

protected:
    GwmLayerItem* mParentItem;
    Qt::CheckState mCheckState = Qt::CheckState::Checked;

private:
    QList<GwmLayerGroupItem*> mChildren;
};

#endif // GWMLAYERITEM_H

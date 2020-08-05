#ifndef GWMLAYERGROUPITEM_H
#define GWMLAYERGROUPITEM_H

#include "prefix.h"
#include <QObject>
#include <QVariant>
#include <qgsvectorlayer.h>
#include "gwmlayeritem.h"
#include "gwmlayervectoritem.h"
#include "gwmlayeroriginitem.h"

class GwmLayerGroupItem : public GwmLayerItem
{
    Q_OBJECT
public:
    explicit GwmLayerGroupItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr);
    ~GwmLayerGroupItem();

public:
    virtual QString text() override;
    virtual QVariant data(int col, int role) override;
    virtual bool setData(int col, int role, QVariant value) override;
    virtual Qt::ItemFlags flags() override;

    virtual GwmLayerItem * child(int row) override;
    virtual int childCount() override;
    virtual int childNumber() override;

    virtual bool insertChildren(int position, int count) override;
    virtual bool removeChildren(int position, int count) override;

    virtual bool insertChildren(int position, QList<GwmLayerItem*> items) override;
    virtual bool appendChildren(QList<GwmLayerItem*> items) override;
    virtual QList<GwmLayerItem*> takeChildren(int position, int count) override;
    virtual bool moveChildren(int position, int count, int destination) override;

    virtual bool readXml(QDomNode &node) override;
    virtual bool writeXml(QDomNode& node, QDomDocument& doc) override;

    inline void setOriginItem(GwmLayerOriginItem* item) { mOriginChild = item; }

    inline GwmLayerOriginItem *originChild() const;
    inline void setOriginChild(GwmLayerOriginItem *originChild);

    inline QList<GwmLayerVectorItem *> analyseChildren() const { return mAnalyseChildren; }
    inline void setAnalyseChildren(const QList<GwmLayerVectorItem *> &analyseChildren);

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::Group; }

private:
    GwmLayerOriginItem* mOriginChild;
    QList<GwmLayerVectorItem*> mAnalyseChildren;
};


inline void GwmLayerGroupItem::setOriginChild(GwmLayerOriginItem *originChild)
{
    mOriginChild = originChild;
}

inline void GwmLayerGroupItem::setAnalyseChildren(const QList<GwmLayerVectorItem *> &analyseChildren)
{
    mAnalyseChildren = analyseChildren;
}

inline GwmLayerOriginItem *GwmLayerGroupItem::originChild() const
{
    return mOriginChild;
}

#endif // GWMLAYERGROUPITEM_H

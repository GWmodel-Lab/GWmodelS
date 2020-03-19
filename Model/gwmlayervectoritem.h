#ifndef GWMLAYERVECTOR_H
#define GWMLAYERVECTOR_H

#include "prefix.h"

#include <QObject>
#include <QList>
#include <QVariant>
#include <qgsvectorlayer.h>
#include "gwmlayeritem.h"
#include "gwmlayersymbolitem.h"

class GwmLayerVectorItem : public GwmLayerItem
{
    Q_OBJECT
public:
    enum SymbolType
    {
        singleSymbol,
        categorizedSymbol,
        graduatedSymbol,
        RuleRenderer,
        heatmapRenderer,
        invertedPolygonRenderer,
        pointCluster,
        pointDisplacement,
        nullSymbol
    };

    static SymbolType renderTypeToSymbolType(QString itemType);

public:
    explicit GwmLayerVectorItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr);
    ~GwmLayerVectorItem();

    virtual QString text() override;
    virtual QVariant data(int col, int role) override;
    virtual Qt::ItemFlags flags() override;
    virtual GwmLayerItem * child(int row) override;
    virtual int childCount() override;
    virtual int childNumber() override;

    virtual bool insertChildren(int position, int count) override;
    virtual bool removeChildren(int position, int count) override;

    inline QgsVectorLayer *layer() const;
    inline void setLayer(QgsVectorLayer* layer);


    inline QList<GwmLayerSymbolItem *> symbolChildren() const;
    inline void setSymbolChildren(const QList<GwmLayerSymbolItem *> &symbolChildren);

    inline SymbolType symbolType() const;
    inline void setSymbolType(const SymbolType &symbolType);

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::Vector; }

protected:
    QgsVectorLayer* mLayer;
    SymbolType mSymbolType;
    QList<GwmLayerSymbolItem*> mSymbolChildren;

private:
    void createSymbolChildren();
};


inline GwmLayerVectorItem::SymbolType GwmLayerVectorItem::symbolType() const
{
    return mSymbolType;
}

inline void GwmLayerVectorItem::setSymbolType(const SymbolType &symbolType)
{
    mSymbolType = symbolType;
}

inline QList<GwmLayerSymbolItem *> GwmLayerVectorItem::symbolChildren() const
{
    return mSymbolChildren;
}

inline QgsVectorLayer *GwmLayerVectorItem::layer() const
{
    return mLayer;
}

inline void GwmLayerVectorItem::setLayer(QgsVectorLayer *layer)
{
    mLayer = layer;
}


inline void GwmLayerVectorItem::setSymbolChildren(const QList<GwmLayerSymbolItem *> &symbolChildren)
{
    mSymbolChildren = symbolChildren;
}

#endif // GWMLAYERVECTOR_H

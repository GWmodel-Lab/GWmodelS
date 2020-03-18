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

    static SymbolType renderTypeToSymbolType(QString type);

public:
    explicit GwmLayerVectorItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr);
    ~GwmLayerVectorItem();

    virtual QString text() override;
    virtual QVariant data(int col, int role) override;
    virtual GwmLayerItem * child(int row) override;
    virtual int childCount() override;
    virtual int childNumber() override;

    virtual bool insertChildren(int position, int count) override;
    virtual bool removeChildren(int position, int count) override;

    inline QgsVectorLayer *layer() const;
    inline void setLayer(QgsVectorLayer* layer);

    inline virtual QList<GwmLayerSymbolItem*>& symbolChildren();
    inline void setSymbolChildren(const QList<GwmLayerSymbolItem *> &symbolChildren);

    inline SymbolType symbolType() const;
    inline void setSymbolType(const SymbolType &symbolType);

protected:
    QgsVectorLayer* mLayer;
    SymbolType mSymbolType;
    QList<GwmLayerSymbolItem*> mSymbolChildren;

private:
    void getSymbolChildren();
};

#endif // GWMLAYERVECTOR_H

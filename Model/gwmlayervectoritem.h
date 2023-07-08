#ifndef GWMLAYERVECTOR_H
#define GWMLAYERVECTOR_H

#include "prefix.h"

#include <QObject>
#include <QList>
#include <QVariant>
#include <qgsvectorlayer.h>
#include <qgsvectorfilewriter.h>

#include "gwmlayeritem.h"
#include "gwmenumvaluenamemapper.h"

class GwmLayerSymbolItem;

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

    static GwmEnumValueNameMapper<SymbolType> SymbolTypeNameMapper;

    static GwmEnumValueNameMapper<QVariant::Type> FieldTypeNameMapper;

public:
    explicit GwmLayerVectorItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr);
    ~GwmLayerVectorItem();

signals:
    void itemSymbolChangedSignal();

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

    inline QgsVectorLayer *layer() const;
    inline void setLayer(QgsVectorLayer* layer);


    inline QList<GwmLayerSymbolItem *> symbolChildren() const;
    inline void setSymbolChildren(const QList<GwmLayerSymbolItem *> &symbolChildren);

    inline SymbolType symbolType() const;
    inline void setSymbolType(const SymbolType &symbolType);

    inline virtual GwmLayerItemType itemType() override { return GwmLayerItemType::Vector; }

    QString provider() const;
    void setProvider(const QString &provider);

    QString path() const;
    void setPath(const QString &path);

    bool save(QString filePath, QString fileName, QString fileType,QgsVectorFileWriter::SaveVectorOptions& options = *(new QgsVectorFileWriter::SaveVectorOptions()));

    void setDataSource(const QString &path, const QString& provider);

protected:
    QgsVectorLayer* mLayer;
    SymbolType mSymbolType;
    QList<GwmLayerSymbolItem*> mSymbolChildren;

    QString mProvider;
    QString mPath;

private:
    void createSymbolChildren();

private slots:
    void onLayerRendererChanged();
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

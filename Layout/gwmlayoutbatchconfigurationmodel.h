#ifndef GWMLAYOUTBATCHCONFIGURATIONMODEL_H
#define GWMLAYOUTBATCHCONFIGURATIONMODEL_H

#include <QAbstractItemModel>

#include <qgsvectorlayer.h>
#include <qgsrenderer.h>

#include "Layout/gwmlayoutbatchlayerlistmodel.h"
#include "Layout/gwmlayoutbatchfieldlistmodel.h"

typedef QgsFeatureRenderer* QgsFeatureRendererPointer;
Q_DECLARE_METATYPE(QgsFeatureRendererPointer)

class GwmLayoutBatchConfigurationItem
{
public:
    enum Type
    {
        Root,
        Layer,
        Field
    };

public:
    GwmLayoutBatchConfigurationItem(GwmLayoutBatchConfigurationItem* parent);
    virtual ~GwmLayoutBatchConfigurationItem();

    QString name() const;
    void setName(const QString &value);

    GwmLayoutBatchConfigurationItem *parent() const;
    void setParent(GwmLayoutBatchConfigurationItem *value);

    virtual Type type() = 0;
    virtual Qt::ItemFlags flags(int column) = 0;

    virtual int size() = 0;
    virtual GwmLayoutBatchConfigurationItem* child(int i) = 0;
    virtual int childNumber(GwmLayoutBatchConfigurationItem* child) = 0;

    virtual QVariant data(int column, int role) const = 0;
    virtual void setData(int column, int role, const QVariant& value) = 0;

    virtual bool insertChild(int i, GwmLayoutBatchConfigurationItem* item) = 0;
    virtual bool removeChild(int i) = 0;

protected:
    QString mName = QStringLiteral("");
    GwmLayoutBatchConfigurationItem* mParent = nullptr;
};

class GwmLayoutBatchConfigurationItemField : public GwmLayoutBatchConfigurationItem
{
public:
    typedef void (*Setter)(GwmLayoutBatchConfigurationItemField* self, const QVariant&);

public:
    GwmLayoutBatchConfigurationItemField(GwmLayoutBatchConfigurationItem *parent);
    GwmLayoutBatchConfigurationItemField(const QgsFieldCheckable& field, const QgsVectorLayer* layer, GwmLayoutBatchConfigurationItem *parent);
    ~GwmLayoutBatchConfigurationItemField();

    QgsFeatureRenderer *renderer() const;
    void setRenderer(QgsFeatureRenderer *renderer);

    Type type() override { return Type::Field; }
    Qt::ItemFlags flags(int column) override;

    int size() override { return 0; }
    GwmLayoutBatchConfigurationItem *child(int i) override;
    int childNumber(GwmLayoutBatchConfigurationItem* child) override { return -1; }
    QVariant data(int column, int role) const override;
    void setData(int column, int role, const QVariant& value) override;

    bool insertChild(int i, GwmLayoutBatchConfigurationItem* item) override;
    bool removeChild(int i) override;

private:
//    int mID = 0;
    QgsFeatureRenderer* mRenderer = nullptr;
};

class GwmLayoutBatchConfigurationItemLayer : public GwmLayoutBatchConfigurationItem
{
public:
    GwmLayoutBatchConfigurationItemLayer(GwmLayoutBatchConfigurationItem *parent);
    GwmLayoutBatchConfigurationItemLayer(QgsVectorLayer *layer, GwmLayoutBatchConfigurationItem *parent);
    ~GwmLayoutBatchConfigurationItemLayer();

    QgsVectorLayer *layer() const;
    void setLayer(QgsVectorLayer *layer);

    Type type() override { return Type::Layer; }
    Qt::ItemFlags flags(int column) override;

    int size() override { return mFieldList.size(); }
    GwmLayoutBatchConfigurationItem *child(int i) override;
    int childNumber(GwmLayoutBatchConfigurationItem* child) override;
    QVariant data(int column, int role) const override;
    void setData(int column, int role, const QVariant& value) override;

    bool insertChild(int i, GwmLayoutBatchConfigurationItem* item) override;
    bool removeChild(int i) override;

    QList<GwmLayoutBatchConfigurationItemField*> fieldList()
    {
        return mFieldList;
    }

private:
    QgsVectorLayer* mLayer = nullptr;
    QList<GwmLayoutBatchConfigurationItemField*> mFieldList;
};

class GwmLayoutBatchConfigurationItemRoot : public GwmLayoutBatchConfigurationItem
{
public:
    GwmLayoutBatchConfigurationItemRoot();
    ~GwmLayoutBatchConfigurationItemRoot();

    Type type() override { return Type::Root; }
    Qt::ItemFlags flags(int column) override;

    int size() override { return mLayerList.size(); }
    GwmLayoutBatchConfigurationItem *child(int i) override;
    int childNumber(GwmLayoutBatchConfigurationItem* child) override;
    QVariant data(int column, int role) const override;
    void setData(int column, int role, const QVariant& value) override;

    bool insertChild(int i, GwmLayoutBatchConfigurationItem* item) override;
    bool removeChild(int i) override;

    QList<GwmLayoutBatchConfigurationItemLayer*> layerList()
    {
        return mLayerList;
    }

private:
    QList<GwmLayoutBatchConfigurationItemLayer*> mLayerList;
};

class GwmLayoutBatchConfigurationModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit GwmLayoutBatchConfigurationModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

//    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool insertLayer(int row, QgsVectorLayer *layer);
    bool removeLayer(int row);
    bool removeLayer(const QgsVectorLayer* layer);

    bool insertField(int row, const QgsFieldCheckable& fieldItem, const QModelIndex& parent);
    bool removeField(int row, const QModelIndex& parent);
    bool removeField(const QgsFieldCheckable& fieldItem, const QModelIndex& parent);
    bool setFieldRenderer(const QModelIndex& index, QgsFeatureRenderer* renderer);

public:
    GwmLayoutBatchConfigurationItem *itemFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromLayer(const QgsVectorLayer* layer) const;
    QModelIndex indexFromField(const QgsFieldCheckable& field, const QModelIndex& parent) const;

private:
    // Add data:
//    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
//    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
//    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
//    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

private:
    GwmLayoutBatchConfigurationItemRoot* mItemRoot;
};


inline QString GwmLayoutBatchConfigurationItem::name() const
{
    return mName;
}

inline void GwmLayoutBatchConfigurationItem::setName(const QString &value)
{
    mName = value;
}

inline GwmLayoutBatchConfigurationItem *GwmLayoutBatchConfigurationItem::parent() const
{
    return mParent;
}

inline void GwmLayoutBatchConfigurationItem::setParent(GwmLayoutBatchConfigurationItem *value)
{
    mParent = value;
}

inline QgsFeatureRenderer *GwmLayoutBatchConfigurationItemField::renderer() const
{
    return mRenderer;
}

inline QgsVectorLayer *GwmLayoutBatchConfigurationItemLayer::layer() const
{
    return mLayer;
}

inline void GwmLayoutBatchConfigurationItemLayer::setLayer(QgsVectorLayer *layer)
{
    mLayer = layer;
}


#endif // GWMLAYOUTBATCHCONFIGURATIONMODEL_H

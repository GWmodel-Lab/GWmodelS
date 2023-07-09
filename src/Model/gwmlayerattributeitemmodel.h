#ifndef GWMLAYERATTRIBUTEITEMMODEL_H
#define GWMLAYERATTRIBUTEITEMMODEL_H

#include <QAbstractItemModel>
#include "gwmlayerattributeitem.h"

class GwmLayerAttributeItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit GwmLayerAttributeItemModel(QObject *parent = nullptr);

    ~GwmLayerAttributeItemModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

//    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QList<GwmLayerAttributeItem*> findItems(QString attributeName);

    bool clear();
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    void appendRow(GwmLayerAttributeItem* item);
    void appendItem(int index, const QString attributeName,const QVariant::Type type);

    GwmLayerAttributeItem* item(int i) const;
    GwmLayerAttributeItem* itemFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromItem(GwmLayerAttributeItem* item) const;

    QList<QString> toLayerAttributeList();

    QList<GwmLayerAttributeItem *> attributeItemList() const;

private:
    QList<GwmLayerAttributeItem*> mAttributeItemList;
};

#endif // GWMLAYERATTRIBUTEITEMMODEL_H

#ifndef GWMLAYERITEMMODEL_H
#define GWMLAYERITEMMODEL_H

#include <QAbstractItemModel>
#include "gwmlayergroupitem.h"

class GwmLayerItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit GwmLayerItemModel(QObject *parent = nullptr);

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

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
//    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
//    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    void addLayer(QgsVectorLayer* layer);

    void setupModel();

private:
    GwmLayerItem* mRootItem;
    GwmLayerItem* itemFromIndex(const QModelIndex& index) const;
};

#endif // GWMLAYERITEMMODEL_H

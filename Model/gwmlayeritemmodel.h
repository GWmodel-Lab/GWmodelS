#ifndef GWMLAYERITEMMODEL_H
#define GWMLAYERITEMMODEL_H

#include <QAbstractItemModel>
#include "gwmlayergroupitem.h"

class GwmLayerItemModel : public QAbstractItemModel
{
    Q_OBJECT

signals:
    void layerAddedSignal();
    void layerRemovedSignal();
    void layerItemChangedSignal(GwmLayerItem* item);
    void layerItemMovedSignal();

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


    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    QList<GwmLayerItem*> takeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    bool insertItem(int row, GwmLayerItem *item, const QModelIndex& parent = QModelIndex());
    GwmLayerItem* takeItem(int row, const QModelIndex& parent = QModelIndex());
    bool appentItem(GwmLayerItem* item, const QModelIndex& parent = QModelIndex());
    void appendItem(QgsVectorLayer* layer, const QString path, const QString provider);

    GwmLayerGroupItem* item(int i);
    GwmLayerItem* itemFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromItem(GwmLayerItem* item) const;
    QgsVectorLayer* layerFromItem(GwmLayerItem* item) const;

    QList<GwmLayerGroupItem*> rootChildren();

    QList<QgsMapLayer*> toMapLayerList();

public: // 要素区工具栏相关函数
    bool canMoveUp(const QModelIndex& index);
    bool canMoveDown(const QModelIndex& index);
    bool canRemove(const QModelIndex& index);
    bool canSetSymbol(const QModelIndex& index);

    void moveUp(const QModelIndex& index);
    void moveDown(const QModelIndex& index);
    void remove(const QModelIndex& index);

private:
    GwmLayerItem* mRootItem;

private slots:
    void onVectorItemSymbolChanged();
};

#endif // GWMLAYERITEMMODEL_H

#include "gwmlayoutbatchlayerlistmodel.h"

GwmLayoutBatchLayerListModel::GwmLayoutBatchLayerListModel(GwmLayerItemModel *layerItemModel, QObject *parent)
    : QAbstractListModel(parent)
{
    for (auto layer : layerItemModel->toMapLayerList())
    {
        mMapLayerList.append(layer);
    }
}

int GwmLayoutBatchLayerListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return mMapLayerList.size();
}

QVariant GwmLayoutBatchLayerListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    if (row > rowCount())
        return QVariant();

    switch (role) {
    case Qt::ItemDataRole::DisplayRole:
        return mMapLayerList[row].layer->name();
    case Qt::ItemDataRole::CheckStateRole:
        return mMapLayerList[row].selected ? Qt::Checked : Qt::Unchecked;
    default:
        return QVariant();
    }
}

bool GwmLayoutBatchLayerListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        int row = index.row();
        switch (role) {
        case Qt::ItemDataRole::CheckStateRole:
            mMapLayerList[row].selected = value.toInt() == Qt::Checked;
            break;
        default:
            break;
        }
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }

    return false;
}

Qt::ItemFlags GwmLayoutBatchLayerListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemFlag::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

QList<QgsMapLayer *> GwmLayoutBatchLayerListModel::checkedLayers()
{
    QList<QgsMapLayer*> layers;
    for (auto item : mMapLayerList)
    {
        if (item.selected)
            layers.append(item.layer);
    }
    return layers;
}

QgsMapLayer *GwmLayoutBatchLayerListModel::layerFromIndex(const QModelIndex &index)
{
    if (!index.isValid())
        return nullptr;

    int row = index.row();
    if (row > rowCount())
        return nullptr;

    return mMapLayerList[row].layer;
}

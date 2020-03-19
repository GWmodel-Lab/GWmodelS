#include "gwmlayeritemmodel.h"
#include "gwmlayergwritem.h"

GwmLayerItemModel::GwmLayerItemModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    mRootItem = new GwmLayerItem();
}

QVariant GwmLayerItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        return section == 0 ? QString(tr("Feature")) : QStringLiteral("");
    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
    default:
        return QVariant();
    }
}

bool GwmLayerItemModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role)) {
        // FIXME: Implement me!
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}

QModelIndex GwmLayerItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    GwmLayerItem* parentItem = itemFromIndex(parent);
    GwmLayerItem* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex GwmLayerItemModel::parent(const QModelIndex &index) const
{
    int row = index.row();
    int col = index.column();

    // Find target item
    if (!index.isValid()) return QModelIndex();

    GwmLayerItem* childItem = itemFromIndex(index);
    GwmLayerItem* parentItem = childItem->parentItem();

    if (parentItem == mRootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

int GwmLayerItemModel::rowCount(const QModelIndex &parent) const
{
    GwmLayerItem* parentItem = this->itemFromIndex(parent);
    return parentItem->childCount();
}

int GwmLayerItemModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant GwmLayerItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    GwmLayerItem* item = itemFromIndex(index);
    return item->data(index.column(), role);

}

bool GwmLayerItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    GwmLayerItem* item = itemFromIndex(index);
    bool state = item->setData(index.column(), role, value);
    emit layerItemChangedSignal(item);
    return state;
}

Qt::ItemFlags GwmLayerItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    GwmLayerItem* item = itemFromIndex(index);

    return item->flags(); // FIXME: Implement me!
}

bool GwmLayerItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    GwmLayerItem* parentItem = itemFromIndex(parent);
    bool success = false;

    beginInsertRows(parent, row, row + count - 1);
    success = parentItem->insertChildren(row, count);
    endInsertRows();

    if (success)
    {
        emit layerAddedSignal();
    }

    return success;
}

//bool GwmLayerItemModel::insertColumns(int column, int count, const QModelIndex &parent)
//{
//    beginInsertColumns(parent, column, column + count - 1);
//    // FIXME: Implement me!
//    endInsertColumns();
//}

bool GwmLayerItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    GwmLayerItem* parentItem = itemFromIndex(parent);
    bool success = false;

    beginRemoveRows(parent, row, row + count - 1);
    success = parentItem->removeChildren(row, count);
    endRemoveRows();

    if (success)
    {
        emit layerRemovedSignal();
    }

    return success;
}

void GwmLayerItemModel::addLayer(QgsVectorLayer *layer)
{
    int nRow = mRootItem->childCount();
    beginInsertRows(QModelIndex(), nRow, nRow + 1);
    auto groupItem = new GwmLayerGroupItem(mRootItem, layer);
    mRootItem->appendChildren(groupItem);
    connect(groupItem->originChild(), &GwmLayerVectorItem::itemSymbolChangedSignal, this, &GwmLayerItemModel::onVectorItemSymbolChanged);
    endInsertRows();

    emit layerAddedSignal();
}

//bool GwmLayerItemModel::removeColumns(int column, int count, const QModelIndex &parent)
//{
//    beginRemoveColumns(parent, column, column + count - 1);
//    // FIXME: Implement me!
//    endRemoveColumns();
//}

GwmLayerItem* GwmLayerItemModel::itemFromIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {
        GwmLayerItem* item = static_cast<GwmLayerItem*>(index.internalPointer());
        if (item) return item;
    }
    return mRootItem;
}

QgsVectorLayer *GwmLayerItemModel::layerFromItem(GwmLayerItem* item) const
{
    switch (item->itemType())
    {
    case GwmLayerItem::GwmLayerItemType::Group:
        return ((GwmLayerGroupItem*)item)->originChild()->layer();
    case GwmLayerItem::GwmLayerItemType::Vector:
    case GwmLayerItem::GwmLayerItemType::Origin:
    case GwmLayerItem::GwmLayerItemType::GWR:
        return ((GwmLayerOriginItem*)item)->layer();
    default:
        return nullptr;
    }
}

QList<QgsMapLayer *> GwmLayerItemModel::toMapLayerList()
{
    QList<QgsMapLayer*> layerList;
    for (GwmLayerGroupItem* group : mRootItem->children())
    {
        if (group->checkState() == Qt::CheckState::Checked)
        {
            if (group->originChild()->checkState() == Qt::CheckState::Checked)
            {
                layerList += group->originChild()->layer();
            }
            for (GwmLayerVectorItem* analyse : group->analyseChildren())
            {
                if (group->checkState() == Qt::CheckState::Checked)
                {
                    layerList += analyse->layer();
                }
            }
        }
    }
    return layerList;
}

void GwmLayerItemModel::onVectorItemSymbolChanged()
{
    emit layoutChanged();
}

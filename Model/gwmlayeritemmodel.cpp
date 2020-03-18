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
    if (role != Qt::EditRole)
        return false;

    GwmLayerItem* item = itemFromIndex(index);
    item->setName(0, value);
}

Qt::ItemFlags GwmLayerItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index); // FIXME: Implement me!
}

bool GwmLayerItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    GwmLayerItem* parentItem = itemFromIndex(parent);
    bool success = false;

    beginInsertRows(parent, row, row + count - 1);
    success = parentItem->insertChildren(row, count);
    endInsertRows();

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

    return success;
}

void GwmLayerItemModel::addLayer(QgsVectorLayer *layer)
{
    int nRow = mRootItem->childCount();
    beginInsertRows(QModelIndex(), nRow, nRow + 1);
    auto groupItem = new GwmLayerGroupItem(mRootItem, layer);
    mRootItem->appendChildren(groupItem);
    endInsertRows();
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

void GwmLayerItemModel::setupModel()
{
    mRootItem = new GwmLayerItem();
//    for (int i = 0; i < 3; i++)
//    {
//        QString groupName = QString("Group %1").arg(i);
//        GwmLayerGroupItem* group = new GwmLayerGroupItem(groupName, mRootItem);
//        for (int s = 0; s < 5; s++)
//        {
//            QString symbolName = QString("Symbol %1").arg(s);
//            GwmLayerVectorItem* origin = group->originChild();
//            origin->symbolChildren().append(new GwmLayerSymbolItem(symbolName, origin));
//        }
//        for (int a = 0; a < 4; a++)
//        {
//            QString analyseName = QString("GWR %1").arg(a);
//            GwmLayerGWRItem* analyse = new GwmLayerGWRItem(analyseName, group);
//            for (int s = 0; s < 5; s++)
//            {
//                QString symbolName = QString("Symbol %1").arg(s);
//                analyse->symbolChildren().append(new GwmLayerSymbolItem(symbolName, analyse));
//            }
//            group->analyseChildren().append(analyse);
//        }
//        mRootItem->children().append(group);
//    }
}

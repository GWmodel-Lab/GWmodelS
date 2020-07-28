#include "gwmlayeritemmodel.h"
#include "gwmlayergwritem.h"
#include "gwmlayerggwritem.h"

#include <qgsproject.h>

GwmLayerItemModel::GwmLayerItemModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    mRootItem = new GwmLayerItem();
}

QVariant GwmLayerItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role) {
//    case Qt::DisplayRole:
//        return QStringLiteral("");
    case Qt::TextAlignmentRole:
        return Qt::AlignLeft;
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

//    qDebug() << "[GwmLayerItemModel::parent]"
//             << "row" << row << "col" << col << "index" << index;

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
    if (state)
    {
        emit layerItemChangedSignal(item);
    }
    return state;
}

Qt::ItemFlags GwmLayerItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    GwmLayerItem* item = itemFromIndex(index);

    return item->flags(); // FIXME: Implement me!
}

bool GwmLayerItemModel::insertItem(int row, GwmLayerItem *item, const QModelIndex &parent)
{
    GwmLayerItem* parentItem = itemFromIndex(parent);
    bool success = false;

    beginInsertRows(parent, row, row + 1 - 1);
    success = parentItem->insertChildren(row, QList<GwmLayerItem*>() << item);
    endInsertRows();

    if (success)
    {
		GwmLayerItem::GwmLayerItemType type = item->itemType();
		switch (type)
		{
        case GwmLayerItem::Base:
            break;
        case GwmLayerItem::Group:
        {
            auto groupItem = static_cast<GwmLayerGroupItem*>(item);
            QgsProject::instance()->addMapLayer(groupItem->originChild()->layer());
            connect(groupItem->originChild(), &GwmLayerVectorItem::itemSymbolChangedSignal, this, &GwmLayerItemModel::onVectorItemSymbolChanged);
            break;
        }
        case GwmLayerItem::Vector:
        case GwmLayerItem::Origin:
        case GwmLayerItem::GWR:
        case GwmLayerItem::Symbol:
        case GwmLayerItem::ScalableGWR:
        case GwmLayerItem::GeneralizedGWR:
        case GwmLayerItem::MultiscaleGWR:
        case GwmLayerItem::GWSS:
        case GwmLayerItem::CollinearityGWR:
        case GwmLayerItem::GWPCA:
        {
            auto vectorItem = static_cast<GwmLayerVectorItem*>(item);
            QgsProject::instance()->addMapLayer(vectorItem->layer());
            connect(vectorItem, &GwmLayerVectorItem::itemSymbolChangedSignal, this, &GwmLayerItemModel::onVectorItemSymbolChanged);
            break;
        }
        default:
            break;
		}
        emit layerAddedSignal();
    }

    return success;
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

	QList<QgsMapLayer*> layers;

	for (size_t i = row; i < row + count; i++)
	{
		GwmLayerItem* item = parentItem->child(row);
		GwmLayerItem::GwmLayerItemType type = item->itemType();
		switch (type)
		{
        case GwmLayerItem::Base:
            break;
        case GwmLayerItem::Group:
        {
            auto groupItem = static_cast<GwmLayerGroupItem*>(item);
            layers.append(groupItem->originChild()->layer());
            disconnect(groupItem->originChild(), &GwmLayerVectorItem::itemSymbolChangedSignal, this, &GwmLayerItemModel::onVectorItemSymbolChanged);
            break;
        }
        case GwmLayerItem::Vector:
        case GwmLayerItem::Origin:
        case GwmLayerItem::GWR:
        case GwmLayerItem::Symbol:
        case GwmLayerItem::ScalableGWR:
        case GwmLayerItem::GeneralizedGWR:
        case GwmLayerItem::MultiscaleGWR:
        case GwmLayerItem::GWSS:
        case GwmLayerItem::CollinearityGWR:
        case GwmLayerItem::GWPCA:
        {
            auto vectorItem = static_cast<GwmLayerVectorItem*>(item);
            layers.append(vectorItem->layer());
            disconnect(vectorItem, &GwmLayerVectorItem::itemSymbolChangedSignal, this, &GwmLayerItemModel::onVectorItemSymbolChanged);
            break;
        }
        default:
            break;
		}
	}

    beginRemoveRows(parent, row, row + count - 1);
    success = parentItem->removeChildren(row, count);
    endRemoveRows();

    if (success)
    {
        QgsProject::instance()->removeMapLayers(layers);
        emit layerRemovedSignal();
    }

    return success;
}

GwmLayerItem *GwmLayerItemModel::takeItem(int row, const QModelIndex &parent)
{
    qDebug() << "[GwmLayerItemModel::takeItem]"
             << "children count" << mRootItem->childCount()
             << "row" << row;

    QList<GwmLayerItem*> takenItems = takeRows(row, 1, parent);
	if (takenItems.size() > 0)
	{
		GwmLayerItem* item = takenItems.first();
		GwmLayerItem::GwmLayerItemType type = item->itemType();
		switch (type)
		{
        case GwmLayerItem::Base:
            break;
        case GwmLayerItem::Group:
        {
            auto groupItem = static_cast<GwmLayerGroupItem*>(item);
            QgsProject::instance()->addMapLayer(groupItem->originChild()->layer());
            connect(groupItem->originChild(), &GwmLayerVectorItem::itemSymbolChangedSignal, this, &GwmLayerItemModel::onVectorItemSymbolChanged);
            break;
        }
        case GwmLayerItem::Vector:
        case GwmLayerItem::Origin:
        case GwmLayerItem::GWR:
        case GwmLayerItem::Symbol:
        case GwmLayerItem::ScalableGWR:
        case GwmLayerItem::GeneralizedGWR:
        case GwmLayerItem::MultiscaleGWR:
        case GwmLayerItem::GWSS:
        case GwmLayerItem::CollinearityGWR:
        case GwmLayerItem::GWPCA:
        {
            auto vectorItem = static_cast<GwmLayerVectorItem*>(item);
            QgsProject::instance()->addMapLayer(vectorItem->layer());
            connect(vectorItem, &GwmLayerVectorItem::itemSymbolChangedSignal, this, &GwmLayerItemModel::onVectorItemSymbolChanged);
            break;
        }
        default:
            break;
		}
		return takenItems.first();
	}
    else
        return nullptr;

}

bool GwmLayerItemModel::appentItem(GwmLayerItem *item, const QModelIndex &parent)
{
    GwmLayerItem* parentItem = itemFromIndex(parent);
    int row = parentItem->childCount();
    bool success = false;

    beginInsertRows(parent, row, row + 1);
    success = parentItem->appendChildren(QList<GwmLayerItem*>() << item);
    endInsertRows();

	if (success)
	{
        item->setParentItem(parentItem);
		GwmLayerItem::GwmLayerItemType type = item->itemType();
		switch (type)
		{
        case GwmLayerItem::Base:
            break;
        case GwmLayerItem::Group:
        {
            auto groupItem = static_cast<GwmLayerGroupItem*>(item);
            QgsProject::instance()->addMapLayer(groupItem->originChild()->layer());
            connect(groupItem->originChild(), &GwmLayerVectorItem::itemSymbolChangedSignal, this, &GwmLayerItemModel::onVectorItemSymbolChanged);
            for (auto item : groupItem->analyseChildren())
            {
                connect(item, &GwmLayerVectorItem::itemSymbolChangedSignal, this, &GwmLayerItemModel::onVectorItemSymbolChanged);
            }
            break;
        }
        case GwmLayerItem::Vector:
        case GwmLayerItem::Origin:
        case GwmLayerItem::GWR:
        case GwmLayerItem::Symbol:
        case GwmLayerItem::ScalableGWR:
        case GwmLayerItem::GeneralizedGWR:
        case GwmLayerItem::MultiscaleGWR:
        case GwmLayerItem::GWSS:
        case GwmLayerItem::CollinearityGWR:
        case GwmLayerItem::GWPCA:
        {
            auto vectorItem = static_cast<GwmLayerVectorItem*>(item);
            QgsProject::instance()->addMapLayer(vectorItem->layer());
            connect(vectorItem, &GwmLayerVectorItem::itemSymbolChangedSignal, this, &GwmLayerItemModel::onVectorItemSymbolChanged);
            break;
        }
        default:
            break;
		}
		emit layerAddedSignal();
	}

    return success;
}

QList<GwmLayerItem *> GwmLayerItemModel::takeRows(int row, int count, const QModelIndex &parent)
{
    GwmLayerItem* parentItem = itemFromIndex(parent);
    QList<GwmLayerItem *> takenItems;

    beginRemoveRows(parent, row, row + count - 1);

    takenItems = parentItem->takeChildren(row, count);

    qDebug() << "[GwmLayerItemModel::takeItem]"
           << "children count" << mRootItem->childCount();

    endRemoveRows();

    if (takenItems.size() > 0)
    {
		for (size_t i = 0; i < count; i++)
		{
			GwmLayerItem* item = takenItems[i];
			GwmLayerItem::GwmLayerItemType type = item->itemType();
			switch (type)
			{
				case GwmLayerItem::Base:
					break;
				case GwmLayerItem::Group:
					QgsProject::instance()->removeMapLayer(static_cast<GwmLayerGroupItem*>(item)->originChild()->layer());
					break;
				case GwmLayerItem::Vector:
				case GwmLayerItem::Origin:
				case GwmLayerItem::GWR:
				case GwmLayerItem::Symbol:
				case GwmLayerItem::ScalableGWR:
				case GwmLayerItem::GeneralizedGWR:
				case GwmLayerItem::MultiscaleGWR:
				case GwmLayerItem::GWSS:
				case GwmLayerItem::CollinearityGWR:
				case GwmLayerItem::GWPCA:
					QgsProject::instance()->removeMapLayer(static_cast<GwmLayerVectorItem*>(item)->layer());
				default:
					break;
			}
		}
        emit layerRemovedSignal();
    }
    return takenItems;
}

void GwmLayerItemModel::appendItem(QgsVectorLayer *layer, const QString path, const QString provider)
{
    int nRow = mRootItem->childCount();

    beginInsertRows(QModelIndex(), nRow, nRow + 1);
    auto groupItem = new GwmLayerGroupItem(mRootItem, layer);
    groupItem->originChild()->setPath(path);
    groupItem->originChild()->setProvider(provider);
    bool success = mRootItem->appendChildren(QList<GwmLayerItem*>() << groupItem);
	if (success)
	{
		QgsProject::instance()->addMapLayer(layer);
		connect(groupItem->originChild(), &GwmLayerVectorItem::itemSymbolChangedSignal, this, &GwmLayerItemModel::onVectorItemSymbolChanged);
	}
    endInsertRows();

    emit layerAddedSignal();
}

GwmLayerGroupItem *GwmLayerItemModel::item(int i)
{
    if (i >= 0 && i < mRootItem->childCount())
        return mRootItem->children().at(i);
    else return nullptr;
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

QModelIndex GwmLayerItemModel::indexFromItem(GwmLayerItem* item) const
{
    if (item)
    {
        qDebug() << "[GwmLayerItemModel::indexFromItem]"
                 << item->itemType() << item->text();
        GwmLayerItem* parentItem = item->parentItem();
        if (parentItem)
            return createIndex(item->childNumber(), 0, item);
        else
            return QModelIndex();
    }
    else return QModelIndex();
}

QgsVectorLayer *GwmLayerItemModel::layerFromItem(GwmLayerItem* item) const
{
    switch (item->itemType())
    {
    case GwmLayerItem::GwmLayerItemType::Base:
        return nullptr;
    case GwmLayerItem::GwmLayerItemType::Symbol:
        return  nullptr;
    case GwmLayerItem::GwmLayerItemType::Group:
        return ((GwmLayerGroupItem*)item)->originChild()->layer();
    case GwmLayerItem::GwmLayerItemType::Vector:
    case GwmLayerItem::GwmLayerItemType::Origin:
    case GwmLayerItem::GwmLayerItemType::GWR:
    case GwmLayerItem::GwmLayerItemType::ScalableGWR:
    case GwmLayerItem::GwmLayerItemType::GeneralizedGWR:
    case GwmLayerItem::GwmLayerItemType::MultiscaleGWR:
    case GwmLayerItem::GwmLayerItemType::GWSS:
    case GwmLayerItem::GwmLayerItemType::CollinearityGWR:
    case GwmLayerItem::GwmLayerItemType::GWPCA:
        return ((GwmLayerOriginItem*)item)->layer();
    default:
        return nullptr;
    }
}

QList<GwmLayerGroupItem *> GwmLayerItemModel::rootChildren()
{
    return mRootItem->children();
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
                layerList.push_back(group->originChild()->layer());
            }
            for (GwmLayerVectorItem* analyse : group->analyseChildren())
            {
                if (analyse->checkState() == Qt::CheckState::Checked)
                {
                    layerList.push_back(analyse->layer());
                }
            }
        }
    }
    return layerList;
}

bool GwmLayerItemModel::canMoveUp(const QModelIndex &index)
{
    int row = index.row();
    GwmLayerItem* item = itemFromIndex(index);
    switch (item->itemType())
    {
    case GwmLayerItem::Group:
        return row > 0 && row < item->parentItem()->childCount();
    case GwmLayerItem::GWR:
    case GwmLayerItem::GeneralizedGWR:
    case GwmLayerItem::ScalableGWR:
    case GwmLayerItem::MultiscaleGWR:
    case GwmLayerItem::CollinearityGWR:
    case GwmLayerItem::GWSS:
    case GwmLayerItem::GWPCA:
        return row > 1 && row < (item->parentItem()->childCount() - 1);
    default:
        return false;
    }
}

bool GwmLayerItemModel::canMoveDown(const QModelIndex &index)
{
    int row = index.row();
    GwmLayerItem* item = itemFromIndex(index);
    switch (item->itemType())
    {
    case GwmLayerItem::Group:
        return row >= 0 && row < (item->parentItem()->childCount() - 1);
    case GwmLayerItem::GWR:
    case GwmLayerItem::GeneralizedGWR:
    case GwmLayerItem::ScalableGWR:
    case GwmLayerItem::MultiscaleGWR:
    case GwmLayerItem::GWSS:
    case GwmLayerItem::CollinearityGWR:
    case GwmLayerItem::GWPCA:
        return row >= 1 && row < (item->parentItem()->childCount() - 2);
    default:
        return false;
    }
}

bool GwmLayerItemModel::canRemove(const QModelIndex &index)
{
    GwmLayerItem* item = itemFromIndex(index);
    switch (item->itemType())
    {
    case GwmLayerItem::Group:
    case GwmLayerItem::GWR:
    case GwmLayerItem::GeneralizedGWR:
    case GwmLayerItem::ScalableGWR:
    case GwmLayerItem::MultiscaleGWR:
    case GwmLayerItem::GWSS:
    case GwmLayerItem::CollinearityGWR:
    case GwmLayerItem::GWPCA:
        return true;
    default:
        return false;
    }
}

bool GwmLayerItemModel::canSetSymbol(const QModelIndex &index)
{
    GwmLayerItem* item = itemFromIndex(index);
    switch (item->itemType())
    {
    case GwmLayerItem::Group:
    case GwmLayerItem::Origin:
    case GwmLayerItem::GWR:
    case GwmLayerItem::GeneralizedGWR:
    case GwmLayerItem::ScalableGWR:
    case GwmLayerItem::MultiscaleGWR:
    case GwmLayerItem::GWSS:
    case GwmLayerItem::CollinearityGWR:
    case GwmLayerItem::GWPCA:
        return true;
    default:
        return false;
    }
}

void GwmLayerItemModel::moveUp(const QModelIndex &index)
{
    int row = index.row();
    GwmLayerItem* item = itemFromIndex(index);
    GwmLayerItem* parent = item->parentItem();
    parent->moveChildren(row, 1, row - 1);
    emit layoutChanged();
    emit layerItemMovedSignal();
}

void GwmLayerItemModel::moveDown(const QModelIndex &index)
{
    int row = index.row();
    GwmLayerItem* item = itemFromIndex(index);
    GwmLayerItem* parent = item->parentItem();
    parent->moveChildren(row, 1, row + 1);
    emit layoutChanged();
    emit layerItemMovedSignal();
}

void GwmLayerItemModel::remove(const QModelIndex &index)
{
    int row = index.row();
    //GwmLayerItem* item = itemFromIndex(index);
    //GwmLayerItem* parent = item->parentItem();
    //parent->removeChildren(row, 1);
	bool success = removeRows(row, 1, index.parent());
	if (success)
	{
	    emit layerRemovedSignal();
	}
}

void GwmLayerItemModel::onVectorItemSymbolChanged()
{
    emit layoutChanged();
}

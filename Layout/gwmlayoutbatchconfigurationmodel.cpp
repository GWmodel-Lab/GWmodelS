#include "gwmlayoutbatchconfigurationmodel.h"

#include <qgsvectorlayer.h>
#include <qgsrenderer.h>

GwmLayoutBatchConfigurationItem::GwmLayoutBatchConfigurationItem(GwmLayoutBatchConfigurationItem *parent)
{
    mParent = parent;
}

GwmLayoutBatchConfigurationItem::~GwmLayoutBatchConfigurationItem()
{

}

GwmLayoutBatchConfigurationItemField::GwmLayoutBatchConfigurationItemField(GwmLayoutBatchConfigurationItem *parent) : GwmLayoutBatchConfigurationItem(parent)
{

}

GwmLayoutBatchConfigurationItemField::GwmLayoutBatchConfigurationItemField(const QgsFieldCheckable& field, const QgsVectorLayer* layer, GwmLayoutBatchConfigurationItem *parent)
    : GwmLayoutBatchConfigurationItem(parent)
{
    mName = field.name();
    mRenderer = layer->renderer()->clone();
}

GwmLayoutBatchConfigurationItemField::~GwmLayoutBatchConfigurationItemField()
{
    if (mRenderer) delete mRenderer;
}

GwmLayoutBatchConfigurationItem *GwmLayoutBatchConfigurationItemField::child(int i)
{
    return nullptr;
}

QVariant GwmLayoutBatchConfigurationItemField::data(int column, int role) const
{
    QMap<int, QList<QVariant> > dataMap;
    dataMap[Qt::DisplayRole] = { name(), QVariant::fromValue((void*)mRenderer) };
    QVariant value = dataMap.value(role, {QVariant(), QVariant()})[column];
    return value;
}

void GwmLayoutBatchConfigurationItemField::setData(int column, int role, const QVariant &value)
{
    QMap<QPair<int, int>, Setter> setterMap = {
        std::make_pair(qMakePair(Qt::EditRole, 0), [](GwmLayoutBatchConfigurationItemField* self, const QVariant& value)
        {
            self->setName(value.toString());
        }),
        std::make_pair(qMakePair(Qt::EditRole, 1), [](GwmLayoutBatchConfigurationItemField* self, const QVariant& value)
        {
            self->setRenderer(value.value<QgsFeatureRenderer*>());
        })
    };
    Setter setter = setterMap.value(qMakePair(role, column), [](GwmLayoutBatchConfigurationItemField* self, const QVariant& value)
    {

    });
    setter(this, value);
}

bool GwmLayoutBatchConfigurationItemField::insertChild(int i, GwmLayoutBatchConfigurationItem *item)
{
    return false;
}

bool GwmLayoutBatchConfigurationItemField::removeChild(int i)
{
    return false;
}

void GwmLayoutBatchConfigurationItemField::setRenderer(QgsFeatureRenderer *renderer)
{
    if (renderer)
    {
        if (mRenderer) delete mRenderer;
        mRenderer = renderer->clone();
    }
}

Qt::ItemFlags GwmLayoutBatchConfigurationItemField::flags(int column)
{
    QMap<int, Qt::ItemFlags> flagMapper =
    {
        std::make_pair(0, Qt::ItemIsEnabled | Qt::ItemIsSelectable),
        std::make_pair(1, Qt::ItemIsEnabled)
    };
    auto value = flagMapper.value(column, Qt::NoItemFlags);
    return value;
}


GwmLayoutBatchConfigurationItemLayer::GwmLayoutBatchConfigurationItemLayer(GwmLayoutBatchConfigurationItem *parent) : GwmLayoutBatchConfigurationItem(parent)
{

}

GwmLayoutBatchConfigurationItemLayer::GwmLayoutBatchConfigurationItemLayer(QgsVectorLayer *layer, GwmLayoutBatchConfigurationItem *parent) : GwmLayoutBatchConfigurationItem(parent)
{
    mName = layer->name();
    mLayer = layer;
}

GwmLayoutBatchConfigurationItemLayer::~GwmLayoutBatchConfigurationItemLayer()
{
    for (auto field : mFieldList)
        delete field;
}

Qt::ItemFlags GwmLayoutBatchConfigurationItemLayer::flags(int column)
{
    QMap<int, Qt::ItemFlags> flagMapper =
    {
        std::make_pair(0, Qt::ItemIsEnabled | Qt::ItemIsSelectable)
    };
    return flagMapper.value(column, Qt::NoItemFlags);
}

GwmLayoutBatchConfigurationItem *GwmLayoutBatchConfigurationItemLayer::child(int i)
{
    if (i < mFieldList.size())
        return mFieldList[i];
    else return nullptr;
}

int GwmLayoutBatchConfigurationItemLayer::childNumber(GwmLayoutBatchConfigurationItem *child)
{
    if (child->type() == Type::Field)
    {
        return mFieldList.indexOf(static_cast<GwmLayoutBatchConfigurationItemField*>(child));
    }
    else return -1;
}

QVariant GwmLayoutBatchConfigurationItemLayer::data(int column, int role) const
{
    QMap<QPair<int, int>, QVariant> dataMap =
    {
        std::make_pair(qMakePair(Qt::DisplayRole, 0), name())
    };
    return dataMap.value(qMakePair(role, column), QVariant());
}

void GwmLayoutBatchConfigurationItemLayer::setData(int column, int role, const QVariant &value)
{
    if (column == 0)
    {
        switch (role) {
        case Qt::EditRole:
            setName(value.toString());
            break;
        default:
            break;
        }
    }
}

bool GwmLayoutBatchConfigurationItemLayer::insertChild(int i, GwmLayoutBatchConfigurationItem *item)
{
    if (i < 0 || i > size() || item->type() != Type::Field)
        return false;

    if (i == size())
    {
        mFieldList.append(static_cast<GwmLayoutBatchConfigurationItemField*>(item));
    }
    else
    {
        mFieldList.insert(i, static_cast<GwmLayoutBatchConfigurationItemField*>(item));
    }
    return true;
}

bool GwmLayoutBatchConfigurationItemLayer::removeChild(int i)
{
    if (i >= 0 || i < size())
    {
        GwmLayoutBatchConfigurationItemField* item = mFieldList.takeAt(i);
        delete item;
        return true;
    }
    else return false;
}

GwmLayoutBatchConfigurationItemRoot::GwmLayoutBatchConfigurationItemRoot() : GwmLayoutBatchConfigurationItem(nullptr)
{

}

GwmLayoutBatchConfigurationItemRoot::~GwmLayoutBatchConfigurationItemRoot()
{
    for (auto layer : mLayerList)
        delete layer;
}

Qt::ItemFlags GwmLayoutBatchConfigurationItemRoot::flags(int column)
{
    return Qt::NoItemFlags;
}

GwmLayoutBatchConfigurationItem *GwmLayoutBatchConfigurationItemRoot::child(int i)
{
    if (i < mLayerList.size())
        return mLayerList[i];
    else return nullptr;
}

int GwmLayoutBatchConfigurationItemRoot::childNumber(GwmLayoutBatchConfigurationItem *child)
{
    if (child->type() == Type::Layer)
    {
        return mLayerList.indexOf(static_cast<GwmLayoutBatchConfigurationItemLayer*>(child));
    }
    else return -1;
}

QVariant GwmLayoutBatchConfigurationItemRoot::data(int column, int role) const
{
    return QVariant();
}

void GwmLayoutBatchConfigurationItemRoot::setData(int column, int role, const QVariant &value)
{

}

bool GwmLayoutBatchConfigurationItemRoot::insertChild(int i, GwmLayoutBatchConfigurationItem *item)
{
    if (i < 0 || i > size() || item->type() != Type::Layer)
        return false;

    if (i == size())
    {
        mLayerList.append(static_cast<GwmLayoutBatchConfigurationItemLayer*>(item));
    }
    else
    {
        mLayerList.insert(i, static_cast<GwmLayoutBatchConfigurationItemLayer*>(item));
    }
    return true;
}

bool GwmLayoutBatchConfigurationItemRoot::removeChild(int i)
{
    if (i >= 0 || i < size())
    {
        GwmLayoutBatchConfigurationItemLayer* item = mLayerList.takeAt(i);
        delete item;
        return true;
    }
    else return false;
}

GwmLayoutBatchConfigurationModel::GwmLayoutBatchConfigurationModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    mItemRoot = new GwmLayoutBatchConfigurationItemRoot();
}

QVariant GwmLayoutBatchConfigurationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QList<QVariant> headerList = { QVariant(), QVariant() };
    if (orientation == Qt::Orientation::Horizontal)
    {
        switch (role) {
        case Qt::DisplayRole:
            headerList = { tr("Layer/Field"), tr("Symbol") };
            break;
        default:
            break;
        }
    }
    return headerList[section];
}

//bool GwmLayoutBatchConfigurationModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
//{
//    if (value != headerData(section, orientation, role)) {
//        // FIXME: Implement me!
//        emit headerDataChanged(orientation, section, section);
//        return true;
//    }
//    return false;
//}

QModelIndex GwmLayoutBatchConfigurationModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid() && parent.column() > 1)
        return QModelIndex();

    GwmLayoutBatchConfigurationItem* parentItem = itemFromIndex(parent);
    GwmLayoutBatchConfigurationItem* childItem = parentItem->child(row);
    if (childItem) return createIndex(row, column, childItem);
    else return QModelIndex();

}

QModelIndex GwmLayoutBatchConfigurationModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    if (!index.isValid()) return QModelIndex();

    GwmLayoutBatchConfigurationItem* childItem = itemFromIndex(index);
    GwmLayoutBatchConfigurationItem* parentItem = childItem->parent();

    if (parentItem == mItemRoot)
        return QModelIndex();

    return createIndex(parentItem->parent()->childNumber(parentItem), 0, parentItem);
}

int GwmLayoutBatchConfigurationModel::rowCount(const QModelIndex &parent) const
{
    int size = 0;
//    if (!parent.isValid())
//        size = mItemRoot->size();

    size = itemFromIndex(parent)->size();
    return size;
}

int GwmLayoutBatchConfigurationModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 2;

    return 2;
}

QVariant GwmLayoutBatchConfigurationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    GwmLayoutBatchConfigurationItem* item = itemFromIndex(index);
    return item ? item->data(index.column(), role) : QVariant();
}

bool GwmLayoutBatchConfigurationModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        GwmLayoutBatchConfigurationItem* item = itemFromIndex(index);
        if (item) item->setData(index.column(), role, value);
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags GwmLayoutBatchConfigurationModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    GwmLayoutBatchConfigurationItem* item = itemFromIndex(index);
    return (item) ? item->flags(index.column()) : Qt::NoItemFlags;
}

bool GwmLayoutBatchConfigurationModel::insertLayer(int row, QgsVectorLayer* layer)
{
    beginInsertRows(QModelIndex(), row, row);
    GwmLayoutBatchConfigurationItemLayer* layerItem = new GwmLayoutBatchConfigurationItemLayer(layer, mItemRoot);
    bool success = mItemRoot->insertChild(row, layerItem);
    endInsertRows();

    return success;
}

bool GwmLayoutBatchConfigurationModel::removeLayer(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    bool success = mItemRoot->removeChild(row);
    endRemoveRows();
    return success;
}

bool GwmLayoutBatchConfigurationModel::removeLayer(const QgsVectorLayer *layer)
{
    const QModelIndex& layerIndex = indexFromLayer(layer);
    if (layerIndex.isValid())
    {
        return removeLayer(layerIndex.row());
    }
    return false;
}

bool GwmLayoutBatchConfigurationModel::insertField(int row, const QgsFieldCheckable &item, const QModelIndex &parent)
{
    if (!parent.isValid())
        return false;

    GwmLayoutBatchConfigurationItem* parentItem = itemFromIndex(parent);
    if (parentItem && parentItem->type() == GwmLayoutBatchConfigurationItem::Type::Layer)
    {
        GwmLayoutBatchConfigurationItemLayer* layerItem = static_cast<GwmLayoutBatchConfigurationItemLayer*>(parentItem);
        GwmLayoutBatchConfigurationItemField* fieldItem = new GwmLayoutBatchConfigurationItemField(item, layerItem->layer(), parentItem);
        beginInsertRows(parent, row, row);
        bool success = layerItem->insertChild(row, fieldItem);
        endInsertRows();
        return success;
    }
    else return false;
}

bool GwmLayoutBatchConfigurationModel::removeField(int row, const QModelIndex &parent)
{
    if (!parent.isValid())
        return false;

    GwmLayoutBatchConfigurationItem* parentItem = itemFromIndex(parent);
    if (parentItem && parentItem->type() == GwmLayoutBatchConfigurationItem::Type::Layer)
    {
        GwmLayoutBatchConfigurationItemLayer* layerItem = static_cast<GwmLayoutBatchConfigurationItemLayer*>(parentItem);
        beginInsertRows(parent, row, row);
        bool success = layerItem->removeChild(row);
        endInsertRows();
        return success;
    }
    else return false;
}

bool GwmLayoutBatchConfigurationModel::removeField(const QgsFieldCheckable &fieldItem, const QModelIndex &parent)
{
    const QModelIndex& fieldIndex = indexFromField(fieldItem, parent);
    if (fieldIndex.isValid())
    {
        return removeField(fieldIndex.row(), parent);
    }
    return false;
}

bool GwmLayoutBatchConfigurationModel::setFieldRenderer(const QModelIndex &index, QgsFeatureRenderer *renderer)
{
    if (index.isValid())
    {
        GwmLayoutBatchConfigurationItem* item = itemFromIndex(index);
        if (item->type() == GwmLayoutBatchConfigurationItem::Field)
        {
            GwmLayoutBatchConfigurationItemField* fieldItem = static_cast<GwmLayoutBatchConfigurationItemField*>(item);
            fieldItem->setRenderer(renderer);
            return true;
        }
    }
    return false;
}

//bool GwmLayoutBatchConfigurationModel::insertRows(int row, int count, const QModelIndex &parent)
//{
//    beginInsertRows(parent, row, row + count - 1);
//    // FIXME: Implement me!
//    endInsertRows();
//}

//bool GwmLayoutBatchConfigurationModel::insertColumns(int column, int count, const QModelIndex &parent)
//{
//    beginInsertColumns(parent, column, column + count - 1);
//    // FIXME: Implement me!
//    endInsertColumns();
//}

//bool GwmLayoutBatchConfigurationModel::removeRows(int row, int count, const QModelIndex &parent)
//{
//    beginRemoveRows(parent, row, row + count - 1);
//    // FIXME: Implement me!
//    endRemoveRows();
//}

//bool GwmLayoutBatchConfigurationModel::removeColumns(int column, int count, const QModelIndex &parent)
//{
//    beginRemoveColumns(parent, column, column + count - 1);
//    // FIXME: Implement me!
//    endRemoveColumns();
//}

GwmLayoutBatchConfigurationItem *GwmLayoutBatchConfigurationModel::itemFromIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {
        GwmLayoutBatchConfigurationItem* item = static_cast<GwmLayoutBatchConfigurationItem*>(index.internalPointer());
        if (item) return item;
    }
    return mItemRoot;
}

QModelIndex GwmLayoutBatchConfigurationModel::indexFromLayer(const QgsVectorLayer *layer) const
{
    if (layer)
    {
        int size = mItemRoot->size();
        for (int i = 0; i < size; i++)
        {
            GwmLayoutBatchConfigurationItemLayer* layerItem = static_cast<GwmLayoutBatchConfigurationItemLayer*>(mItemRoot->child(i));
            if (layerItem->layer() == layer)
            {
                return createIndex(i, 0, layerItem);
            }
        }
    }
    return QModelIndex();
}

QModelIndex GwmLayoutBatchConfigurationModel::indexFromField(const QgsFieldCheckable &field, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return QModelIndex();

    GwmLayoutBatchConfigurationItem* parentItem = itemFromIndex(parent);
    if (parentItem && parentItem->type() == GwmLayoutBatchConfigurationItem::Type::Layer)
    {
        GwmLayoutBatchConfigurationItemLayer* layerItem = static_cast<GwmLayoutBatchConfigurationItemLayer*>(parentItem);
        int size = layerItem->size();
        for (int row = 0; row < size; row++)
        {
            GwmLayoutBatchConfigurationItemField* fieldItem = static_cast<GwmLayoutBatchConfigurationItemField*>(layerItem->child(row));
            if (fieldItem->name() == field.name())
            {
                return createIndex(row, 0, fieldItem);
            }
        }
    }
    return QModelIndex();
}

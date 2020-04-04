#include "gwmlayerattributeitemmodel.h"
#include <qdebug.h>

GwmLayerAttributeItemModel::GwmLayerAttributeItemModel(QObject *parent)
    : QAbstractItemModel(parent)
{
//    attributeItem = new GwmLayerAttributeItem();
    attributeItemList = QList<GwmLayerAttributeItem*>();
}

GwmLayerAttributeItemModel::~GwmLayerAttributeItemModel()
{
    for (GwmLayerAttributeItem* item : attributeItemList)
    {
        delete item;
    }
    attributeItemList.clear();
}

QVariant GwmLayerAttributeItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
    switch (role) {
//    case Qt::DisplayRole:
//        return section == 0 ? QString(tr("Feature")) : QStringLiteral("");
    case Qt::TextAlignmentRole:
        return Qt::AlignLeft;
    default:
        return QVariant();
    }
}

bool GwmLayerAttributeItemModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role)) {
        // FIXME: Implement me!
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}

QModelIndex GwmLayerAttributeItemModel::index(int row, int column, const QModelIndex &parent) const
{
    // FIXME: Implement me!
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();
    return createIndex(row,column);
}

QModelIndex GwmLayerAttributeItemModel::parent(const QModelIndex &index) const
{
    // FIXME: Implement me!
    if (!index.isValid()) return QModelIndex();
    return QModelIndex();
}

int GwmLayerAttributeItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return attributeItemList.count();
    // FIXME: Implement me!
}

int GwmLayerAttributeItemModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 1;
    return 1;
    // FIXME: Implement me!
}

QVariant GwmLayerAttributeItemModel::data(const QModelIndex &index, int role) const
{
    GwmLayerAttributeItem* item = itemFromIndex(index);
    if (role == Qt::DisplayRole) {
        return item->data(index.column(),role);
    }
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }
    return QVariant();
}
Qt::ItemFlags GwmLayerAttributeItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    GwmLayerAttributeItem* item = itemFromIndex(index);

    return item->flags(); // FIXME: Implement me!
}

bool GwmLayerAttributeItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    GwmLayerAttributeItem* item = itemFromIndex(index);
//    bool state = item->setData(role, value);
//    return state;
    if (value != data(index, role)) {
        // FIXME: Implement me!

        emit dataChanged(index,index);
        return item->setData(index.column(),role, value);
    }
    return false;
}

bool GwmLayerAttributeItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    GwmLayerAttributeItem* parentItem = itemFromIndex(parent);
    bool success = true;
    beginInsertRows(parent, row, row + count - 1);

    for (int num = 0; num < count; num++)
    {
        GwmLayerAttributeItem* item = new GwmLayerAttributeItem();
        attributeItemList.insert(num, item);
    }
    endInsertRows();

    return success;
}


bool GwmLayerAttributeItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    GwmLayerAttributeItem* parentItem = itemFromIndex(parent);
    bool success = false;

    beginRemoveRows(parent, row, row + count - 1);
    for (int num = 0; num < count; num++)
        success = attributeItemList.removeOne(attributeItemList[row+num]);
    endRemoveRows();

    return success;
}

bool GwmLayerAttributeItemModel::clear(){
    for (GwmLayerAttributeItem* item : attributeItemList)
    {
        delete item;
    }
    attributeItemList.clear();
    return true;
}

QList<GwmLayerAttributeItem*> GwmLayerAttributeItemModel::findItems(QString attributeName){
    QList<GwmLayerAttributeItem*> itemList;
    for(GwmLayerAttributeItem* item : attributeItemList){
        if(item->text() == attributeName){
            itemList.append(item);
        }
    }
    return itemList;
}

void GwmLayerAttributeItemModel::appendItem(int index, const QString attributeName, const QString type)
{
    GwmLayerAttributeItem* item = new GwmLayerAttributeItem(index,attributeName,type);
    int row = attributeItemList.count()-1;
    beginInsertRows(QModelIndex(), row, row + 1);
    attributeItemList.append(item);
    endInsertRows();
}

void GwmLayerAttributeItemModel::appendRow(GwmLayerAttributeItem *item){
    int row = attributeItemList.count()-1;
    beginInsertRows(QModelIndex(), row, row + 1);
    attributeItemList.append(item);
    endInsertRows();
}

QModelIndex GwmLayerAttributeItemModel::indexFromItem(GwmLayerAttributeItem* item) const
{
    if (item)
    {
       return createIndex(attributeItemList.indexOf(item),0);
    }
    else
        return QModelIndex();
}

GwmLayerAttributeItem* GwmLayerAttributeItemModel::itemFromIndex(const QModelIndex &index) const{
    if(index.isValid()){
        return attributeItemList[index.row()];
    }
    return nullptr;
}

GwmLayerAttributeItem* GwmLayerAttributeItemModel::item(int i){
    return attributeItemList[i]? attributeItemList[i]:nullptr;
}

QList<QString> GwmLayerAttributeItemModel::toLayerAttributeList(){
    QList<QString> layerAttributeList;
    for(GwmLayerAttributeItem* item :attributeItemList){
        layerAttributeList.append(item->text());
    }
    return  layerAttributeList;
}

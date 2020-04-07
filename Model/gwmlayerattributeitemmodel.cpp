#include "gwmlayerattributeitemmodel.h"
#include <qdebug.h>

GwmLayerAttributeItemModel::GwmLayerAttributeItemModel(QObject *parent)
    : QAbstractItemModel(parent)
{
//    attributeItem = new GwmLayerAttributeItem();
    mAttributeItemList = QList<GwmLayerAttributeItem*>();
}

GwmLayerAttributeItemModel::~GwmLayerAttributeItemModel()
{
    for (GwmLayerAttributeItem* item : mAttributeItemList)
    {
        delete item;
    }
    mAttributeItemList.clear();
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
    if(column == 0){
        return createIndex(row,column);
    }
    else
        return QModelIndex();
}

QModelIndex GwmLayerAttributeItemModel::parent(const QModelIndex &index) const
{
    // FIXME: Implement me!
    if (!index.isValid()) return QModelIndex();
//    if(index.row() < mAttributeItemList.count()&&index.column()==0)
//        return index;
//    else
        return QModelIndex();
}

int GwmLayerAttributeItemModel::rowCount(const QModelIndex &parent) const
{
//    if (parent.isValid())
//        return 1;
    return mAttributeItemList.count() ;
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
    if (!index.isValid())
        return QVariant();
    GwmLayerAttributeItem* item = itemFromIndex(index);
    switch (role) {
        case Qt::DisplayRole:
            return item->data(index.column(),role);
//        case Qt::TextAlignmentRole:
//            return Qt::AlignCenter;
        default:
            break;
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
//    GwmLayerAttributeItem* parentItem = itemFromIndex(parent);
    bool success = true;
    beginInsertRows(QModelIndex(), row, row + count - 1);

    for (int num = row; num <  row + count; num++)
    {
        GwmLayerAttributeItem* item = new GwmLayerAttributeItem();
        mAttributeItemList.insert(num, item);
    }
    endInsertRows();

    return success;
}


bool GwmLayerAttributeItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
//    GwmLayerAttributeItem* parentItem = itemFromIndex(parent);
    bool success = false;
//    qDebug() << rowCount();
//    qDebug() << count;
    if (rowCount() == row + count){
        beginRemoveRows(QModelIndex(), row - 1, row + count - 2);
    }
    else{
        beginRemoveRows(QModelIndex(), row, row + count - 1);
    }
    for (int num = 0; num < count; num++){
//         qDebug() << num;
        GwmLayerAttributeItem* item = mAttributeItemList[row + num];
        success = mAttributeItemList.removeOne(item);
        delete item;
    }
    endRemoveRows();

    return success;
}

bool GwmLayerAttributeItemModel::clear(){
    beginRemoveRows(QModelIndex(),0,rowCount()-1);
    for (GwmLayerAttributeItem* item : mAttributeItemList)
    {
        delete item;
    }
    mAttributeItemList.clear();
    endRemoveRows();
    return true;
}

QList<GwmLayerAttributeItem*> GwmLayerAttributeItemModel::findItems(QString attributeName){
    QList<GwmLayerAttributeItem*> itemList;
    for(GwmLayerAttributeItem* item : mAttributeItemList){
        if(item->text() == attributeName){
            itemList.append(item);
        }
    }
    return itemList;
}

void GwmLayerAttributeItemModel::appendItem(int index, const QString attributeName,const QVariant::Type type)
{
    GwmLayerAttributeItem* item = new GwmLayerAttributeItem(index,attributeName,type);
    int row = mAttributeItemList.count();
    beginInsertRows(QModelIndex(), row, row);
    mAttributeItemList.append(item);
    endInsertRows();
}

void GwmLayerAttributeItemModel::appendRow(GwmLayerAttributeItem *item){
    int row = mAttributeItemList.count();
    beginInsertRows(QModelIndex(), row, row);
    mAttributeItemList.append(item);
    endInsertRows();
}

QModelIndex GwmLayerAttributeItemModel::indexFromItem(GwmLayerAttributeItem* item) const
{
    if (item)
    {
       return createIndex(mAttributeItemList.indexOf(item),0);
    }
    else
        return QModelIndex();
}

GwmLayerAttributeItem* GwmLayerAttributeItemModel::itemFromIndex(const QModelIndex &index) const{
    if(index.isValid()){
        return mAttributeItemList[index.row()];
//        GwmLayerAttributeItem* item = static_cast<GwmLayerAttributeItem*>(index.internalPointer());
//        if (item) return item;
    }
    return nullptr;
}

GwmLayerAttributeItem* GwmLayerAttributeItemModel::item(int i) const{
    return mAttributeItemList[i]? mAttributeItemList[i]:nullptr;
}

QList<QString> GwmLayerAttributeItemModel::toLayerAttributeList(){
    QList<QString> layerAttributeList;
    for(GwmLayerAttributeItem* item :mAttributeItemList){
        layerAttributeList.append(item->text());
    }
    return  layerAttributeList;
}

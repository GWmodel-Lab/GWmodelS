#include "gwmlayerattributeitem.h"

GwmLayerAttributeItem::GwmLayerAttributeItem(int index, const QString attributeName, const QString type,QObject *parent)
: QObject(parent),
  attributeIndex(index),
  attributeName(attributeName),
  dataType(type)
{

}

QString GwmLayerAttributeItem::text(){
    return attributeName;
}

QVariant GwmLayerAttributeItem::data(int col, int role){
    if (col == 0)
    {
//        return text();
        switch (role) {
            case Qt::DisplayRole:
                return col == 0 ? text() : QString();
            default:
                break;
        }
    }
    return QVariant();
}

int GwmLayerAttributeItem::index(){
    return attributeIndex;
}

bool GwmLayerAttributeItem::setData(int col, int role, QVariant value){
    if (col == 0)
    {
        switch (role) {
        case Qt::DisplayRole:
            attributeName = value.toString();
            return true;
        default:
            break;
        }
    }
    return true;
}

Qt::ItemFlags GwmLayerAttributeItem::flags()
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

GwmLayerAttributeItem* GwmLayerAttributeItem::clone(){
    return new GwmLayerAttributeItem(attributeIndex,attributeName,dataType);
}

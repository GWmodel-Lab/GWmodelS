#include "gwmlayerattributeitem.h"

GwmLayerAttributeItem::GwmLayerAttributeItem(int index, const QString mAttributeName,const QVariant::Type type,QObject *parent)
: QObject(parent),
  mAttributeIndex(index),
  mAttributeName(mAttributeName),
  mAttributeType(type)
{

}

QString GwmLayerAttributeItem::text(){
    return mAttributeName;
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
    return mAttributeIndex;
}

bool GwmLayerAttributeItem::setData(int col, int role, QVariant value){
    if (col == 0)
    {
        switch (role) {
        case Qt::DisplayRole:
            mAttributeName = value.toString().size() == 0 ? mAttributeName : value.toString();
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
    return new GwmLayerAttributeItem(mAttributeIndex,mAttributeName,mAttributeType);
}

QVariant::Type GwmLayerAttributeItem::type(){
    return mAttributeType;
}

int GwmLayerAttributeItem::attributeIndex() const
{
    return mAttributeIndex;
}

void GwmLayerAttributeItem::setAttributeIndex(int attributeIndex)
{
    mAttributeIndex = attributeIndex;
}

QString GwmLayerAttributeItem::attributeName() const
{
    return mAttributeName;
}

void GwmLayerAttributeItem::setAttributeName(const QString &attributeName)
{
    mAttributeName = attributeName;
}

QVariant::Type GwmLayerAttributeItem::attributeType() const
{
    return mAttributeType;
}

void GwmLayerAttributeItem::setAttributeType(const QVariant::Type &columnType)
{
    mAttributeType = columnType;
}

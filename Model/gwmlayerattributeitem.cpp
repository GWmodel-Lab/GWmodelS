#include "gwmlayerattributeitem.h"

GwmLayerAttributeItem::GwmLayerAttributeItem(int index, const QString mAttributeName,const QgsAttributeTableConfig attributeTableConfig,QObject *parent)
: QObject(parent),
  mAttributeIndex(index),
  mAttributeName(mAttributeName),
  mAttributeTableConfig(attributeTableConfig)
{
     QVector<QgsAttributeTableConfig::ColumnConfig> mColumns = mAttributeTableConfig.columns();
    for ( int i = mColumns.count() - 1; i >= 0; --i )
    {
      const QgsAttributeTableConfig::ColumnConfig &column = mColumns.at( i );
      if( column.name == mAttributeName){
          mColumnType = column.type;
      }
    }
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
            mAttributeName = value.toString();
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
    return new GwmLayerAttributeItem(mAttributeIndex,mAttributeName,mAttributeTableConfig);
}

QgsAttributeTableConfig::Type GwmLayerAttributeItem::type(){
    return mColumnType;
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

QgsAttributeTableConfig::Type GwmLayerAttributeItem::columnType() const
{
    return mColumnType;
}

void GwmLayerAttributeItem::setColumnType(const QgsAttributeTableConfig::Type &columnType)
{
    mColumnType = columnType;
}

QgsAttributeTableConfig GwmLayerAttributeItem::attributeTableConfig() const
{
    return mAttributeTableConfig;
}

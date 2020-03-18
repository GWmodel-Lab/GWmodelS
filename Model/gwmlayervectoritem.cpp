#include "gwmlayervectoritem.h"
#include <qgsrenderer.h>
#include <qgssinglesymbolrenderer.h>
#include <qgscategorizedsymbolrenderer.h>

GwmLayerVectorItem::SymbolType GwmLayerVectorItem::renderTypeToSymbolType(QString type)
{
    if ( type == QStringLiteral( "singleSymbol" ) ) return SymbolType::singleSymbol;
    else if ( type == QStringLiteral( "categorizedSymbol" ) ) return SymbolType::categorizedSymbol;
    else if ( type == QStringLiteral( "graduatedSymbol" ) ) return SymbolType::graduatedSymbol;
    else if ( type == QStringLiteral( "RuleRenderer" ) ) return SymbolType::RuleRenderer;
    else if ( type == QStringLiteral( "heatmapRenderer" ) ) return SymbolType::heatmapRenderer;
    else if ( type == QStringLiteral( "invertedPolygonRenderer" ) ) return SymbolType::invertedPolygonRenderer;
    else if ( type == QStringLiteral( "pointCluster" ) ) return SymbolType::pointCluster;
    else if ( type == QStringLiteral( "pointDisplacement" ) ) return SymbolType::pointDisplacement;
    else return SymbolType::nullSymbol;
}

GwmLayerVectorItem::GwmLayerVectorItem(GwmLayerItem* parent, QgsVectorLayer* vector)
    : GwmLayerItem(parent)
    , mLayer(vector)
{
    if (vector)
    {
        mSymbolType = GwmLayerVectorItem::renderTypeToSymbolType(vector->renderer()->type());
    }
}

GwmLayerVectorItem::~GwmLayerVectorItem()
{
    for (GwmLayerSymbolItem* item : mSymbolChildren)
    {
        delete item;
    }
    mSymbolChildren.clear();
}


QString GwmLayerVectorItem::text()
{
    return mLayer ? mLayer->name() : QStringLiteral("");
}

GwmLayerItem* GwmLayerVectorItem::child(int row)
{
    if (childCount() > 0)
        return mSymbolChildren.at(row);
    else return nullptr;
}

int GwmLayerVectorItem::childCount()
{
    switch (mSymbolType) {
    case SymbolType::singleSymbol:
    case SymbolType::nullSymbol:
        return 0;
    default:
        return mSymbolChildren.size();
    }
    return mSymbolType == SymbolType::singleSymbol ? 0 : mSymbolChildren.size();
}

int GwmLayerVectorItem::childNumber()
{
    return 0;
}

bool GwmLayerVectorItem::insertChildren(int position, int count)
{
    return false;
}

bool GwmLayerVectorItem::removeChildren(int position, int count)
{
    return false;
}

QVariant GwmLayerVectorItem::data(int col, int role)
{
    if (col == 0)
    {
        switch (role) {
        case Qt::DisplayRole:
            return col == 0 ? text() : QString();
        default:
            break;
        }
    }
    return QVariant();
}

GwmLayerVectorItem::SymbolType GwmLayerVectorItem::symbolType() const
{
    return mSymbolType;
}

void GwmLayerVectorItem::setSymbolType(const SymbolType &symbolType)
{
    mSymbolType = symbolType;
}

void GwmLayerVectorItem::setSymbolChildren(const QList<GwmLayerSymbolItem *> &symbolChildren)
{
    mSymbolChildren = symbolChildren;
}

QgsVectorLayer *GwmLayerVectorItem::layer() const
{
    return mLayer;
}

void GwmLayerVectorItem::getSymbolChildren()
{
    auto symbolItemList = mLayer->renderer()->legendSymbolItems();
    for (auto symbolItem : symbolItemList)
    {
        auto symbol = symbolItem.symbol();
        auto label = symbolItem.label();
        auto item = new GwmLayerSymbolItem(this, symbol, label);
        mSymbolChildren.append(item);
    }
}

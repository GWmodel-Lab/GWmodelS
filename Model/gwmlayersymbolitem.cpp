#include "gwmlayersymbolitem.h"

#include <QPixmap>
#include <QColor>
#include <QIcon>
#include <qgsrenderer.h>

#include "gwmlayervectoritem.h"

GwmLayerSymbolItem::GwmLayerSymbolItem(GwmLayerVectorItem* parent, QgsSymbol* symbol, QString label)
    : GwmLayerItem((GwmLayerItem*) parent)
    , mLabel(label)
    , mSymbol(symbol)
{
}

QString GwmLayerSymbolItem::text()
{
    return mLabel;
}

QVariant GwmLayerSymbolItem::data(int col, int role)
{
    if (col == 0 && mSymbol)
    {
        switch (role) {
        case Qt::DisplayRole:
            return col == 0 ? text() : QString();
        case Qt::DecorationRole:
        {
            QSize iconSize(12, 12);
            QPixmap pixmap(iconSize);
            pixmap.fill(Qt::GlobalColor::transparent);
            QPainter painter(&pixmap);
            mSymbol->drawPreviewIcon(&painter, iconSize);
            return QIcon(pixmap);
        }
        default:
            break;
        }
    }
    return QVariant();
}

GwmLayerItem* GwmLayerSymbolItem::child(int row)
{
    return nullptr;
}

int GwmLayerSymbolItem::childCount()
{
    return 0;
}

int GwmLayerSymbolItem::childNumber()
{
    GwmLayerVectorItem* parent = (GwmLayerVectorItem*) mParentItem;
    return parent->symbolChildren().indexOf(this);
}

bool GwmLayerSymbolItem::insertChildren(int position, int count)
{
    return false;
}

bool GwmLayerSymbolItem::removeChildren(int position, int count)
{
    return false;
}

#include "gwmlayersymbolitem.h"

#include <QPixmap>
#include <QColor>
#include <QIcon>
#include <qgsrenderer.h>

#include "gwmlayervectoritem.h"

GwmLayerSymbolItem::GwmLayerSymbolItem(GwmLayerVectorItem* parent, QIcon symbol, QString label)
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
//    qDebug() << mSymbol->color();
    if (col == 0)
    {
        switch (role) {
        case Qt::DisplayRole:
            return col == 0 ? text() : QString();
        case Qt::DecorationRole:
        {
            return mSymbol;
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

bool GwmLayerSymbolItem::insertChildren(int position, QList<GwmLayerItem *> items)
{
    return false;
}

bool GwmLayerSymbolItem::appendChildren(QList<GwmLayerItem *> items)
{
    return false;
}

QList<GwmLayerItem*> GwmLayerSymbolItem::takeChildren(int position, int count)
{
    return QList<GwmLayerItem*>();
}

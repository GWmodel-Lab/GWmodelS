#include "gwmlayergroupitem.h"
#include "gwmlayergwritem.h"

GwmLayerGroupItem::GwmLayerGroupItem(GwmLayerItem* parent, QgsVectorLayer* vector)
    : GwmLayerItem(parent)
    , mOriginChild(nullptr)
{
    if (vector)
    {
        mOriginChild = new GwmLayerVectorItem(this, vector);
    }
}

GwmLayerGroupItem::~GwmLayerGroupItem()
{
    delete mOriginChild;
    for (GwmLayerVectorItem* item : mAnalyseChildren)
    {
        delete item;
    }
    mAnalyseChildren.clear();
}

QVariant GwmLayerGroupItem::data(int col, int role)
{
    if (col == 0)
    {
        switch (role) {
        case Qt::DisplayRole:
            return QVariant(text());
        default:
            break;
        }
    }
    return QVariant();
}

QString GwmLayerGroupItem::text()
{
    return mOriginChild ? mOriginChild->layer()->name() : QStringLiteral("");
}

GwmLayerItem* GwmLayerGroupItem::child(int row)
{
    if (row == 0) return mOriginChild;
    else return mAnalyseChildren.at(row - 1);
}

int GwmLayerGroupItem::childCount()
{
    return 1 + mAnalyseChildren.size();
}

int GwmLayerGroupItem::childNumber()
{
    return mParentItem->children().indexOf(this);
}

bool GwmLayerGroupItem::insertChildren(int position, int count)
{
    int row = position - 1;
    if (row < 0 || row > mAnalyseChildren.size())
        return false;

    for (int row = 0; row < count; row++)
    {
        GwmLayerVectorItem* item = new GwmLayerVectorItem(this);
        mAnalyseChildren.insert(row, item);
    }

    return true;
}

bool GwmLayerGroupItem::removeChildren(int position, int count)
{
    int row = position - 1;
    if (row < 0 || row + count > mAnalyseChildren.size())
        return false;

    for (int r = 0; r < count; r++)
    {
        delete mAnalyseChildren.takeAt(row);
    }

    return true;
}

void GwmLayerGroupItem::setOriginChild(GwmLayerVectorItem *originChild)
{
    mOriginChild = originChild;
}

void GwmLayerGroupItem::setAnalyseChildren(const QList<GwmLayerVectorItem *> &analyseChildren)
{
    mAnalyseChildren = analyseChildren;
}

GwmLayerVectorItem* GwmLayerGroupItem::originChild()
{
    return mOriginChild;
}

QList<GwmLayerVectorItem*>& GwmLayerGroupItem::analyseChildren()
{
    return mAnalyseChildren;
}

#include "gwmlayeritem.h"
#include "gwmlayergroupitem.h"

GwmLayerItem::GwmLayerItem(GwmLayerItem* parent)
    : mName(QStringLiteral("root"))
    , mParentItem(parent)
    , mChildren()
{
}

GwmLayerItem::~GwmLayerItem()
{
    while (mChildren.size() > 0) {
        delete mChildren.takeAt(0);
    }
}

QString GwmLayerItem::text()
{
    return QStringLiteral("");
}

GwmLayerItem* GwmLayerItem::parentItem()
{
    return mParentItem;
}

GwmLayerItem* GwmLayerItem::child(int row)
{
    if (mChildren.size() > 0)
        return mChildren.at(row);
    else return nullptr;
}

int GwmLayerItem::childCount()
{
    return mChildren.size();
}

int GwmLayerItem::childNumber()
{
    return 0;
}

QVariant GwmLayerItem::data(int col, int role)
{
    return QVariant();
}

QList<GwmLayerGroupItem*>& GwmLayerItem::children()
{
    return mChildren;
}

void GwmLayerItem::setParentItem(GwmLayerItem *parentItem)
{
    mParentItem = parentItem;
}

void GwmLayerItem::setChildren(const QList<GwmLayerGroupItem *> &children)
{
    mChildren = children;
}

bool GwmLayerItem::insertChildren(int position, int count)
{
    if (position < 0 || position > mChildren.size())
        return false;

    for (int row = 0; row < count; row++)
    {
        GwmLayerGroupItem* item = new GwmLayerGroupItem(this);
        mChildren.insert(position, item);
    }

    return true;
}

bool GwmLayerItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > mChildren.size())
        return false;

    for (int r = 0; r < count; r++)
    {
        delete mChildren.takeAt(position);
    }

    return true;
}

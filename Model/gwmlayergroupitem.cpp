#include "gwmlayergroupitem.h"
#include "gwmlayergwritem.h"

GwmLayerGroupItem::GwmLayerGroupItem(GwmLayerItem* parent, QgsVectorLayer* vector)
    : GwmLayerItem(parent)
    , mOriginChild(nullptr)
{
    if (vector)
    {
        mOriginChild = new GwmLayerOriginItem(this, vector);
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
        case Qt::CheckStateRole:
            return mCheckState;
        case Qt::DecorationRole:
            return mOriginChild->data(col, role);
        default:
            break;
        }
    }
    return QVariant();
}

Qt::ItemFlags GwmLayerGroupItem::flags()
{
    return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | GwmLayerItem::flags();
}

QString GwmLayerGroupItem::text()
{
    return mOriginChild ? mOriginChild->layer()->name() : QStringLiteral("");
}

GwmLayerItem* GwmLayerGroupItem::child(int row)
{
    if (row == 0) return mOriginChild;
    else if (row > 0 && row < childCount())
        return mAnalyseChildren.at(row - 1);
    else return nullptr;
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

bool GwmLayerGroupItem::insertChildren(int position, QList<GwmLayerItem *> items)
{
    int count = items.size();
    if (position < 0 || position > mAnalyseChildren.size())
        return false;
    else if (position == mAnalyseChildren.size())
    {
        return appendChildren(items);
    }
    else
    {
        for (int row = count -1; row >= 0; row--)
        {
            GwmLayerItem* item = items.at(row);
            if (item->itemType() == GwmLayerItemType::Group)
                mAnalyseChildren.insert(position, (GwmLayerVectorItem*)item);
            else return true;
        }
    }
    return true;
}

bool GwmLayerGroupItem::appendChildren(QList<GwmLayerItem *> items)
{
    for (int row = 0; row < items.size(); row ++)
    {
        GwmLayerItem* item = items.at(row);
        switch (item->itemType())
        {
        case GwmLayerItemType::GWR:
        case GwmLayerItemType::ScalableGWR:
        case GwmLayerItemType::GGWR:
        case GwmLayerItemType::MultiscaleGWR:
            mAnalyseChildren.append((GwmLayerVectorItem*)item);
            return true;
        default:
            return false;
        }
    }
    return true;
}

QList<GwmLayerItem *> GwmLayerGroupItem::takeChildren(int position, int count)
{
    position = position - 1;
    QList<GwmLayerItem*> takenItems;
    if (position < 0 || position + count > mAnalyseChildren.size())
        return takenItems;

    for (int r = 0; r < count; r++)
    {
        takenItems += mAnalyseChildren.takeAt(position);
    }

    return takenItems;
}

bool GwmLayerGroupItem::moveChildren(int position, int count, int destination)
{
    position = position - 1;
    QList<GwmLayerItem*> removedChildren = takeChildren(position, count);
    if (removedChildren.size() > 0)
        return insertChildren(destination, removedChildren);
    else return false;
}

#include "gwmlayeritem.h"
#include "gwmlayergroupitem.h"

GwmEnumValueNameMapper<GwmLayerItem::GwmLayerItemType> GwmLayerItem::LayerItemTypeNameMapper = {
    std::make_pair(GwmLayerItem::GwmLayerItemType::Base, "Base"),
    std::make_pair(GwmLayerItem::GwmLayerItemType::Group, "Group"),
    std::make_pair(GwmLayerItem::GwmLayerItemType::Vector, "Vector"),
    std::make_pair(GwmLayerItem::GwmLayerItemType::Origin, "Origin"),
    std::make_pair(GwmLayerItem::GwmLayerItemType::GWR, "GWR"),
    std::make_pair(GwmLayerItem::GwmLayerItemType::Symbol, "Symbol"),
    std::make_pair(GwmLayerItem::GwmLayerItemType::ScalableGWR, "ScalableGWR"),
    std::make_pair(GwmLayerItem::GwmLayerItemType::GeneralizedGWR, "GeneralizedGWR"),
    std::make_pair(GwmLayerItem::GwmLayerItemType::MultiscaleGWR, "MultiscaleGWR"),
    std::make_pair(GwmLayerItem::GwmLayerItemType::CollinearityGWR, "CollinearityGWR"),
    std::make_pair(GwmLayerItem::GwmLayerItemType::GWSS, "GWSS"),
    std::make_pair(GwmLayerItem::GwmLayerItemType::GWPCA, "GWPCA")
};

GwmLayerItem::GwmLayerItem(GwmLayerItem* parent)
    : mParentItem(parent)
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

GwmLayerItem* GwmLayerItem::parentItem() const
{
    return mParentItem;
}

GwmLayerItem* GwmLayerItem::child(int row)
{
    if (row >= 0 && row < mChildren.size())
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

Qt::ItemFlags GwmLayerItem::flags()
{
    return Qt::ItemIsEnabled;
}

bool GwmLayerItem::setData(int col, int role, QVariant value)
{
    if (col == 0)
    {
        switch (role) {
        case Qt::CheckStateRole:
            mCheckState = Qt::CheckState(value.toInt());
            break;
        default:
            return false;
        }
    }
    return true;
}

void GwmLayerItem::setParentItem(GwmLayerItem *parentItem)
{
    mParentItem = parentItem;
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

bool GwmLayerItem::insertChildren(int position, QList<GwmLayerItem *> items)
{
    int count = items.size();
    if (position < 0 || position > mChildren.size())
        return false;
    else if (position == mChildren.size())
    {
        return appendChildren(items);
    }
    else
    {
        for (int row = count -1; row >= 0; row--)
        {
            GwmLayerItem* item = items.at(row);
            if (item->itemType() == GwmLayerItemType::Group)
                mChildren.insert(position, (GwmLayerGroupItem*)item);
            else return false;
        }
    }
    return true;
}

bool GwmLayerItem::appendChildren(QList<GwmLayerItem *> items)
{
    for (int row = 0; row < items.size(); row ++)
    {
        GwmLayerItem* item = items.at(row);
        if (item->itemType() == GwmLayerItemType::Group)
            mChildren.append((GwmLayerGroupItem*)item);
        else return false;
    }
    return true;
}

QList<GwmLayerItem*> GwmLayerItem::takeChildren(int position, int count)
{
    QList<GwmLayerItem*> takenItems;
    if (position < 0 || position + count > mChildren.size())
        return takenItems;

    for (int r = 0; r < count; r++)
    {
        takenItems += mChildren.takeAt(position);
    }

    return takenItems;
}

bool GwmLayerItem::moveChildren(int position, int count, int destination)
{
    QList<GwmLayerItem*> removedChildren = takeChildren(position, count);
    if (removedChildren.size() > 0)
        return insertChildren(destination, removedChildren);
    else return false;
}

bool GwmLayerItem::readXml(QDomNode &node)
{
    return false;
}

bool GwmLayerItem::writeXml(QDomNode &node, QDomDocument &doc)
{
    QDomElement parent = node.toElement();
    for (auto item : mChildren)
    {
        QDomElement group = doc.createElement("group");
        group.setAttribute("name", text());
        parent.appendChild(group);

        item->writeXml(group, doc);
    }
    return true;
}

Qt::CheckState GwmLayerItem::checkState() const
{
    return mCheckState;
}

void GwmLayerItem::setCheckState(const Qt::CheckState &checkState)
{
    mCheckState = checkState;
}

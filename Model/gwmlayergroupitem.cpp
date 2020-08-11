#include "gwmlayergroupitem.h"
#include "gwmlayerbasicgwritem.h"
#include "gwmlayergwssitem.h"
#include "gwmlayerscalablegwritem.h"
#include "gwmlayermultiscalegwritem.h"
#include "gwmlayercollinearitygwritem.h"
#include "gwmlayerggwritem.h"
#include "gwmlayergwpcaitem.h"
#include "gwmlayergtwritem.h"

#include <qmessagebox.h>

#include "gwmapp.h"

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
    if (mOriginChild)
    {
        delete mOriginChild;
    }
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
            return col == 0 ? text() : QString();
        case Qt::CheckStateRole:
            return mCheckState;
        case Qt::DecorationRole:
            return mOriginChild->data(col, role);
        case Qt::EditRole:
            return col == 0 ? text() : QString();
        default:
            break;
        }
    }
    return QVariant();
}

bool GwmLayerGroupItem::setData(int col, int role, QVariant value)
{
    if (col == 0)
    {
        switch (role) {
        case Qt::CheckStateRole:
            mCheckState = Qt::CheckState(value.toInt());
            break;
        case Qt::EditRole:
            if (mOriginChild) mOriginChild->layer()->setName(value.toString());
            break;
        default:
            return false;
        }
    }
    return true;
}

Qt::ItemFlags GwmLayerGroupItem::flags()
{
    return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEditable | GwmLayerItem::flags();
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
        mAnalyseChildren.takeAt(row);
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
        case GwmLayerItemType::GeneralizedGWR:
        case GwmLayerItemType::MultiscaleGWR:
        case GwmLayerItemType::GWSS:
        case GwmLayerItemType::CollinearityGWR:
        case GwmLayerItemType::GWPCA:
        case GwmLayerItemType::GTWR:
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

bool GwmLayerGroupItem::readXml(QDomNode &node)
{
    QDomElement group = node.toElement();

    QDomElement origin = node.firstChildElement("origin");
    if (origin.isNull())
        return false;

    GwmLayerOriginItem* originItem = new GwmLayerOriginItem(this);
    if (originItem->readXml(origin))
    {
        mOriginChild = originItem;
    }
    else
    {
        delete mOriginChild;
        mOriginChild = nullptr;
        return false;
    }

    QDomElement analyseList = node.firstChildElement("analyseList");
    if (analyseList.isNull())
        return false;

    QDomElement analyseNode = analyseList.firstChildElement("analyse");
    while (!analyseNode.isNull())
    {
        if (analyseNode.hasAttribute("type"))
        {
            QString analyseTypeName = analyseNode.attribute("type");
            GwmLayerItemType type = LayerItemTypeNameMapper.value(analyseTypeName);
            GwmLayerVectorItem* analyseItem;
            switch (type) {
            case GwmLayerItemType::GWR:
                analyseItem = new GwmLayerBasicGWRItem(this);
                break;
            case GwmLayerItemType::ScalableGWR:
                analyseItem = new GwmLayerScalableGWRItem(this);
                break;
            case GwmLayerItemType::MultiscaleGWR:
                analyseItem = new GwmLayerMultiscaleGWRItem(this);
                break;
            case GwmLayerItemType::GeneralizedGWR:
                analyseItem = new GwmLayerGGWRItem(this);
                break;
            case GwmLayerItemType::CollinearityGWR:
                analyseItem = new GwmLayerCollinearityGWRItem(this);
                break;
            case GwmLayerItemType::GTWR:
                analyseItem = new GwmLayerGTWRItem(this);
                break;
            case GwmLayerItemType::GWSS:
                analyseItem = new GwmLayerGWSSItem(this);
                break;
            case GwmLayerItemType::GWPCA:
                analyseItem = new GwmLayerGWPCAItem(this);
                break;
            default:
                analyseItem = new GwmLayerVectorItem(this);
                break;
            }
            if (analyseItem->readXml(analyseNode))
            {
                mAnalyseChildren.append(analyseItem);
            }
            else
            {
                delete analyseItem;
            }
        }
        analyseNode = analyseNode.nextSiblingElement("analyse");
    }

    return true;
}

bool GwmLayerGroupItem::writeXml(QDomNode &node, QDomDocument &doc)
{
    QDomElement group = node.toElement();

    QDomElement origin = doc.createElement("origin");
    origin.setAttribute("name", text());
    group.appendChild(origin);
    mOriginChild->writeXml(origin, doc);

    if (mAnalyseChildren.size() > 0)
    {
        QDomElement analyseList = doc.createElement("analyseList");
        for (auto analyseChild : mAnalyseChildren)
        {
            QDomElement analyse = doc.createElement("analyse");
            analyse.setAttribute("name", analyseChild->text());
            if (analyseChild->writeXml(analyse, doc))
                analyseList.appendChild(analyse);
            else
            {
                QMessageBox::warning(GwmApp::Instance(), tr("Save Analyse Layer Error!"), tr(""));
            }
        }
        group.appendChild(analyseList);
    }

    return true;
}

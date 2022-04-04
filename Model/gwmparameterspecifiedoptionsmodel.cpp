#include "gwmparameterspecifiedoptionsmodel.h"

GwmParameterSpecifiedOptionsModel::GwmParameterSpecifiedOptionsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    GwmParameterSpecifiedOption intercept;
    intercept.attributeName = "Intercept";
    intercept.attributeIndex = -1;
    mItems.append(intercept);
}

QVariant GwmParameterSpecifiedOptionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

bool GwmParameterSpecifiedOptionsModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role)) {
        // FIXME: Implement me!
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}

int GwmParameterSpecifiedOptionsModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return mItems.size();
}

QVariant GwmParameterSpecifiedOptionsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int i = index.row();
    switch (role) {
    case Qt::ItemDataRole::DisplayRole:
        return mItems[i].attributeName;
    default:
        break;
    }
    return QVariant();
}

Qt::ItemFlags GwmParameterSpecifiedOptionsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable; // FIXME: Implement me!
}

GwmParameterSpecifiedOption* GwmParameterSpecifiedOptionsModel::item(const QModelIndex &index)
{
    if (!index.isValid())
        return nullptr;

    return &(mItems[index.row()]);
}

GwmParameterSpecifiedOption *GwmParameterSpecifiedOptionsModel::item(const int row)
{
    if (row >= 0 && row < mItems.size())
        return &(mItems[row]);
    else return nullptr;
}

void GwmParameterSpecifiedOptionsModel::syncWithAttributes(const GwmVariableItemModel *attributeModel)
{
    // 保存截距项
    if (mItems.size() > 1)
    {
        beginRemoveRows(QModelIndex(), 1, rowCount() - 1);
        mItems.erase(++mItems.begin(), mItems.end());
        endRemoveRows();
    }

    // 添加新项
    beginInsertRows(QModelIndex(), 1, attributeModel->rowCount());
    int nAttribute = attributeModel->rowCount();
    for (int i = 0; i < nAttribute; i++)
    {
        GwmVariable item = attributeModel->item(i);
        GwmParameterSpecifiedOption option;
        option.attributeName = item.name;
        option.attributeIndex = item.index;
        mItems.append(option);
    }
    endInsertRows();
}

void GwmParameterSpecifiedOptionsModel::mItemUnshift(){
    //封装出队列的函数
    mItems.takeFirst();
}

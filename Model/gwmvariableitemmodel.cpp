#include "gwmvariableitemmodel.h"

GwmVariableItemModel::GwmVariableItemModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant GwmVariableItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical || section > 0)
        return QVariant();
    else return mHorizontalHeaderData[0];
}

bool GwmVariableItemModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation == Qt::Vertical || section > 0)
        return false;
    mHorizontalHeaderData[0] = value.toString();
    return true;
}

int GwmVariableItemModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return mItems.size();
}

QVariant GwmVariableItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    return (row >= 0 && row < rowCount()) ? mItems[row].name : QVariant();
}

bool GwmVariableItemModel::insert(int row, GwmVariable variable)
{
    beginInsertRows(QModelIndex(), row, row);
    mItems.insert(row, variable);
    endInsertRows();
}

bool GwmVariableItemModel::insert(int row, QList<GwmVariable> variables)
{
    beginInsertRows(QModelIndex(), row, row + variables.size() - 1);
    for (auto i = variables.rbegin(); i != variables.rend(); i++)
    {
        mItems.insert(mItems.begin() + row, *i);
    }
    endInsertRows();
}

bool GwmVariableItemModel::remove(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    mItems.erase(mItems.begin() + row);
    endRemoveRows();
}

bool GwmVariableItemModel::remove(int row, int count)
{
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    mItems.erase(mItems.begin() + row, mItems.begin() + row + count);
    endRemoveRows();
}

bool GwmVariableItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    mItems.insert(row, count, GwmVariable());
    endInsertRows();
}

bool GwmVariableItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    mItems.erase(mItems.begin() + row, mItems.begin() + row + count);
    endRemoveRows();
}

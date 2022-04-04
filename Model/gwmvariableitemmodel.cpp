#include "gwmvariableitemmodel.h"

GwmVariableItemModel::GwmVariableItemModel(QObject *parent) : QAbstractListModel(parent)
{
}

GwmVariableItemModel::GwmVariableItemModel(QgsVectorLayer *layer, QObject *parent) : QAbstractListModel(parent)
{
    QgsFields fields = layer->fields();
    for (int f = 0; f < fields.size(); f++)
    {
        GwmVariable variable;
        variable.index = f;
        variable.name = fields[f].name();
        variable.type = fields[f].type();
        variable.isNumeric = fields[f].isNumeric();
        mItems.append(variable);
    }
}
//创建一个新的构造函数，用于将两个自变量模型合成一个
GwmVariableItemModel::GwmVariableItemModel(GwmVariableItemModel* indepVarModelX, GwmVariableItemModel* indepVarModelY, QObject *parent) : QAbstractListModel(parent)
{
    int count = 0;
    for( int i = 0; i < indepVarModelX->rowCount(); i++){
        for( int j = 0; j < indepVarModelY->rowCount(); j++){
            if(indepVarModelX->item(i).name == indepVarModelY->item(j).name) continue;
            GwmVariable variable;
            variable.index = count;
            variable.name = indepVarModelX->item(i).name + "*" + indepVarModelY->item(j).name;
            variable.type = indepVarModelX->item(i).type;
            variable.isNumeric = indepVarModelX->item(i).isNumeric;
            mItems.append(variable);
            count++;
        }
    }
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
    switch (role) {
    case Qt::ItemDataRole::DisplayRole:
        return (row >= 0 && row < rowCount()) ? mItems[row].name : QVariant();
    default:
        return QVariant();
    }
}

bool GwmVariableItemModel::insert(int row, GwmVariable variable)
{
    beginInsertRows(QModelIndex(), row, row);
    mItems.insert(row, variable);
    endInsertRows();
    return true;
}

bool GwmVariableItemModel::insert(int row, QList<GwmVariable> variables)
{
    beginInsertRows(QModelIndex(), row, row + variables.size() - 1);
    for (auto i = variables.rbegin(); i != variables.rend(); i++)
    {
        mItems.insert(mItems.begin() + row, *i);
    }
    endInsertRows();
    return true;
}

bool GwmVariableItemModel::append(GwmVariable variable)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mItems.append(variable);
    endInsertRows();
    return true;
}

bool GwmVariableItemModel::append(QList<GwmVariable> variables)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + variables.size() - 1);
    mItems.append(variables);
    endInsertRows();
    return true;
}

bool GwmVariableItemModel::remove(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    mItems.erase(mItems.begin() + row);
    endRemoveRows();
    return true;
}

bool GwmVariableItemModel::remove(int row, int count)
{
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    mItems.erase(mItems.begin() + row, mItems.begin() + row + count);
    endRemoveRows();
    return true;
}

bool GwmVariableItemModel::clear(){
    if (rowCount() <= 0) return true;
    beginRemoveRows(QModelIndex(),0,rowCount()-1);
    mItems.clear();
    endRemoveRows();
    return true;
}

GwmVariable GwmVariableItemModel::itemFromIndex(const QModelIndex &index) const{
    if(index.isValid()){
        return mItems[index.row()];
    }
    return *(new GwmVariable);
}

QList<GwmVariable> GwmVariableItemModel::attributeItemList() const
{
    return mItems;
}

GwmVariable GwmVariableItemModel::item(int i) const
{
    return mItems[i];
}

bool GwmVariableItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    for (int i = 0; i < count; i++)
    {
        mItems.insert(row, GwmVariable());
    }
    endInsertRows();
    return true;
}

bool GwmVariableItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    mItems.erase(mItems.begin() + row, mItems.begin() + row + count);
    endRemoveRows();
    return true;
}

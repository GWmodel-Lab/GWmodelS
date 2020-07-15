#include "gwmlayoutbatchfieldlistmodel.h"

QgsFieldCheckable::QgsFieldCheckable(const QgsField &field)
    : QgsField(field)
{
    mChecked = false;
}

GwmLayoutBatchFieldListModel::GwmLayoutBatchFieldListModel(QgsVectorLayer *layer, QObject *parent)
    : QAbstractListModel(parent)
{
    for (auto field : layer->fields())
    {
        mFieldList.append(new QgsFieldCheckable(field));
    }
}

GwmLayoutBatchFieldListModel::~GwmLayoutBatchFieldListModel()
{
    for (auto field : mFieldList)
    {
        delete field;
    }
}

int GwmLayoutBatchFieldListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return mFieldList.size();
}

QVariant GwmLayoutBatchFieldListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    if (row > rowCount())
        return QVariant();

    switch (role)
    {
    case Qt::ItemDataRole::DisplayRole:
        return mFieldList[row]->name();
    case Qt::ItemDataRole::CheckStateRole:
        return mFieldList[row]->checked() ? Qt::Checked : Qt::Unchecked;
    default:
        return QVariant();
    }
}

bool GwmLayoutBatchFieldListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        int row = index.row();
        switch (role)
        {
        case Qt::ItemDataRole::CheckStateRole:
            mFieldList[row]->setChecked(value.toInt() == Qt::Checked);
            break;
        default:
            break;
        }

        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags GwmLayoutBatchFieldListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}

QgsFieldCheckable *GwmLayoutBatchFieldListModel::itemFromIndex(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= mFieldList.size())
        return nullptr;

    return mFieldList[index.row()];
}

int GwmLayoutBatchFieldListModel::checkedIndex(const QgsFieldCheckable *field)
{
    int c = -1;
    for (int i = 0; i < rowCount(); i++)
    {
        const QgsFieldCheckable* item = mFieldList[i];
        if (item->checked())
        {
            c++;
            if (item->name() == field->name())
                return c;
        }
    }
    return -1;
}





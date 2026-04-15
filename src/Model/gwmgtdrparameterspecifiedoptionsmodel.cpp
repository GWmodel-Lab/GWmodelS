// 新建文件：src/Model/gwmgtdrparameterspecifiedoptionsmodel.cpp
#include "gwmgtdrparameterspecifiedoptionsmodel.h"

GwmGTDRParameterSpecifiedOptionsModel::GwmGTDRParameterSpecifiedOptionsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant GwmGTDRParameterSpecifiedOptionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case 0: return tr("Variable Name");
        case 1: return tr("Initial Bandwidth");
        case 2: return tr("Kernel Function");
        default: return QVariant();
        }
    }
    return QVariant();
}

int GwmGTDRParameterSpecifiedOptionsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return mItems.size();
}

QVariant GwmGTDRParameterSpecifiedOptionsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= mItems.size())
        return QVariant();

    const GwmGTDRParameterSpecifiedOption& item = mItems[index.row()];

    switch (role)
    {
    case Qt::DisplayRole:
        return item.attributeName;  // 只返回变量名
    default:
        return QVariant();
    }
}

bool GwmGTDRParameterSpecifiedOptionsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= mItems.size() || role != Qt::EditRole)
        return false;

    GwmGTDRParameterSpecifiedOption& item = mItems[index.row()];

    switch (index.column())
    {
    case 0:  // 变量名不可编辑
        return false;
    case 1:  // 初始带宽值
        item.initialBandwidthSize = value.toDouble();
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        return true;
    case 2:  // 核函数类型
        item.kernel = static_cast<gwm::BandwidthWeight::KernelFunctionType>(value.toInt());
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        return true;
    default:
        return false;
    }
}

Qt::ItemFlags GwmGTDRParameterSpecifiedOptionsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    // 变量名不可编辑，其他列可编辑
    if (index.column() != 0)
        flags |= Qt::ItemIsEditable;

    return flags;
}

GwmGTDRParameterSpecifiedOption* GwmGTDRParameterSpecifiedOptionsModel::item(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= mItems.size())
        return nullptr;
    return &(mItems[index.row()]);
}

GwmGTDRParameterSpecifiedOption* GwmGTDRParameterSpecifiedOptionsModel::item(const int row)
{
    if (row >= 0 && row < mItems.size())
        return &(mItems[row]);
    return nullptr;
}

void GwmGTDRParameterSpecifiedOptionsModel::syncWithAttributes(const GwmVariableItemModel* attributeModel)
{
    if (!attributeModel)
    {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        mItems.clear();
        endRemoveRows();
        return;
    }

    // 清空现有项（GTDR 不需要 Intercept）
    beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
    mItems.clear();
    endRemoveRows();

    // 添加新项
    int nAttribute = attributeModel->rowCount();
    if (nAttribute > 0)
    {
        beginInsertRows(QModelIndex(), 0, nAttribute - 1);
        for (int i = 0; i < nAttribute; i++)
        {
            GwmVariable var = attributeModel->item(i);
            GwmGTDRParameterSpecifiedOption option;
            option.attributeName = var.name;
            option.attributeIndex = var.index;
            // 使用默认值（可以在外部设置）
            option.initialBandwidthSize = 100.0;
            option.kernel = gwm::BandwidthWeight::KernelFunctionType::Gaussian;
            mItems.append(option);
        }
        endInsertRows();
    }
}

#include "gwmpropertygwcorrelationsparameterspecifieditemmodel.h"

GwmPropertyGWCorrelationsParameterSpecifiedItemModel::GwmPropertyGWCorrelationsParameterSpecifiedItemModel(GwmLayerGWCorrelationItem* layerItem,QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<GwmVariable> mVariables = layerItem->variables();
    QList<GwmVariable> mVariablesY = layerItem->variablesY();
    int nVarY = mVariablesY.size();
    for (int i = 0; i < mVariables.size()*mVariablesY.size(); ++i)
    {
        GwmParameterSpecifiedOption* option = new GwmParameterSpecifiedOption ();
        GwmBandwidthWeight bw = layerItem->bandwidthWeights()[i];
        option->attributeName = mVariables[i/nVarY].name + "." + mVariablesY[(i+nVarY)%nVarY].name;
        option->bandwidthSize = bw.bandwidth();
        option->adaptive = bw.adaptive();
        option->kernel = bw.kernel();
        option->bandwidthSeledType = layerItem->bandwidthInitilize()[i];
        option->approach = layerItem->bandwidthSelectionApproach()[i];
        option->distanceType = layerItem->distaneTypes()[i];
        mItems.append(option);
    }
}

GwmPropertyGWCorrelationsParameterSpecifiedItemModel::~GwmPropertyGWCorrelationsParameterSpecifiedItemModel()
{
    for (GwmParameterSpecifiedOption* option : mItems)
    {
        delete option;
    }
}

QVariant GwmPropertyGWCorrelationsParameterSpecifiedItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role) {
    case Qt::ItemDataRole::DisplayRole:
    {
        QList<QString> dataList = { "", "Value" };
        return dataList[section];
    }
    default:
        return QVariant();
    }
}

QModelIndex GwmPropertyGWCorrelationsParameterSpecifiedItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() > 1)
        return QModelIndex();

    if (parent.isValid())
        return createIndex(row, column, mItems[parent.row()]);
    else return createIndex(row, column);
}

QModelIndex GwmPropertyGWCorrelationsParameterSpecifiedItemModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    if (index.internalPointer())
        return createIndex(mItems.indexOf((GwmParameterSpecifiedOption*)index.internalPointer()), 0);
    else return QModelIndex();
}

int GwmPropertyGWCorrelationsParameterSpecifiedItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return parent.internalPointer() ? 0 : 6;
    else return mItems.size();
}

int GwmPropertyGWCorrelationsParameterSpecifiedItemModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant GwmPropertyGWCorrelationsParameterSpecifiedItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.internalPointer())
    {
        GwmParameterSpecifiedOption* option = (GwmParameterSpecifiedOption*) index.internalPointer();
        QList<QList<QVariant> > dataArray;
        switch (role) {
        case Qt::ItemDataRole::DisplayRole:
            dataArray = {
                { "Size", option->bandwidthSize },
                { "Type", GwmBandwidthWeight::BandwidthTypeNameMapper[option->adaptive] },
                { "Kernel", GwmBandwidthWeight::KernelFunctionTypeNameMapper[option->kernel] },
                { "Selected", GwmMultiscaleGWRAlgorithm::BandwidthInitilizeTypeNameMapper[option->bandwidthSeledType] },
                { "Approach", GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionTypeNameMapper[option->approach] },
                { "Distance", GwmDistance::TypeNameMapper[option->distanceType] }
            };
            return dataArray[index.row()][index.column()];
        default:
            return QVariant();
        }
    }
    else
    {
        GwmParameterSpecifiedOption* option = mItems[index.row()];
        QList<QVariant> dataArray;
        switch (role) {
        case Qt::ItemDataRole::DisplayRole:
            dataArray = {
                option->attributeName,
                QString("%1 %2").arg(GwmBandwidthWeight::BandwidthTypeNameMapper[option->adaptive]).arg(option->bandwidthSize)
            };
            return dataArray[index.column()];
        default:
            return QVariant();
        }
    }
}

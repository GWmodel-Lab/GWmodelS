#include "gwmpropertymultiscaleparameterspecifieditemmodel.h"

QMap<GwmMultiscaleGWRTaskThread::BandwidthInitilizeType, QString> GwmPropertyMultiscaleParameterSpecifiedItemModel::BandwidthSeledTypeName = {
    std::make_pair(GwmMultiscaleGWRTaskThread::BandwidthInitilizeType::Null, "Not initizlied"),
    std::make_pair(GwmMultiscaleGWRTaskThread::BandwidthInitilizeType::Initial, "Initizlied"),
    std::make_pair(GwmMultiscaleGWRTaskThread::BandwidthInitilizeType::Specified, "Specified")
};

QMap<GwmGWRTaskThread::BandwidthSelectionApproach, QString> GwmPropertyMultiscaleParameterSpecifiedItemModel::BandwidthSelectionApproachName = {
    std::make_pair(GwmGWRTaskThread::BandwidthSelectionApproach::CV, "CV"),
    std::make_pair(GwmGWRTaskThread::BandwidthSelectionApproach::AIC, "AIC")
};

QMap<GwmGWRTaskThread::BandwidthType, QString> GwmPropertyMultiscaleParameterSpecifiedItemModel::BandwidthTypeName = {
    std::make_pair(GwmGWRTaskThread::BandwidthType::Fixed, "Fixed"),
    std::make_pair(GwmGWRTaskThread::BandwidthType::Adaptive, "Adaptive")
};

QMap<GwmGWRTaskThread::KernelFunction, QString> GwmPropertyMultiscaleParameterSpecifiedItemModel::KernelFunctionName = {
    std::make_pair(GwmGWRTaskThread::KernelFunction::Gaussian, "Gaussian"),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Exponential, "Exponential"),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Boxcar, "Boxcar"),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Bisquare, "Bisquare"),
    std::make_pair(GwmGWRTaskThread::KernelFunction::Tricube, "Tricube")
};

QMap<GwmGWRTaskThread::DistanceSourceType, QString> GwmPropertyMultiscaleParameterSpecifiedItemModel::DistanceTypeName = {
    std::make_pair(GwmGWRTaskThread::DistanceSourceType::CRS, "CRS"),
    std::make_pair(GwmGWRTaskThread::DistanceSourceType::Minkowski, "Minkowski"),
    std::make_pair(GwmGWRTaskThread::DistanceSourceType::DMatFile, "Distance Matrix")
};

GwmPropertyMultiscaleParameterSpecifiedItemModel::GwmPropertyMultiscaleParameterSpecifiedItemModel(GwmLayerMultiscaleGWRItem* layerItem, QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<GwmLayerAttributeItem*> indepVars = layerItem->indepVarsOrigin();
    for (int i = 0; i < indepVars.size() + 1; ++i)
    {
        GwmParameterSpecifiedOption* option = new GwmParameterSpecifiedOption();
        option->attributeName = i == 0 ? tr("Intercept") : indepVars[i - 1]->attributeName();
        option->bandwidthSize = layerItem->bandwidthSize()[i];
        option->bandwidthUnit = layerItem->bandwidthUnit()[i];
        option->bandwidthType = layerItem->bandwidthType()[i];
        option->kernel = layerItem->bandwidthKernelFunction()[i];
        option->bandwidthSeledType = layerItem->bandwidthSeled()[i];
        option->approach = layerItem->bandwidthSelectionApproach()[i];
        option->distanceType = layerItem->distanceSource()[i];
        option->distanceParameters = layerItem->distSrcParameters()[i];
        mItems.append(option);
    }
}

GwmPropertyMultiscaleParameterSpecifiedItemModel::~GwmPropertyMultiscaleParameterSpecifiedItemModel()
{
    for (GwmParameterSpecifiedOption* option : mItems)
    {
        delete option;
    }
}

QVariant GwmPropertyMultiscaleParameterSpecifiedItemModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QModelIndex GwmPropertyMultiscaleParameterSpecifiedItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() > 1)
        return QModelIndex();

    if (parent.isValid())
        return createIndex(row, column, mItems[parent.row()]);
    else return createIndex(row, column);
}

QModelIndex GwmPropertyMultiscaleParameterSpecifiedItemModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    if (index.internalPointer())
        return createIndex(mItems.indexOf((GwmParameterSpecifiedOption*)index.internalPointer()), 0);
    else return QModelIndex();
}

int GwmPropertyMultiscaleParameterSpecifiedItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return parent.internalPointer() ? 0 : 7;
    else return mItems.size();
}

int GwmPropertyMultiscaleParameterSpecifiedItemModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant GwmPropertyMultiscaleParameterSpecifiedItemModel::data(const QModelIndex &index, int role) const
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
                { "Unit", option->bandwidthUnit },
                { "Type", BandwidthTypeName[option->bandwidthType] },
                { "Kernel", KernelFunctionName[option->kernel] },
                { "Seled", BandwidthSeledTypeName[option->bandwidthSeledType] },
                { "Approach", BandwidthSelectionApproachName[option->approach] },
                { "Distance", DistanceTypeName[option->distanceType] }
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
                QString("%1 %2").arg(BandwidthTypeName[option->bandwidthType]).arg(option->bandwidthSize)
            };
            return dataArray[index.column()];
        default:
            return QVariant();
        }
    }
}

#ifndef GWMPARAMETERSPECIFIEDOPTIONSMODEL_H
#define GWMPARAMETERSPECIFIEDOPTIONSMODEL_H

#include <QAbstractListModel>
#include "TaskThread/gwmmultiscalegwralgorithm.h"
#include "Model/gwmlayerattributeitemmodel.h"

struct GwmParameterSpecifiedOption
{
    QString attributeName;
    int attributeIndex;
    bool checkState = false;

    double bandwidthSize = 100.0;
    bool adaptive = true;

    GwmBandwidthWeight::KernelFunctionType kernel = GwmBandwidthWeight::KernelFunctionType::Gaussian;
    GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType bandwidthSeledType = GwmMultiscaleGWRAlgorithm::Null;
    GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType approach = GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType::AIC;
    double threshold = 0.01;

    GwmDistance::DistanceType distanceType = GwmDistance::DistanceType::CRSDistance;
    double p = 2.0;
    double theta = 0.0;
    QString dmatFile;

    bool predictorCentralization = true;
};

class GwmParameterSpecifiedOptionsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit GwmParameterSpecifiedOptionsModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    GwmParameterSpecifiedOption* item(const QModelIndex &index);
    GwmParameterSpecifiedOption* item(const int row);

    void syncWithAttributes(const GwmVariableItemModel* attributeModel);
    void mItemUnshift();

private:
    QList<GwmParameterSpecifiedOption> mItems;
};

#endif // GWMPARAMETERSPECIFIEDOPTIONSMODEL_H

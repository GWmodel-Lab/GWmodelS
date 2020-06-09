#ifndef GWMPARAMETERSPECIFIEDOPTIONSMODEL_H
#define GWMPARAMETERSPECIFIEDOPTIONSMODEL_H

#include <QAbstractListModel>
#include "TaskThread/gwmmultiscalegwrtaskthread.h"
#include "Model/gwmlayerattributeitemmodel.h"

struct GwmParameterSpecifiedOption
{
    QString attributeName;
    int attributeIndex;
    bool checkState = false;

    GwmMultiscaleGWRTaskThread::BandwidthSeledType bandwidthSeledType = GwmMultiscaleGWRTaskThread::Null;
    GwmGWRTaskThread::BandwidthSelectionApproach approach = GwmGWRTaskThread::CV;
    GwmGWRTaskThread::BandwidthType bandwidthType = GwmGWRTaskThread::Adaptive;
    double bandwidthSize = 0;
    QString bandwidthUnit = "x1";
    GwmGWRTaskThread::KernelFunction kernel = GwmGWRTaskThread::Gaussian;

    GwmGWRTaskThread::DistanceSourceType distanceType = GwmGWRTaskThread::CRS;
    QVariant distanceParameters = QVariant();

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

    void syncWithAttributes(GwmLayerAttributeItemModel* attributeModel);

private:
    QList<GwmParameterSpecifiedOption> mItems;
};

#endif // GWMPARAMETERSPECIFIEDOPTIONSMODEL_H

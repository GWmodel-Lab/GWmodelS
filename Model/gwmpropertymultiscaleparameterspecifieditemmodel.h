#ifndef GWMPROPERTYMUULTISCALEPARAMETERSPECIFIEDITEMMODEL_H
#define GWMPROPERTYMUULTISCALEPARAMETERSPECIFIEDITEMMODEL_H

#include <QAbstractItemModel>

#include <Model/gwmparameterspecifiedoptionsmodel.h>
#include <Model/gwmlayermultiscalegwritem.h>

class GwmPropertyMultiscaleParameterSpecifiedItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    static QMap<GwmMultiscaleGWRTaskThread::BandwidthSeledType, QString> BandwidthSeledTypeName;
    static QMap<GwmGWRTaskThread::BandwidthSelectionApproach, QString> BandwidthSelectionApproachName;
    static QMap<GwmGWRTaskThread::BandwidthType, QString> BandwidthTypeName;
    static QMap<GwmGWRTaskThread::KernelFunction, QString> KernelFunctionName;
    static QMap<GwmGWRTaskThread::DistanceSourceType, QString> DistanceTypeName;

public:
    explicit GwmPropertyMultiscaleParameterSpecifiedItemModel(GwmLayerMultiscaleGWRItem* layerItem, QObject *parent = nullptr);
    ~GwmPropertyMultiscaleParameterSpecifiedItemModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QVector<GwmParameterSpecifiedOption*> mItems;
};

#endif // GWMPROPERTYMUULTISCALEPARAMETERSPECIFIEDITEMMODEL_H

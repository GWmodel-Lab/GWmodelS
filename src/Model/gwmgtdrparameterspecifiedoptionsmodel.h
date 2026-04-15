// 新建文件：src/Model/gwmgtdrparameterspecifiedoptionsmodel.h
#ifndef GWMGTDRPARAMETERSPECIFIEDOPTIONSMODEL_H
#define GWMGTDRPARAMETERSPECIFIEDOPTIONSMODEL_H

#include <QAbstractListModel>
#include "Model/gwmvariableitemmodel.h"
#include "Model/gwmlayerattributeitemmodel.h"
#include "SpatialWeight/gwmbandwidthweight.h"
#include <gwmodel.h>

struct GwmGTDRParameterSpecifiedOption
{
    QString attributeName;
    int attributeIndex;

    double initialBandwidthSize = 100.0;
    gwm::BandwidthWeight::KernelFunctionType kernel = gwm::BandwidthWeight::KernelFunctionType::Gaussian;
};

class GwmGTDRParameterSpecifiedOptionsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit GwmGTDRParameterSpecifiedOptionsModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    GwmGTDRParameterSpecifiedOption* item(const QModelIndex &index);
    GwmGTDRParameterSpecifiedOption* item(const int row);

    void syncWithAttributes(const GwmVariableItemModel* attributeModel);

private:
    QList<GwmGTDRParameterSpecifiedOption> mItems;
};

#endif // GWMGTDRPARAMETERSPECIFIEDOPTIONSMODEL_H

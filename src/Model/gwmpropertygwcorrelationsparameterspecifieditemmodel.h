#ifndef GWMPROPERTYGWCORRELATIONSPARAMETERSPECIFIEDITEMMODEL_H
#define GWMPROPERTYGWCORRELATIONSPARAMETERSPECIFIEDITEMMODEL_H

#include <QAbstractItemModel>
#include <Model/gwmparameterspecifiedoptionsmodel.h>
#include "Model/gwmlayergwcorrelationitem.h"

class GwmPropertyGWCorrelationsParameterSpecifiedItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit GwmPropertyGWCorrelationsParameterSpecifiedItemModel(GwmLayerGWCorrelationItem* layerItem, QObject *parent = nullptr);
    ~GwmPropertyGWCorrelationsParameterSpecifiedItemModel();
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
    QList<GwmParameterSpecifiedOption*> mItems;
};

#endif // GWMPROPERTYGWCORRELATIONSPARAMETERSPECIFIEDITEMMODEL_H

#ifndef GWMLAYOUTBATCHLAYERLISTMODEL_H
#define GWMLAYOUTBATCHLAYERLISTMODEL_H

#include <QAbstractListModel>

#include <qgsmaplayer.h>

#include "Model/gwmlayeritemmodel.h"

class GwmLayoutBatchLayerListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    struct Item
    {
        QgsMapLayer* layer = nullptr;
        bool selected = false;

        Item(QgsMapLayer* pLayer)
        {
            layer = pLayer;
            selected = false;
        }
    };

public:
    explicit GwmLayoutBatchLayerListModel(GwmLayerItemModel* layerItemModel, QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QList<QgsMapLayer*> checkedLayers();
    QgsMapLayer* layerFromIndex(const QModelIndex& index);

private:
    QList<Item> mMapLayerList;
};



#endif // GWMLAYOUTBATCHLAYERLISTMODEL_H

#ifndef GWMLAYOUTBATCHLAYERLISTMODEL_H
#define GWMLAYOUTBATCHLAYERLISTMODEL_H

#include <QAbstractListModel>

#include <qgsmaplayer.h>

#include "Model/gwmlayeritemmodel.h"
#include "Layout/gwmlayoutbatchfieldlistmodel.h"

class GwmLayoutBatchLayerListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    struct Item
    {
        QgsVectorLayer* layer = nullptr;
        bool selected = false;
        GwmLayoutBatchFieldListModel* fieldModel = nullptr;

        explicit Item(QgsVectorLayer* pLayer, QObject* parent = nullptr);
        ~Item();
    };

public:
    explicit GwmLayoutBatchLayerListModel(GwmLayerItemModel* layerItemModel, QObject *parent = nullptr);
    ~GwmLayoutBatchLayerListModel();

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QList<QgsVectorLayer*> checkedLayers();
    int checkedIndex(const QgsVectorLayer* layer);
    QgsVectorLayer* layerFromIndex(const QModelIndex& index);
    Item* itemFromindex(const QModelIndex& index);

private:
    QList<Item*> mMapLayerList;
};



#endif // GWMLAYOUTBATCHLAYERLISTMODEL_H

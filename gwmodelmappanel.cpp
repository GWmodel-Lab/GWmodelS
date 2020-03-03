#include "gwmodelmappanel.h"

#include <QLayout>
#include <QMessageBox>

GWmodelMapPanel::GWmodelMapPanel(QWidget *parent, QStandardItemModel* model)
    : QWidget(parent)
    , mapModel(model)
{
    mapCanvas = new QgsMapCanvas();
    mapCanvas->setLayers(mapLayerSet);
    mapCanvas->setVisible(true);
    QGridLayout* layout = new QGridLayout(parent);
    layout->addWidget(mapCanvas);
    layout->setMargin(0);
    setLayout(layout);

    connect(mapModel, &QStandardItemModel::rowsInserted, this, &GWmodelMapPanel::onMapItemInserted);
}

GWmodelMapPanel::~GWmodelMapPanel()
{

}

void GWmodelMapPanel::onMapItemInserted(const QModelIndex &parent, int first, int last)
{
    if (!parent.isValid())
    {
        bool isSetExtend = false;
        if (mapLayerSet.length() < 1)
        {
            isSetExtend = true;
        }
        for (int i = first; i <= last; i++)
        {
            QMap<QString, QVariant> itemData = mapModel->item(i)->data().toMap();
            QString path = itemData["path"].toString();
            QgsVectorLayer* vectorLayer = new QgsVectorLayer(path, QString("Layer%1").arg(i));
            if (vectorLayer->isValid())
            {
                mapLayerSet.append(vectorLayer);
            }
        }
        mapCanvas->setLayers(mapLayerSet);
        if (isSetExtend && mapLayerSet.length() > 0)
        {
            mapCanvas->setExtent(mapLayerSet.first()->extent());
        }
        mapCanvas->refresh();
    }
}

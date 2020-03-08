#include "gwmodelmappanel.h"

#include <QStackedLayout>
#include <QMessageBox>

#include <QDebug>

GWmodelMapPanel::GWmodelMapPanel(QWidget *parent, QStandardItemModel* model)
    : QWidget(parent)
    , mapModel(model)
{
    mapCanvas = new QgsMapCanvas();
    mapCanvas->setLayers(mapLayerSet);
    mapCanvas->setVisible(true);
    QStackedLayout* layout = new QStackedLayout(parent);
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

// 显示图层函数
void GWmodelMapPanel::receiveShowLayer(const QModelIndex &index)
{
    //qDebug() << 11;
    qDebug("显示图层");
    qDebug() << index;
}

// 缩放至图层函数
void GWmodelMapPanel::receiveZoomLayer(const QModelIndex &index)
{
    qDebug("缩放图层");
    qDebug() << index;
}

// 属性表
void GWmodelMapPanel::receiveAttribute(const QModelIndex &index)
{
    qDebug("属性表");
    qDebug() << index;
}

// 投影到坐标系
void GWmodelMapPanel::receiveProj(const QModelIndex &index)
{
    qDebug("投影");
    qDebug() << index;
}

// 符号
void GWmodelMapPanel::receiveSymbol(const QModelIndex &index)
{
    qDebug("符号");
    qDebug() << index;
}

// 导出shp
void GWmodelMapPanel::receiveShp(const QModelIndex &index)
{
    qDebug("导出shp");
    qDebug() << index;
}

// 导出GeoJSON
void GWmodelMapPanel::receiveGeoJson(const QModelIndex &index)
{
    qDebug("导出GeoJson");
    qDebug() << index;
}

// 导出Excel
void GWmodelMapPanel::receiveExcel(const QModelIndex &index)
{
    qDebug("导出Excel");
    qDebug() << index;
}

// 导出Csv
void GWmodelMapPanel::receiveCsv(const QModelIndex &index)
{
    qDebug("导出Csv");
    qDebug() << index;
}

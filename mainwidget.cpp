#include <QtWidgets>
#include <QFileInfo>
#include <qgsvectorlayer.h>
#include "mainwidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , mainLayout(new QVBoxLayout)
    , mapModel(new QStandardItemModel)
    , mapLayerNameDict()
{
    createMainZone();
    createToolbar();
    mainLayout->addWidget(toolbar);
    mainLayout->addWidget(mainZone);
    setLayout(mainLayout);
    mainLayout->setStretchFactor(mainZone, 1);
}

MainWidget::~MainWidget()
{

}

void MainWidget::openFileImportShapefile(){
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open ESRI Shapefile"), tr(""), tr("ESRI Shapefile (*.shp)"));
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists())
    {
        QString fileName = fileInfo.baseName();
        QStandardItem* item = new QStandardItem(fileName);
        QMap<QString, QVariant> itemData;
        itemData["path"] = QVariant(filePath);
        item->setData(QVariant(itemData));
        item->setCheckable(true);
        item->setCheckState(Qt::CheckState::Checked);
        mapModel->appendRow(item);
    }
}

void MainWidget::openFileImportJson(){
    QFileDialog::getOpenFileName(this, tr("Open GeoJson"), tr(""), tr("GeoJson (*.json *.geojson)"));
}

void MainWidget::openFileImportCsv(){
    QFileDialog::getOpenFileName(this, tr("Open CSV"), tr(""), tr("CSV (*.csv)"));
}

void MainWidget::createToolbar()
{
    toolbar = new GwmToolbar(this);
    // 连接信号槽
    connect(toolbar, &GwmToolbar::openFileImportShapefileSignal, this, &MainWidget::openFileImportShapefile);
    connect(toolbar, &GwmToolbar::openFileImportJsonSignal, this, &MainWidget::openFileImportJson);
    connect(toolbar, &GwmToolbar::openFileImportCsvSignal, this, &MainWidget::openFileImportCsv);
    connect(toolbar, &GwmToolbar::openByXYBtnSingnal, this, &MainWidget::openFileImportCsv);
    connect(toolbar, &GwmToolbar::fullScreenBtnSignal, this, &MainWidget::onFullScreen);
}

void MainWidget::createMainZone()
{
    mainZone = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(mainZone);
    layout->setMargin(0);

    createFeaturePanel();
    createPropertyPanel();
    createMapPanel();

    layout->addWidget(featurePanel);
    layout->addWidget(mapCanvas);
    layout->addWidget(propertyPanel);
    layout->setStretchFactor(mapCanvas, 1);

    mainZone->setLayout(layout);
}

void MainWidget::createFeaturePanel()
{
    featurePanel = new GwmFeaturePanel(mainZone, mapModel);
    featurePanel->setFixedWidth(320);
    // 连接信号槽
    connect(featurePanel, &GwmFeaturePanel::showLayerPropertySignal, this, &MainWidget::onShowLayerProperty);
}

void MainWidget::createPropertyPanel()
{
    propertyPanel = new GwmPropertyPanel(mainZone, mapModel);
    propertyPanel->setFixedWidth(420);
}

void MainWidget::createMapPanel()
{
    mapCanvas = new QgsMapCanvas();
    mapCanvas->setLayers(mapLayerSet);
    mapCanvas->setVisible(true);

    // 连接信号槽
    connect(mapModel, &QStandardItemModel::rowsInserted, this, &MainWidget::onMapItemInserted);
}

void MainWidget::onMapItemInserted(const QModelIndex &parent, int first, int last)
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
            QStandardItem* item = mapModel->item(i);
            QMap<QString, QVariant> itemData = item->data().toMap();
            QString path = itemData["path"].toString();
            QgsVectorLayer* vectorLayer = new QgsVectorLayer(path, item->text());
            if (vectorLayer->isValid())
            {
                mapLayerSet.append(vectorLayer);
                mapLayerNameDict[item->text()] = vectorLayer;
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

void MainWidget::onShowLayerProperty(const QModelIndex &index)
{
    QStandardItem* item = mapModel->itemFromIndex(index);
    qDebug() << "Layer Name: " << item->text();
    QString layerName = item->text();
    QgsVectorLayer* layer = mapLayerNameDict[layerName];
    propertyPanel->addStatisticTab(index, layer);
}

void MainWidget::onFullScreen()
{
    auto extent = mapCanvas->fullExtent();
    mapCanvas->setExtent(extent);
    mapCanvas->refresh();
}

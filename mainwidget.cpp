#include <QtWidgets>
#include <QFileInfo>
#include <qgsvectorlayer.h>
#include "mainwidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , mainLayout(new QVBoxLayout)
    , toolBar(new GwmToolbar)
    , mapModel(new QStandardItemModel)
{
    createMainZone();
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(mainZone);
    setLayout(mainLayout);
    mainLayout->setStretchFactor(mainZone, 1);

    connect(toolBar, &GwmToolbar::openFileImportShapefileSignal, this, &MainWidget::openFileImportShapefile);
    connect(toolBar, &GwmToolbar::openFileImportJsonSignal, this, &MainWidget::openFileImportJson);
    connect(toolBar, &GwmToolbar::openFileImportCsvSignal, this, &MainWidget::openFileImportCsv);
    connect(toolBar, &GwmToolbar::openByXYBtnSingnal, this, &MainWidget::openFileImportCsv);
    // 连接MainWidget和MapPanel
//    connect(this,SIGNAL(sendDataSigShowLayer(const QModelIndex &)), mapPanel, SLOT(receiveShowLayer(const QModelIndex &)));
//    connect(this,SIGNAL(sendDataSigZoomLayer(const QModelIndex &)), mapPanel, SLOT(receiveZoomLayer(const QModelIndex &)));
//    connect(this,SIGNAL(sendDataSigAttributeTable(const QModelIndex &)), mapPanel, SLOT(receiveAttribute(const QModelIndex &)));
//    connect(this,SIGNAL(sendDataSigProj(const QModelIndex &)), mapPanel, SLOT(receiveProj(const QModelIndex &)));
//    connect(this,SIGNAL(sendDataSigSymbol(const QModelIndex &)), mapPanel, SLOT(receiveSymbol(const QModelIndex &)));
//    connect(this,SIGNAL(sendDataSigEsriShp(const QModelIndex &)),mapPanel,SLOT(receiveShp(const QModelIndex &)));
//    connect(this,SIGNAL(sendDataSigGeoJson(const QModelIndex &)),mapPanel,SLOT(receiveGeoJson(const QModelIndex &)));
//    connect(this,SIGNAL(sendDataSigExcel(const QModelIndex &)),mapPanel,SLOT(receiveExcel(const QModelIndex &)));
//    connect(this,SIGNAL(sendDataSigCsv(const QModelIndex &)),mapPanel,SLOT(receiveCsv(const QModelIndex &)));
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
}

void MainWidget::createPropertyPanel()
{
    propertyPanel = new QTabWidget(mainZone);
    propertyPanel->setFixedWidth(420);
    // Demo Tab
    QLabel* demoTab = new QLabel(tr("Select a feature to show its property."), propertyPanel);
    propertyPanel->addTab(demoTab, tr("Property"));
    // [End] Demo Tab
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

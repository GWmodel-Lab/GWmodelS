#include "mainwidget.h"
#include <QtWidgets>
#include <QFileInfo>
#include <QMouseEvent>

#include <qgsvectorlayer.h>
#include <qgsrenderer.h>
#include <qgsattributetableview.h>
#include <qgsattributetablemodel.h>
#include <qgsvectorlayercache.h>
#include <qgsattributetablefiltermodel.h>
#include <qgseditorwidgetregistry.h>
#include <qgsfeatureselectionmodel.h>

#include "gwmattributetableview.h"
//#include "qgssymbolselectordialog.h"
//#include "qgssinglesymbolrenderer.h"
#include <qgsapplication.h>
#include "qgsstyle.h"
#include <qdebug.h>
#include <qgsstylemodel.h>
#include <qgssinglesymbolrenderer.h>

#include "gwmopenxyeventlayerdialog.h"


//#include "qgssymbolselectordialog.h"
//#include "qgssinglesymbolrenderer.h"
#include <qgsapplication.h>
#include "qgsstyle.h"
#include <qdebug.h>
#include <qgsstylemodel.h>
#include <qgssinglesymbolrenderer.h>


MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , mainLayout(new QVBoxLayout)
    , mapModel(new GwmLayerItemModel)
    , mapLayerIdDict()
    , mapPoint0(-1, -1)
    , isFeaturePanelDragging(false)
{
    createMainZone();
    createToolbar();
    mainLayout->addWidget(toolbar);
    mainLayout->addWidget(mainZone);
    setLayout(mainLayout);
    mainLayout->setStretchFactor(mainZone, 1);
    QgsApplication::initQgis();
//    connect(mapModel, &QStandardItemModel::itemChanged, this, &MainWidget::onMapModelItemChanged);
    // 连接featurePanel和mainWidget
    connect(featurePanel, &GwmFeaturePanel::sendDataSigAttributeTable,this, &MainWidget::onShowAttributeTable);
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
        addLayerToModel(filePath, fileName, "ogr");
    }
}

void MainWidget::openFileImportJson()
{
    QFileDialog::getOpenFileName(this, tr("Open GeoJson"), tr(""), tr("GeoJson (*.json *.geojson)"));
}

void MainWidget::openFileImportCsv()
{
    GwmOpenXYEventLayerDialog* dialog = new GwmOpenXYEventLayerDialog(this);
    connect(dialog, &GwmOpenXYEventLayerDialog::addVectorLayer, this, &MainWidget::addLayerToModel);
    dialog->show();
}

void MainWidget::onSelectMode()
{
    mapCanvas->setMapTool(mapIdentifyTool);
}

void MainWidget::onNavigateMode()
{
    mapCanvas->setMapTool(mapPanTool);
}

void MainWidget::onEditMode()
{
    QgsVectorLayer* layer = (QgsVectorLayer*) mapLayerList[0];
    layer->selectAll();
}

void MainWidget::createToolbar()
{
    toolbar = new GwmToolbar(this);
    // 连接信号槽
    connect(toolbar, &GwmToolbar::openFileImportShapefileSignal, this, &MainWidget::openFileImportShapefile);
    connect(toolbar, &GwmToolbar::openFileImportJsonSignal, this, &MainWidget::openFileImportJson);
    connect(toolbar, &GwmToolbar::openFileImportCsvSignal, this, &MainWidget::openFileImportCsv);
    connect(toolbar, &GwmToolbar::openByXYBtnSingnal, this, &MainWidget::openFileImportCsv);
    connect(toolbar, &GwmToolbar::zoomFullBtnSignal, this, &MainWidget::onFullScreen);
    connect(toolbar, &GwmToolbar::selectBtnSignal, this, &MainWidget::onSelectMode);
    connect(toolbar, &GwmToolbar::moveBtnSignal, this, &MainWidget::onNavigateMode);
    connect(toolbar, &GwmToolbar::editBtnSignal, this, &MainWidget::onEditMode);
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
    connect(featurePanel, &GwmFeaturePanel::sendDataSigZoomLayer, this, &MainWidget::onZoomToLayer);
    connect(featurePanel, &GwmFeaturePanel::showLayerPropertySignal, this, &MainWidget::onShowLayerProperty);
    connect(featurePanel, &GwmFeaturePanel::rowOrderChangedSignal, this, &MainWidget::onFeaturePanelRowOrderChanged);
    connect(featurePanel, &GwmFeaturePanel::beginDragDropSignal, this, &MainWidget::onFeaturePanelBeginDragDrop);
    connect(featurePanel, &GwmFeaturePanel::endDragDropSignal, this, &MainWidget::onFeaturePanelEndDragDrop);

    //连接槽函数
    connect(featurePanel,&GwmFeaturePanel::sendDataSigSymbol,this,&MainWidget::symbolSlot);

}

void MainWidget::createPropertyPanel()
{
    propertyPanel = new GwmPropertyPanel(mainZone, mapModel);
    propertyPanel->setFixedWidth(420);
}

void MainWidget::createMapPanel()
{
    mapCanvas = new QgsMapCanvas();
    mapCanvas->setLayers(mapLayerList);
    mapCanvas->setVisible(true);
//    mapCanvas->setSelectionColor(QColor(255, 0, 0));

    // 工具
    mapPanTool = new QgsMapToolPan(mapCanvas);
    mapIdentifyTool = new GwmMapToolIdentifyFeature(mapCanvas);
    mapCanvas->setMapTool(mapIdentifyTool);

    // 连接信号槽
    connect(mapModel, &GwmLayerItemModel::layerAddedSignal, this, &MainWidget::onMapModelChanged);
    connect(mapModel, &GwmLayerItemModel::layerRemovedSignal, this, &MainWidget::onMapModelChanged);
    connect(mapModel, &GwmLayerItemModel::layerItemChangedSignal, this, &MainWidget::onMapModelChanged);
    connect(mapCanvas, &QgsMapCanvas::selectionChanged, this, &MainWidget::onMapSelectionChanged);
}

void MainWidget::addLayerToModel(const QString &uri, const QString &layerName, const QString &providerKey)
{
    qDebug() << "[MainWidget::addLayerToModel]"
             << uri << layerName << providerKey;
    QgsVectorLayer* vectorLayer = new QgsVectorLayer(uri, layerName, providerKey);
    if (vectorLayer->isValid())
    {
        mapModel->addLayer(vectorLayer);
    }
    else delete vectorLayer;
}

void MainWidget::onZoomToLayer(const QModelIndex &index)
{
    qDebug() << "[MainWidget::onZoomToLayer]"
             << "index:" << index;
    GwmLayerItem* item = mapModel->itemFromIndex(index);
    QgsVectorLayer* layer = mapModel->layerFromItem(item);
    if (layer)
    {
        mapCanvas->setExtent(layer->extent());
        mapCanvas->refresh();
    }
}

void MainWidget::onFullScreen()
{
    auto extent = mapCanvas->fullExtent();
    mapCanvas->setExtent(extent);
    mapCanvas->refresh();
}

void MainWidget::onMapSelectionChanged(QgsVectorLayer *layer)
{
    qDebug() << "[MainWidget]" << "Map Selection Changed: (layer " << layer->name() << ")";
    // 移除旧的橡皮条
    QList<QgsRubberBand*> rubbers0 = mapLayerRubberDict[layer];
    if (rubbers0.size() > 0)
    {
        for (QgsRubberBand* rubber : rubbers0)
        {
            rubber->reset();
        }
    }
    rubbers0.clear();
    // 添加新的橡皮条
    QgsFeatureList selectedFeatures = layer->selectedFeatures();
    for (QgsFeature feature : selectedFeatures)
    {
        QgsGeometry geometry = feature.geometry();
        QgsRubberBand* rubber = new QgsRubberBand(mapCanvas, geometry.type());
        rubber->addGeometry(geometry, layer);
        rubber->setStrokeColor(QColor(255, 0, 0));
        rubber->setFillColor(QColor(255, 0, 0, 144));
        mapLayerRubberDict[layer] += rubber;
    }
}

void MainWidget::onMapModelChanged()
{
    qDebug() << "[MainWidget::onMapModelItemChanged]"
             << "isFeaturePanelDragging" << isFeaturePanelDragging;
    if (!isFeaturePanelDragging)
    {
        mapLayerList = mapModel->toMapLayerList();
        mapCanvas->setLayers(mapLayerList);
        if (mapLayerList.size() == 1)
        {
            QgsRectangle extent = mapLayerList.first()->extent();
            mapCanvas->setExtent(extent);
        }
        mapCanvas->refresh();
    }
}

void MainWidget::onShowLayerProperty(const QModelIndex &index)
{
    propertyPanel->addPropertyTab(index);
}


void MainWidget::onFeaturePanelRowOrderChanged(int from, int dest)
{
    qDebug() << "[MainWidget::onFeaturePanelRowOrderChanged]"
             << "layer list size" << mapLayerList.size();
    mapLayerList = mapModel->toMapLayerList();
    mapCanvas->setLayers(mapLayerList);
    mapCanvas->refresh();
}

void MainWidget::onFeaturePanelBeginDragDrop()
{
    this->isFeaturePanelDragging = true;
}

void MainWidget::onFeaturePanelEndDragDrop()
{
    this->isFeaturePanelDragging = false;
}

// 属性表
void MainWidget::onShowAttributeTable(const QModelIndex &index)
{
    // qDebug() << 123;
    qDebug() << "[MainWidget::onShowAttributeTable]"
             << "index:" << index;
    GwmLayerItem* item = mapModel->itemFromIndex(index);
    QgsVectorLayer* currentLayer = mapModel->layerFromItem(item);
    if (currentLayer)
    {
        currentLayer->setProviderEncoding("UTF-8");
        GwmAttributeTableView* tv = new GwmAttributeTableView(this);
        tv->setDisplayMapLayer(mapCanvas, currentLayer);
        tv->show();
    }
}

void MainWidget::onAttributeTableSelected(QgsVectorLayer* layer, QList<QgsFeatureId> list)
{
    for (QgsFeatureId id : list)
    {
        qDebug() << "[MainWidget::receiveSigAttriToMap]"
                 << "id:" << id;
    }
}
void MainWidget::symbolSlot(const QModelIndex &index)
{
    createSymbolWindow(index);
    connect(symbolWindow,&GwmSymbolWindow::canvasRefreshSingal,this,&MainWidget::refreshCanvas);
    symbolWindow->show();
}

void MainWidget::refreshCanvas(){
    mapCanvas->refresh();
}

void MainWidget::createSymbolWindow(const QModelIndex &index)
{
    GwmLayerItem* item = mapModel->itemFromIndex(index);
    QgsVectorLayer* layer = mapModel->layerFromItem(item);
    if (layer)
    {
        symbolWindow = new GwmSymbolWindow(layer);
    }
}

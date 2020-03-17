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
    , mapModel(new QStandardItemModel)
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
    connect(mapModel, &QStandardItemModel::itemChanged, this, &MainWidget::onMapModelItemChanged);
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
    connect(featurePanel, &GwmFeaturePanel::removeLayerSignal, this, &MainWidget::onRemoveLayer);


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
    connect(mapModel, &QStandardItemModel::rowsInserted, this, &MainWidget::onMapItemInserted);
    connect(mapCanvas, &QgsMapCanvas::selectionChanged, this, &MainWidget::onMapSelectionChanged);
}

void MainWidget::addLayerToModel(const QString &uri, const QString &layerName, const QString &providerKey)
{
    QStandardItem* item = new QStandardItem(layerName);
    QMap<QString, QVariant> itemData;
    itemData["path"] = QVariant(uri);
    itemData["provider"] = QVariant(providerKey);
    item->setData(QVariant(itemData));
    item->setCheckable(true);
    item->setCheckState(Qt::CheckState::Checked);
    mapModel->appendRow(item);
}

void MainWidget::onMapItemInserted(const QModelIndex &parent, int first, int last)
{
    if (!isFeaturePanelDragging)
    {
        if (!parent.isValid())
        {
            bool isSetExtend = false;
            if (mapLayerList.length() < 1)
            {
                isSetExtend = true;
            }
            for (int i = first; i <= last; i++)
            {
                QStandardItem* item = mapModel->item(i);
                QMap<QString, QVariant> itemData = item->data().toMap();
                QString name = item->text();
                QString path = itemData["path"].toString();
                QString provider = itemData["provider"].toString();
                QgsVectorLayer* vectorLayer = new QgsVectorLayer(path, name, provider);
                if (vectorLayer->isValid())
                {
                    QString layerID = QString("%1")
                            .arg((unsigned long long)vectorLayer, 0, 16)
                            .toUpper();
                    mapLayerList.append(vectorLayer);
                    mapLayerIdDict[layerID] = vectorLayer;
                    mapLayerRubberDict[vectorLayer] = QList<QgsRubberBand*>();
                    // 记录ID
                    itemData["ID"] = QVariant(layerID);
                    item->setData(itemData);
                    // 获取符号
                    QgsSingleSymbolRenderer* renderer = (QgsSingleSymbolRenderer*) vectorLayer->renderer();
                    QgsSymbol* symbol = renderer->symbol();
                    QSize iconSize(12, 12);
                    QPixmap icon(iconSize);
                    icon.fill(Qt::transparent);
                    QPainter iconPainter(&icon);
                    symbol->drawPreviewIcon(&iconPainter, iconSize);
                    item->setIcon(icon);
                }
            }
            mapCanvas->setLayers(mapLayerList);
            if (isSetExtend && mapLayerList.length() > 0)
            {
                mapCanvas->setExtent(mapLayerList.first()->extent());
                mapCanvas->refresh();
            }
        }
    }
}

void MainWidget::onZoomToLayer(const QModelIndex &index)
{
    QStandardItem* item = mapModel->itemFromIndex(index);
    QString layerID = item->data().toMap()["ID"].toString();
    QgsVectorLayer* layer = mapLayerIdDict[layerID];
    mapCanvas->setExtent(layer->extent());
    mapCanvas->refresh();
}

void MainWidget::onRemoveLayer(const QModelIndex &index)
{
    QStandardItem* item = mapModel->itemFromIndex(index);
    QString layerName = item->text();
    QString layerID = item->data().toMap()["ID"].toString();
    QgsVectorLayer* layer = mapLayerIdDict[layerID];
    mapModel->removeRow(index.row());
    mapLayerIdDict.remove(layerID);
    delete layer;
    deriveLayersFromModel();
    mapCanvas->setLayers(mapLayerList);
    mapCanvas->refresh();
}

void MainWidget::onShowLayerProperty(const QModelIndex &index)
{
    QStandardItem* item = mapModel->itemFromIndex(index);
    qDebug() << "Layer Name: " << item->text();
    QString layerName = item->text();
    QString layerID = item->data().toMap()["ID"].toString();
    QgsVectorLayer* layer = mapLayerIdDict[layerID];
    propertyPanel->addStatisticTab(index, layer);
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

void MainWidget::deriveLayersFromModel()
{
    int modelSize = mapModel->rowCount();
    mapLayerList.clear();
    for (int r = 0; r < modelSize; ++r)
    {
        QStandardItem* item = mapModel->item(r);
        bool isLayerShow = item->checkState() == Qt::Unchecked ? false : true;
        if (isLayerShow)
        {
            QString layerID = item->data().toMap()["ID"].toString();
            mapLayerList += mapLayerIdDict[layerID];
        }
    }
}

void MainWidget::onMapModelItemChanged(QStandardItem* item)
{
    qDebug() << "[MainWidget::onMapModelItemChanged]"
             << "isFeaturePanelDragging" << isFeaturePanelDragging;
    if (!isFeaturePanelDragging)
    {
        deriveLayersFromModel();
        mapCanvas->setLayers(mapLayerList);
        mapCanvas->refresh();
    }
}


void MainWidget::onFeaturePanelRowOrderChanged(int from, int dest)
{
    deriveLayersFromModel();
    qDebug() << "[MainWidget::onFeaturePanelRowOrderChanged]"
             << "layer list size" << mapLayerList.size();
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
    qDebug() << index;
    // QgsEditorWidgetRegistry test;
    // test.initEditors(mapCanvas);
    // 获取当前矢量图层路径
    QMap<QString, QVariant> itemData = mapModel->itemFromIndex(index)->data().toMap();
    // 当前矢量图层
    QString layerID = itemData["ID"].toString();
    QgsVectorLayer* currentLayer = mapLayerIdDict[layerID];
    // 设置图层编码格式支持中文
    currentLayer->setProviderEncoding("UTF-8");
    GwmAttributeTableView* tv = new GwmAttributeTableView();
    tv->setDisplayMapLayer(mapCanvas, currentLayer);
    tv->show();
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
    symbolWindow = new GwmSymbolWindow(dynamic_cast<QgsVectorLayer *>(mapLayerList[index.row()]));
}

#include <QtWidgets>
#include <QFileInfo>
#include <qgsvectorlayer.h>
#include <qgsrenderer.h>
#include "mainwidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , mainLayout(new QVBoxLayout)
    , mapModel(new QStandardItemModel)
    , mapLayerNameDict()
    , mapPoint0(-1, -1)
{
    createMainZone();
    createToolbar();
    mainLayout->addWidget(toolbar);
    mainLayout->addWidget(mainZone);
    setLayout(mainLayout);
    mainLayout->setStretchFactor(mainZone, 1);
    // 绑定模型 itemChanged 信号
    connect(mapModel, &QStandardItemModel::itemChanged, this, &MainWidget::onMapModelItemChanged);
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
        // 遍历查找同名元素的个数
        int duplicateCount = 0;
        for (int i = 0; i < mapModel->rowCount(); ++i)
        {
            QStandardItem* cur = mapModel->item(i);
            if (cur->text().startsWith(fileName))
            {
                duplicateCount++;
            }
        }
        // 重复名字后面添加标号
        QString itemText = duplicateCount > 0 ?
                    QString("%1 (%2)").arg(fileName).arg(duplicateCount + 1) :
                    fileName;
        QStandardItem* item = new QStandardItem(itemText);
        QMap<QString, QVariant> itemData;
        itemData["path"] = QVariant(filePath);
        itemData["name"] = QVariant(fileName);
        item->setData(QVariant(itemData));
        item->setCheckable(true);
        item->setCheckState(Qt::CheckState::Checked);
        mapModel->appendRow(item);
    }
}

void MainWidget::openFileImportJson()
{
    QFileDialog::getOpenFileName(this, tr("Open GeoJson"), tr(""), tr("GeoJson (*.json *.geojson)"));
}

void MainWidget::openFileImportCsv()
{
    QFileDialog::getOpenFileName(this, tr("Open CSV"), tr(""), tr("CSV (*.csv)"));
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
    connect(toolbar, &GwmToolbar::fullScreenBtnSignal, this, &MainWidget::onFullScreen);
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

void MainWidget::onMapItemInserted(const QModelIndex &parent, int first, int last)
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
            QString path = itemData["path"].toString();
            QString name = itemData["name"].toString();
            QgsVectorLayer* vectorLayer = new QgsVectorLayer(path, name);
            if (vectorLayer->isValid())
            {
                mapLayerList.append(vectorLayer);
                mapLayerNameDict[item->text()] = vectorLayer;
                mapLayerRubberDict[vectorLayer] = QList<QgsRubberBand*>();
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
        QString name = item->text();
        bool isLayerShow = item->checkState() == Qt::Unchecked ? false : true;
        if (isLayerShow)
        {
            mapLayerList += mapLayerNameDict[name];
        }
    }
}

void MainWidget::onMapModelItemChanged(QStandardItem* item)
{
    deriveLayersFromModel();
    mapCanvas->setLayers(mapLayerList);
    mapCanvas->refresh();
}

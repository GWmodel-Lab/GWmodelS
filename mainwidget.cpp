#include "mainwidget.h"
#include "ui_mainwidget.h"

#include <QtWidgets>
#include <QFileInfo>
#include <QMouseEvent>
#include <QDebug>

#include <qgsvectorlayer.h>
#include <qgsrenderer.h>
#include "mainwidget.h"

#include <qgsapplication.h>

#include <qgsattributetableview.h>
#include <qgsattributetablemodel.h>
#include <qgsvectorlayercache.h>
#include <qgsattributetablefiltermodel.h>
#include <qgseditorwidgetregistry.h>
#include <qgsfeatureselectionmodel.h>
#include <qgsapplication.h>
#include <qgsstyle.h>
#include <qgsstylemodel.h>
#include <qgssinglesymbolrenderer.h>

#include "gwmattributetableview.h"
#include "gwmopenxyeventlayerdialog.h"


//#include "qgssymbolselectordialog.h"
//#include "qgssinglesymbolrenderer.h"
#include <qgsapplication.h>
#include "qgsstyle.h"
#include <qdebug.h>
#include <qgsstylemodel.h>
#include <qgssinglesymbolrenderer.h>


#include "gwmcoordinate.h"

#include "qgsprojectionselectionwidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , mapModel(new GwmLayerItemModel)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    setupMapPanel();
    setupFeaturePanel();
    setupToolbar();
    createMainZone();
    createToolbar();
    mainLayout->addWidget(toolbar);
    mainLayout->addWidget(mainZone);
    setLayout(mainLayout);
    mainLayout->setStretchFactor(mainZone, 1);
    QgsApplication::initQgis();
//    connect(mapModel, &QStandardItemModel::itemChanged, this, &MainWidget::onMapModelItemChanged);
    connect(mapModel, &QStandardItemModel::itemChanged, this, &MainWidget::onMapModelItemChanged);

    Gwm_Coordinate = new GwmCoordinate();

    // 连接featurePanel和mainWidget
    connect(featurePanel, &GwmFeaturePanel::sendDataSigAttributeTable,this, &MainWidget::onShowAttributeTable);
    connect(featurePanel, &GwmFeaturePanel::sendDataSigProj,this,&MainWidget::onShowCoordinate);
    // 连接投影坐标系窗口和主窗口
    connect(Gwm_Coordinate, &GwmCoordinate::sendSigCoordinate,this,&MainWidget::handleCoordinate);
    //connect(Gwm_Coordinate, &GwmCoordinate::sendSigSetCoordinate,this,&MainWidget::setNewCoordinate);
}

MainWidget::~MainWidget()
{
    delete ui;
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
    connect(dialog, &GwmOpenXYEventLayerDialog::addVectorLayerSignal, this, &MainWidget::addLayerToModel);
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

void MainWidget::setupToolbar()
{
    toolbar = ui->toolbar;
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

void MainWidget::setupFeaturePanel()
{
    featurePanel = ui->featurePanel;
    featurePanel->setMapModel(mapModel);
    // 连接信号槽
    connect(featurePanel, &GwmFeaturePanel::showAttributeTableSignal,this, &MainWidget::onShowAttributeTable);
    connect(featurePanel, &GwmFeaturePanel::zoomToLayerSignal, this, &MainWidget::onZoomToLayer);
    connect(featurePanel, &GwmFeaturePanel::showLayerPropertySignal, this, &MainWidget::onShowLayerProperty);
    connect(featurePanel, &GwmFeaturePanel::rowOrderChangedSignal, this, &MainWidget::onFeaturePanelRowOrderChanged);
    connect(featurePanel, &GwmFeaturePanel::showSymbolSettingSignal, this, &MainWidget::onShowSymbolSetting);

}

void MainWidget::setupPropertyPanel()
{
    propertyPanel = ui->propertyPanel;
    propertyPanel->setMapModel(mapModel);
}

void MainWidget::setupMapPanel()
{
    mapCanvas = ui->mapCanvas;
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
        mapModel->appendItem(vectorLayer);
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
    mapLayerList = mapModel->toMapLayerList();
    mapCanvas->setLayers(mapLayerList);
    if (mapLayerList.size() == 1)
    {
        QgsRectangle extent = mapLayerList.first()->extent();
        mapCanvas->setExtent(extent);
    }
    mapCanvas->refresh();
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
void MainWidget::onShowSymbolSetting(const QModelIndex &index)
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

// 投影到坐标系
void MainWidget::onShowCoordinate(const QModelIndex &index)
{
    // qDebug() << index;
    // 获取当前矢量图层路径
    QMap<QString, QVariant> itemData = mapModel->itemFromIndex(index)->data().toMap();
    // 当前矢量图层
    QString layerID = itemData["ID"].toString();
    QgsVectorLayer* currentLayer = mapLayerIdDict[layerID];
    // 原始图层的投影坐标系
    QString resPrj = QgsProjectionSelectionWidget::crsOptionText(currentLayer->sourceCrs());
    //qDebug() << resPrj;
    Gwm_Coordinate->setProperty("resPrj",resPrj);
    Gwm_Coordinate->setProperty("MapIndex",index);
    Gwm_Coordinate->show();

    qDebug()<<currentLayer->getGeometry(1).asJson()<<1;
}

// 投影到坐标系的实现函数
void MainWidget::handleCoordinate(QString des, QModelIndex index)
{
//    qDebug()<<"PKGPath="<<QgsApplication::pkgDataPath();
//    qDebug()<<"DBFilePath="<<QgsApplication::srsDatabaseFilePath();
//    QgsCoordinateReferenceSystem wgs84(4326,QgsCoordinateReferenceSystem::EpsgCrsId);
//    qDebug()<<"WGS84="<<wgs84.toWkt();
//    QgsCoordinateReferenceSystem web(3857,QgsCoordinateReferenceSystem::EpsgCrsId);



//    QgsCoordinateTransform trans;
//    trans.setSourceCrs(wgs84);
//    trans.setDestinationCrs(web);
//    QgsPointXY pt(116,40);
//    QgsPointXY ptweb = trans.transform(pt,QgsCoordinateTransform::ForwardTransform);
//    qDebug()<<"trans116,40="<<ptweb.x()<<","<<ptweb.y();

    // 获取当前矢量图层路径
    QMap<QString, QVariant> itemData = mapModel->itemFromIndex(index)->data().toMap();
//    // 当前矢量图层
    QString layerID = itemData["ID"].toString();
    QgsVectorLayer* currentLayer = mapLayerIdDict[layerID];
//    // qDebug() << mapCanvas->getCoordinateTransform();

//    ori = currentLayer->crs().toWkt();
//    // 获取用户填写的目标坐标系
//    int UserDes = des.toInt();
//    QgsCoordinateReferenceSystem DesCoordinate(UserDes,QgsCoordinateReferenceSystem::EpsgCrsId);
//    mapCanvas->setDestinationCrs(DesCoordinate);
//    mapCanvas->setExtent(mapCanvas->fullExtent());
//    mapCanvas->refresh();
    // qDebug() << mapCanvas->getCoordinateTransform();
    //qDebug() << currentLayer->crs().fromWkt(des);
    // 设置新的坐标系
//    currentLayer->setCrs(currentLayer->crs().fromWkt(des));
//    // 当前坐标系
//    qDebug() << currentLayer->sourceCrs().toWkt();
//    mapCanvas->refresh();
    //qDebug()<<currentLayer->getGeometry(1).asJson()<<1;

    QgsCoordinateTransform myTransform;
    myTransform.setSourceCrs(currentLayer->sourceCrs());
    myTransform.setDestinationCrs(currentLayer->crs().fromWkt(des));

    QgsFeatureIterator featureIt = currentLayer->getFeatures();
//    QgsFeatureIds featureIdSet = currentLayer->allFeatureIds();
//    QgsFeatureIds::iterator featureIdItera = featureIdSet.begin();
    QgsFeature f;
    int idx = 0;

    // 放入新的图层
    QString layerProperties = "Point?";    // 几何类型
    //QString layerProperties = currentLayer->geometryType();
    QgsVectorLayer *newLayer = new QgsVectorLayer(layerProperties, QString( "临时点层" ), QString( "memory" ));
    QgsVectorDataProvider* dateProvider = newLayer->dataProvider();

    while(featureIt.nextFeature(f))
    {
        qDebug() << idx;
        QgsGeometry g = f.geometry();
        if(g.transform(myTransform) == 0)
        {
            //qDebug() << f.geometry().asJson();
            f.setGeometry(g);
            //qDebug() << f.geometry().asJson();
        }
        else
        {
            f.clearGeometry();
        }
        idx++;
        dateProvider->addFeature(f);
        newLayer->commitChanges();
    }
    //mapCanvas->refresh();
    //qDebug()<<currentLayer->getGeometry(1).asJson()<<2;
    mapLayerList.append(newLayer);
    mapCanvas->setLayers(mapLayerList);
    mapCanvas->refresh();
    qDebug() << newLayer->getGeometry(1).asJson() << 3;
}



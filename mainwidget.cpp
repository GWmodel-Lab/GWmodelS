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


#include "gwmcoordtranssettingdialog.h"
#include "qgsprojectionselectionwidget.h"
#include "gwmcoordtranssettingdialog.h"
#include "gwmprogressdialog.h"

#include <qgsproviderregistry.h>
#include <qgsdatumtransformdialog.h>


MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , mapModel(new GwmLayerItemModel)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    setupMapPanel();
    setupFeaturePanel();
    setupToolbar();
    setupPropertyPanel();
    // ！！！重要！！！
    // 将 FeaturePanel, MapCanvas, PropertyPanel 相关的设置代码全部写在对应的 setup函数 中！
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
        createLayerToModel(filePath, fileName, "ogr");
    }
}

void MainWidget::openFileImportJson()
{
    QFileDialog::getOpenFileName(this, tr("Open GeoJson"), tr(""), tr("GeoJson (*.json *.geojson)"));
}

void MainWidget::openFileImportCsv()
{
    GwmOpenXYEventLayerDialog* dialog = new GwmOpenXYEventLayerDialog(this);
    connect(dialog, &GwmOpenXYEventLayerDialog::addVectorLayerSignal, this, &MainWidget::createLayerToModel);
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
//    QgsVectorLayer* layer = (QgsVectorLayer*) mapLayerList[0];
//    layer->selectAll();
    if ( QMessageBox::question( this,tr( "Warning" ),
                                          tr( "Are you sure to delete the features?" ) ) == QMessageBox::Yes ){
        for(int i = 0; i < mapLayerList.size(); i++){
            ((QgsVectorLayer *)mapLayerList[i])->startEditing();
            qDebug() << ((QgsVectorLayer *)mapLayerList[i])->deleteSelectedFeatures();
            qDebug() << ((QgsVectorLayer *)mapLayerList[i])->commitChanges();
            onMapSelectionChanged((QgsVectorLayer *)mapLayerList[i]);
        }
        mapCanvas->refresh();
    }
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
    connect(toolbar, &GwmToolbar::saveLayerBtnSignal, this, &MainWidget::onSaveLayer);
    connect(toolbar, &GwmToolbar::exportLayerBtnSignal, this, &MainWidget::onExportLayer);
    connect(toolbar,&GwmToolbar::zoomToLayerBtnSignal,this,&MainWidget::onZoomToLayerBtn);
    connect(toolbar,&GwmToolbar::zoomToSelectionBtnSignal,this,&MainWidget::onZoomToSelection);

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
    connect(featurePanel, &GwmFeaturePanel::showCoordinateTransDlg,this,&MainWidget::onShowCoordinateTransDlg);
    connect(featurePanel, &GwmFeaturePanel::currentChanged,this,&MainWidget::onFeaturePanelCurrentChanged);

}

void MainWidget::setupPropertyPanel()
{
    propertyPanel = ui->propertyPanel;
    propertyPanel->setMapModel(mapModel);
}

void MainWidget::setupMapPanel()
{
    mapCanvas = ui->mapCanvas;
    mapCanvas->setDestinationCrs(QgsProject::instance()->crs());
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

void MainWidget::addLayerToModel(QgsVectorLayer *layer)
{
    if (layer->isValid())
    {
        mapModel->appendItem(layer);
    }
}

void MainWidget::createLayerToModel(const QString &uri, const QString &layerName, const QString &providerKey)
{
    qDebug() << "[MainWidget::addLayerToModel]"
             << uri << layerName << providerKey;
    QgsVectorLayer* vectorLayer = new QgsVectorLayer(uri, layerName, providerKey);
    if (vectorLayer->isValid())
    {
        mapModel->appendItem(vectorLayer,uri,providerKey);
    }
    else delete vectorLayer;
}

void MainWidget::onFeaturePanelCurrentChanged(const QModelIndex &current,const QModelIndex &previous){
    qDebug() << current;
    qDebug() << previous;
    if(current.isValid()){
        GwmLayerVectorItem* layerItem;
        GwmLayerItem* item = mapModel->itemFromIndex(current);
        switch (item->itemType()) {
            case GwmLayerItem::GwmLayerItemType::Group:
                layerItem = ((GwmLayerGroupItem*)item)->originChild();
            break;
            case GwmLayerItem::GwmLayerItemType::Vector:
            case GwmLayerItem::GwmLayerItemType::Origin:
            case GwmLayerItem::GwmLayerItemType::GWR:
                layerItem = ((GwmLayerVectorItem*)item);
            break;
            default:
                layerItem = nullptr;
        }
        if(layerItem && layerItem->itemType() != GwmLayerItem::GwmLayerItemType::Symbol){
            toolbar->setBtnEnabled(true);
        }
        else{
            toolbar->setBtnEnabled(false);
        }
    }
    else{
        toolbar->setBtnEnabled(false);
    }
}

void MainWidget::onZoomToLayer(const QModelIndex &index)
{
    qDebug() << "[MainWidget::onZoomToLayer]"
             << "index:" << index;
    GwmLayerItem* item = mapModel->itemFromIndex(index);
    QgsVectorLayer* layer = mapModel->layerFromItem(item);
    if (layer)
    {
        QgsCoordinateTransform transform;
        transform.setSourceCrs(layer->crs());
        transform.setDestinationCrs(QgsProject::instance()->crs());
        QgsRectangle extent = transform.transformBoundingBox(layer->extent());
        mapCanvas->setExtent(extent);
        mapCanvas->refresh();
    }
}

void MainWidget::onFullScreen()
{
    auto extent = mapCanvas->fullExtent();
    mapCanvas->setExtent(extent);
    mapCanvas->refresh();
}

void MainWidget::onSaveLayer()
{
    QModelIndexList selected = featurePanel->selectionModel()->selectedIndexes();
    for (QModelIndex index : selected)
    {
        GwmLayerVectorItem* layerItem;
        GwmLayerItem* item = mapModel->itemFromIndex(index);
        switch (item->itemType()) {
            case GwmLayerItem::GwmLayerItemType::Group:
                layerItem = ((GwmLayerGroupItem*)item)->originChild();
            break;
            case GwmLayerItem::GwmLayerItemType::Vector:
            case GwmLayerItem::GwmLayerItemType::Origin:
            case GwmLayerItem::GwmLayerItemType::GWR:
                layerItem = ((GwmLayerVectorItem*)item);
            break;
            default:
                layerItem = nullptr;
        }
        if(layerItem && layerItem->itemType() != GwmLayerItem::GwmLayerItemType::Symbol){
            if(layerItem->provider() != "ogr"){
                QString filePath = QFileDialog::getSaveFileName(this,tr("Save file"),tr(""),tr("ESRI Shapefile (*.shp)"));
                QFileInfo fileInfo(filePath);
                QString fileName = fileInfo.baseName();
                QString file_suffix = fileInfo.suffix();
                layerItem->save(filePath,fileName,file_suffix);

            }
            else
            {
                QString filePath = layerItem->path();
                QFileInfo fileInfo(filePath);
                QString fileName = fileInfo.baseName();
                QString file_suffix = fileInfo.suffix();
                layerItem->save(filePath,fileName,file_suffix);
            }
        }
    }
}

void MainWidget::onExportLayer()
{
    QModelIndexList selected = featurePanel->selectionModel()->selectedIndexes();
    for (QModelIndex index : selected)
    {
        GwmLayerVectorItem* layerItem;
        GwmLayerItem* item = mapModel->itemFromIndex(index);
        switch (item->itemType()) {
            case GwmLayerItem::GwmLayerItemType::Group:
                layerItem = ((GwmLayerGroupItem*)item)->originChild();
            break;
            case GwmLayerItem::GwmLayerItemType::Vector:
            case GwmLayerItem::GwmLayerItemType::Origin:
            case GwmLayerItem::GwmLayerItemType::GWR:
                layerItem = ((GwmLayerVectorItem*)item);
            break;
            default:
                layerItem = nullptr;
        }
        if(layerItem && layerItem->itemType() != GwmLayerItem::GwmLayerItemType::Symbol){
                QString filePath = QFileDialog::getSaveFileName(this,tr("Save file"),tr(""),tr("ESRI Shapefile (*.shp)"));
                QFileInfo fileInfo(filePath);
                QString fileName = fileInfo.baseName();
                QString file_suffix = fileInfo.suffix();
                layerItem->save(filePath,fileName,file_suffix);
        }
    }
}

void MainWidget::onZoomToLayerBtn(){
    QModelIndexList selected = featurePanel->selectionModel()->selectedIndexes();
    for (QModelIndex index : selected){
        onZoomToLayer(index);
    }
}

void MainWidget::onZoomToSelection(){
    QModelIndexList selected = featurePanel->selectionModel()->selectedIndexes();
    for (QModelIndex index : selected){
        GwmLayerItem* item = mapModel->itemFromIndex(index);
        QgsVectorLayer* layer = mapModel->layerFromItem(item);
        mapCanvas->zoomToSelected(layer);
        mapCanvas->refresh();
    }
}

bool MainWidget::askUserForDatumTransfrom(const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QgsMapLayer *layer)
{
    Q_ASSERT( qApp->thread() == QThread::currentThread() );

      QString title;
      if ( layer )
      {
        // try to make a user-friendly (short!) identifier for the layer
        QString layerIdentifier;
        if ( !layer->name().isEmpty() )
        {
          layerIdentifier = layer->name();
        }
        else
        {
          const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( layer->providerType(), layer->source() );
          if ( parts.contains( QStringLiteral( "path" ) ) )
          {
            const QFileInfo fi( parts.value( QStringLiteral( "path" ) ).toString() );
            layerIdentifier = fi.fileName();
          }
          else if ( layer->dataProvider() )
          {
            const QgsDataSourceUri uri( layer->source() );
            layerIdentifier = uri.table();
          }
        }
        if ( !layerIdentifier.isEmpty() )
          title = tr( "Select Transformation for %1" ).arg( layerIdentifier );
      }

      return QgsDatumTransformDialog::run( sourceCrs, destinationCrs, this, mapCanvas, title );
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
        qDebug() << "[MainWidget::onMapSelectionChanged]"
                 << "selected" << geometry.asWkt();
    }
}

void MainWidget::onMapModelChanged()
{
    mapLayerList = mapModel->toMapLayerList();
    mapCanvas->setLayers(mapLayerList);
    if (mapLayerList.size() == 1)
    {
        QgsMapLayer* firstLayer = mapLayerList.first();
        QgsRectangle extent = mapCanvas->mapSettings().layerExtentToOutputExtent(firstLayer, firstLayer->extent());
        qDebug() << "[MainWidget::onMapModelChanged]"
                 << "origin extent: " << firstLayer->extent()
                 << "trans extent: " << extent;
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

bool MainWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == mapCanvas)
    {
        switch (e->type()) {
        case QEvent::MouseMove:
            qDebug() << "[MainWidget::eventFilter]"
                     << "Cur map position: (" << mapCanvas->getCoordinateTransform()->toMapCoordinates(((QMouseEvent*)e)->pos()).asWkt();
            return true;
        default:
            break;
        }
    }
    return false;
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
void MainWidget::onShowCoordinateTransDlg(const QModelIndex &index)
{
    // qDebug() << index;
    // 获取当前矢量图层
    GwmLayerItem* item = mapModel->itemFromIndex(index);
    QgsVectorLayer* currentLayer = mapModel->layerFromItem(item);
    // 原始图层的投影坐标系
    if (currentLayer)
    {
        GwmCoordTransSettingDialog *mCoordinateTransDlg = new GwmCoordTransSettingDialog(this);
        // 连接投影坐标系窗口和主窗口
        mCoordinateTransDlg->setSrcCrs(currentLayer->crs());
        if (mCoordinateTransDlg->exec() == QDialog::Accepted)
        {
            QgsCoordinateReferenceSystem desCrs = mCoordinateTransDlg->desCrs();
            GwmCoordTransThread* thread = new GwmCoordTransThread(currentLayer, desCrs);
            GwmProgressDialog* progressDlg = new GwmProgressDialog(thread, this);
            if (progressDlg->exec() == QDialog::Accepted)
            {
                qDebug() << "[MainWidget::onShowCoordinateTransDlg]"
                         << "Finished";
                addLayerToModel(thread->getWorkLayer());
            }
        }
    }
}


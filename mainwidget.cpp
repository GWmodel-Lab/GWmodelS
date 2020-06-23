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
#include <qgsprojectionselectionwidget.h>
#include <qgsproviderregistry.h>
#include <qgsdatumtransformdialog.h>

#include "gwmopenxyeventlayerdialog.h"
#include "gwmprogressdialog.h"
#include "gwmcoordtranssettingdialog.h"
#include "TaskThread/gwmcoordtransthread.h"
#include <gwmsaveascsvdialog.h>
#include <qgsvectorfilewriter.h>
#include <TaskThread/gwmcsvtodatthread.h>
#include "gwmggwroptionsdialog.h"

#include "gwmscalablegwroptionsdialog.h"
#include "TaskThread/gwmscalablegwralgorithm.h"
#include "Model/gwmlayerscalablegwritem.h"

#include "gwmmultiscalegwroptionsdialog.h"
#include "TaskThread/gwmmultiscalegwralgorithm.h"
#include "PropertyPanelTabs/gwmpropertymultiscalegwrtab.h"

#include "gwmattributetabledialog.h"
#include "Model/gwmlayergwritem.h"
#include "Model/gwmlayerggwritem.h"

#include <TaskThread/gwmbasicgwralgorithm.h>
#include "TaskThread/gwmggwralgorithm.h"
#include <Model/gwmlayergwssitem.h>
#include <gwmgwssoptionsdialog.h>
#include <SpatialWeight/gwmcrsdistance.h>

#include <Model/gwmlayerbasicgwritem.h>
#include <Model/gwmlayercollinearitygwritem.h>

#include <TaskThread/gwmgwpcataskthread.h>

#include "gwmgwpcaoptionsdialog.h"
#include <Model/gwmlayergwpcaitem.h>

//鲁棒GWR
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
    QgsGui::editorWidgetRegistry()->initEditors(mapCanvas);
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

void MainWidget::onCsvToDat()
{
    GwmCsvToDatDialog* csvtodatDlg = new GwmCsvToDatDialog();
    if(csvtodatDlg->exec() == QDialog::Accepted){
        QString csvFileName = csvtodatDlg->csvFileName();
        QString datFileName = csvtodatDlg->datFileName();
        if(csvFileName != "" && datFileName != ""){
            GwmCsvToDatThread* thread = new GwmCsvToDatThread(csvFileName,datFileName);
            thread->setIsColumnStore(csvtodatDlg->isColumnStore());
            GwmProgressDialog* progressDlg = new GwmProgressDialog(thread, this);
            if (progressDlg->exec() == QDialog::Accepted)
            {
                qDebug() << "[MainWidget::onCsvToDat]"
                         << "Finished";
            }
        }
    }
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
    connect(toolbar, &GwmToolbar::exportLayerBtnSignal, this, &MainWidget::onExportLayerAsShpfile);
    connect(toolbar, &GwmToolbar::zoomToLayerBtnSignal,this,&MainWidget::onZoomToLayerBtn);
    connect(toolbar, &GwmToolbar::zoomToSelectionBtnSignal,this,&MainWidget::onZoomToSelection);
    connect(toolbar,&GwmToolbar::gwmodelGWRBtnSignal,this,&MainWidget::onGWRBtnClicked);

    connect(toolbar,&GwmToolbar::gwmodelGWSSBtnSignal,this,&MainWidget::onGWSSBtnClicked);
    connect(toolbar, &GwmToolbar::gwmodelGWPCABtnSignal,this,&MainWidget::onGWPCABtnClicked);
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
    connect(featurePanel,&GwmFeaturePanel::sendDataSigCsv,this,&MainWidget::onExportLayerAsCsv);
    connect(ui->featureSortUpBtn, &QAbstractButton::clicked, featurePanel, &GwmFeaturePanel::onSortUpBtnClicked);
    connect(ui->featureSortDownBtn, &QAbstractButton::clicked, featurePanel, &GwmFeaturePanel::onSortDownBtnClicked);
    connect(ui->featureRemoveBtn, &QAbstractButton::clicked, featurePanel, &GwmFeaturePanel::removeLayer);
    connect(ui->featureSymbolBtn, &QAbstractButton::clicked, featurePanel, &GwmFeaturePanel::symbol);
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
    connect(mapModel, &GwmLayerItemModel::layerItemMovedSignal, this, &MainWidget::onMapModelChanged);
    connect(mapCanvas, &QgsMapCanvas::selectionChanged, this, &MainWidget::onMapSelectionChanged);
}

void MainWidget::addLayerToModel(QgsVectorLayer *layer)
{
    if (layer->isValid())
    {
        mapModel->appendItem(layer, layer->dataProvider()->dataSourceUri(), layer->providerType());
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
//    qDebug() << current;
//    qDebug() << previous;
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
        case GwmLayerItem::GwmLayerItemType::GGWR:
        case GwmLayerItem::GwmLayerItemType::ScalableGWR:
        case GwmLayerItem::GwmLayerItemType::MultiscaleGWR:
        case GwmLayerItem::GwmLayerItemType::GWSS:
        case GwmLayerItem::GwmLayerItemType::CollinearityGWR:
        case GwmLayerItem::GwmLayerItemType::GWPCA:
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
        // 设置图层组件工具按钮的状态
        ui->featureSortUpBtn->setEnabled(mapModel->canMoveUp(current));
        ui->featureSortDownBtn->setEnabled(mapModel->canMoveDown(current));
        ui->featureRemoveBtn->setEnabled(mapModel->canRemove(current));
        ui->featureSymbolBtn->setEnabled(mapModel->canSetSymbol(current));
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
        case GwmLayerItem::GwmLayerItemType::GGWR:
        case GwmLayerItem::GwmLayerItemType::ScalableGWR:
        case GwmLayerItem::GwmLayerItemType::MultiscaleGWR:
        case GwmLayerItem::GwmLayerItemType::GWSS:
        case GwmLayerItem::GwmLayerItemType::CollinearityGWR:
        case GwmLayerItem::GwmLayerItemType::GWPCA:
            layerItem = ((GwmLayerVectorItem*)item);
            break;
        default:
            layerItem = nullptr;
        }
        if(layerItem && layerItem->itemType() != GwmLayerItem::GwmLayerItemType::Symbol){
            if(layerItem->provider() != "ogr"){
                QString filePath = QFileDialog::getSaveFileName(this,tr("Save file"),tr(""),tr("ESRI Shapefile (*.shp)"));
                if(filePath != ""){
                    QFileInfo fileInfo(filePath);
                    QString fileName = fileInfo.baseName();
                    QString file_suffix = fileInfo.suffix();
                    layerItem->save(filePath,fileName,file_suffix);
                }
            }
            else
            {
                 mapModel->layerFromItem(item)->commitChanges();
            }
        }
    }
}

void MainWidget::onExportLayerAsShpfile()
{
    onExportLayer(tr("ESRI Shapefile (*.shp)"));
}

void MainWidget::onExportLayerAsCsv(const QModelIndex &index)
{
//    onExportLayer(tr("CSV (*.csv)"));
    GwmLayerItem* item = mapModel->itemFromIndex(index);
    GwmLayerVectorItem* layerItem;
    switch (item->itemType()) {
    case GwmLayerItem::GwmLayerItemType::Group:
        layerItem = ((GwmLayerGroupItem*)item)->originChild();
        break;
    case GwmLayerItem::GwmLayerItemType::Vector:
    case GwmLayerItem::GwmLayerItemType::Origin:
    case GwmLayerItem::GwmLayerItemType::GWR:
    case GwmLayerItem::GwmLayerItemType::GGWR:
    case GwmLayerItem::GwmLayerItemType::ScalableGWR:
    case GwmLayerItem::GwmLayerItemType::MultiscaleGWR:
    case GwmLayerItem::GwmLayerItemType::GWSS:
    case GwmLayerItem::GwmLayerItemType::CollinearityGWR:
    case GwmLayerItem::GwmLayerItemType::GWPCA:
        layerItem = ((GwmLayerVectorItem*)item);
        break;
    default:
        layerItem = nullptr;
    }
    if(layerItem && layerItem->itemType() != GwmLayerItem::GwmLayerItemType::Symbol){
        GwmSaveAsCSVDialog* saveAsCSVDlg = new GwmSaveAsCSVDialog();
        if(saveAsCSVDlg->exec() == QDialog::Accepted){
            QString filePath = saveAsCSVDlg->filepath();
            if(filePath != ""){
                QFileInfo fileInfo(filePath);
                QString fileName = fileInfo.baseName();
                QString file_suffix = fileInfo.suffix();
                QgsVectorFileWriter::SaveVectorOptions& options = *(new QgsVectorFileWriter::SaveVectorOptions());
                if(saveAsCSVDlg->isAddXY()){
                    QStringList layerOptions;
                    if(layerItem->layer()->geometryType() == QgsWkbTypes::Type::Point){
                        layerOptions << QStringLiteral( "%1=%2" ).arg( "GEOMETRY", "AS_XY" );
                    }
                    else{
                        layerOptions << QStringLiteral( "%1=%2" ).arg( "GEOMETRY", "AS_WKT" );
                    }
                    options.layerOptions = layerOptions;
                }
                layerItem->save(filePath,fileName,file_suffix,options);
            }
        }
    }
}

void MainWidget::onExportLayer(QString filetype)
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
        case GwmLayerItem::GwmLayerItemType::GGWR:
        case GwmLayerItem::GwmLayerItemType::ScalableGWR:
        case GwmLayerItem::GwmLayerItemType::MultiscaleGWR:
        case GwmLayerItem::GwmLayerItemType::GWSS:
        case GwmLayerItem::GwmLayerItemType::CollinearityGWR:
        case GwmLayerItem::GwmLayerItemType::GWPCA:
            layerItem = ((GwmLayerVectorItem*)item);
            break;
        default:
            layerItem = nullptr;
        }
        if(layerItem && layerItem->itemType() != GwmLayerItem::GwmLayerItemType::Symbol){
                QString filePath = QFileDialog::getSaveFileName(this,tr("Save file"),tr(""),filetype);
                if(filePath != ""){
                    QFileInfo fileInfo(filePath);
                    QString fileName = fileInfo.baseName();
                    QString file_suffix = fileInfo.suffix();
                    layerItem->save(filePath,fileName,file_suffix);
                }
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
        GwmAttributeTableDialog *d = new GwmAttributeTableDialog(currentLayer,mapCanvas,this,Qt::Dialog);
        d->show();
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


void MainWidget::onGWRBtnClicked()
{
    GwmBasicGWRAlgorithm* gwrTaskThread = new GwmBasicGWRAlgorithm();
    GwmGWROptionsDialog* gwrOptionDialog = new GwmGWROptionsDialog(mapModel->rootChildren(), gwrTaskThread);
    QModelIndexList selectedIndexes = featurePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwrOptionDialog->exec() == QDialog::Accepted)
    {
        gwrOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwrOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwrTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwrTaskThread->resultLayer();
            GwmLayerBasicGWRItem* gwrItem = new GwmLayerBasicGWRItem(selectedItem, resultLayer, gwrTaskThread);
            mapModel->appentItem(gwrItem, selectedIndex);
        }
    }
}

void MainWidget::onGWRNewBtnClicked()
{
    GwmBasicGWRAlgorithm* algorithm = new GwmBasicGWRAlgorithm();
    QgsVectorLayer* dataLayer = static_cast<GwmLayerGroupItem*>(mapModel->rootChildren()[0])->originChild()->layer();
    algorithm->setDataLayer(dataLayer);
    QgsFields fields = dataLayer->fields();
    GwmVariable depVar = {0, fields[0].name(), fields[0].type(), fields[0].isNumeric()};
    QList<GwmVariable> indepVars;
    for (int i : {1, 10, 12, 13, 15})
    {
        indepVars.append({ i, fields[i].name(), fields[i].type(), fields[i].isNumeric()});
    }
    algorithm->setDependentVariable(depVar);
    algorithm->setIndependentVariables(indepVars);
    algorithm->setIsAutoselectIndepVars(true);
    algorithm->setIndepVarSelectionThreshold(150.0);
    GwmSpatialWeight spatialWeight;
    spatialWeight.setDistance(GwmCRSDistance(dataLayer->featureCount(), false));
    spatialWeight.setWeight(GwmBandwidthWeight(36, true, GwmBandwidthWeight::Gaussian));
    algorithm->setSpatialWeight(spatialWeight);
    algorithm->setIsAutoselectBandwidth(true);
    algorithm->setBandwidthSelectionCriterionType(GwmBasicGWRAlgorithm::CV);
    algorithm->setHasHatMatrix(true);
    algorithm->setHasFTest(true);


    GwmProgressDialog* progressDlg = new GwmProgressDialog(algorithm);
    if (progressDlg->exec() == QDialog::Accepted)
    {
        QgsVectorLayer* resultLayer = algorithm->resultLayer();
        addLayerToModel(resultLayer);

        GwmDiagnostic diagnostic = algorithm->diagnostic();
        GwmBasicGWRAlgorithm::FTestResultPack fTestResult = algorithm->fTestResult();
    }
}

void MainWidget::onGWSSBtnClicked()
{
    GwmGWSSTaskThread* gwssTaskThread = new GwmGWSSTaskThread();
    GwmGWSSOptionsDialog* gwssOptionDialog = new GwmGWSSOptionsDialog(mapModel->rootChildren(), gwssTaskThread);
    QModelIndexList selectedIndexes = featurePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwssOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwssOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwssOptionDialog->exec() == QDialog::Accepted)
    {
        gwssOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwssOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwssTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwssTaskThread->resultLayer();
            GwmLayerGWSSItem* gwssItem = new GwmLayerGWSSItem(selectedItem, resultLayer, gwssTaskThread);
            mapModel->appentItem(gwssItem, selectedIndex);
        }
    }

}

void MainWidget::onScalableGWRBtnClicked()
{
    GwmScalableGWRAlgorithm* gwrTaskThread = new GwmScalableGWRAlgorithm();
    GwmScalableGWROptionsDialog* gwrOptionDialog = new GwmScalableGWROptionsDialog(mapModel->rootChildren(), gwrTaskThread);
    QModelIndexList selectedIndexes = featurePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwrOptionDialog->exec() == QDialog::Accepted)
    {
        gwrOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwrOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwrTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwrTaskThread->resultLayer();
            GwmLayerScalableGWRItem* gwrItem = new GwmLayerScalableGWRItem(selectedItem, resultLayer, gwrTaskThread);
            mapModel->appentItem(gwrItem, selectedIndex);
        }
    }
}

void MainWidget::onMultiscaleGWRBtnClicked()
{
    GwmMultiscaleGWRAlgorithm* gwrTaskThread = new GwmMultiscaleGWRAlgorithm();
    GwmMultiscaleGWROptionsDialog* gwrOptionDialog = new GwmMultiscaleGWROptionsDialog(mapModel->rootChildren(), gwrTaskThread);
    QModelIndexList selectedIndexes = featurePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwrOptionDialog->exec() == QDialog::Accepted)
    {
        gwrOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwrOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwrTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwrTaskThread->resultLayer();
            GwmLayerMultiscaleGWRItem* gwrItem = new GwmLayerMultiscaleGWRItem(selectedItem, resultLayer, gwrTaskThread);
            mapModel->appentItem(gwrItem, selectedIndex);
        }
    }
}

void MainWidget::onRobustGWR()
{
    GwmRobustGWRAlgorithm* gwrRobustTaskThread = new GwmRobustGWRAlgorithm();
    GwmRobustGWROptionsDialog* gwrRobustOptionDialog = new GwmRobustGWROptionsDialog(mapModel->rootChildren(), gwrRobustTaskThread);
    QModelIndexList selectedIndexes = featurePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwrRobustOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwrRobustOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwrRobustOptionDialog->exec() == QDialog::Accepted)
    {
        gwrRobustOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwrRobustOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwrRobustTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwrRobustTaskThread->resultLayer();
            GwmLayerBasicGWRItem* gwrItem = new GwmLayerBasicGWRItem(selectedItem, resultLayer, gwrRobustTaskThread);
            mapModel->appentItem(gwrItem, selectedIndex);
        }
    }
}

void MainWidget::onRobustGWRBtnClicked()
{
    GwmRobustGWRAlgorithm* gwrRobustTaskThread = new GwmRobustGWRAlgorithm();
    GwmRobustGWROptionsDialog* gwrRobustOptionDialog = new GwmRobustGWROptionsDialog(mapModel->rootChildren(), gwrRobustTaskThread);
    QModelIndexList selectedIndexes = featurePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwrRobustOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwrRobustOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwrRobustOptionDialog->exec() == QDialog::Accepted)
    {
        gwrRobustOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwrRobustOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwrRobustTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwrRobustTaskThread->resultLayer();
            GwmLayerBasicGWRItem* gwrItem = new GwmLayerBasicGWRItem(selectedItem, resultLayer, gwrRobustTaskThread);
            mapModel->appentItem(gwrItem, selectedIndex);
        }
    }
}

void MainWidget::onLcrGWRBtnClicked()
{
    GwmLocalCollinearityGWRAlgorithm * lcrGWRTaskThread = new GwmLocalCollinearityGWRAlgorithm();
    GwmLcrGWROptionsDialog* gwrLcrOptionDialog = new GwmLcrGWROptionsDialog(mapModel->rootChildren(), lcrGWRTaskThread);
    QModelIndexList selectedIndexes = featurePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwrLcrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwrLcrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwrLcrOptionDialog->exec() == QDialog::Accepted)
    {
        gwrLcrOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwrLcrOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(lcrGWRTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = lcrGWRTaskThread->resultLayer();
            GwmLayerCollinearityGWRItem* gwrItem = new GwmLayerCollinearityGWRItem(selectedItem, resultLayer, lcrGWRTaskThread);
            mapModel->appentItem(gwrItem, selectedIndex);
        }
    }
}

void MainWidget::onGGWRBtnClicked(){
    GwmGGWRAlgorithm* ggwrTaskThread = new GwmGGWRAlgorithm();
    GwmGGWROptionsDialog* ggwrOptionDialog = new GwmGGWROptionsDialog(this->mapModel->rootChildren(), ggwrTaskThread);
    QModelIndexList selectedIndexes = featurePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            ggwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            ggwrOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (ggwrOptionDialog->exec() == QDialog::Accepted)
    {
        ggwrOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = ggwrOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(ggwrTaskThread); //
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = ggwrTaskThread->resultLayer();
            GwmLayerGGWRItem* ggwrItem = new GwmLayerGGWRItem(selectedItem, resultLayer, ggwrTaskThread);
            mapModel->appentItem(ggwrItem, selectedIndex);
        }
    }
}



//void wpca(const mat &x, const vec &wt, double nu, double nv, mat &V, vec &S)
//{
//    //首先完成中心化
//    mat tmpLocalCenter(x.n_rows,x.n_cols,fill::zeros);
//    for(int i=0;i<x.n_rows;i++)
//    {
//        tmpLocalCenter.row(i) = x.row(i) * wt(i);
//    }
//    //SVD
//    mat tmpRes(5,2,fill::zeros);
//    tmpRes = (x.each_row() - sum(tmpLocalCenter)/sum(wt)).each_col() % sqrt(wt);
//    tmpRes.print();
//    mat U;
//    svd(U,S,V,tmpRes);
//    //S即为R中的d
//    //V即为R中的v
//}

void MainWidget::onGWPCABtnClicked()
{
    GwmGWPCATaskThread* gwpcaTaskThread = new GwmGWPCATaskThread();
    GwmGWPCAOptionsDialog* gwpcaOptionDialog = new GwmGWPCAOptionsDialog(mapModel->rootChildren(), gwpcaTaskThread);
    QModelIndexList selectedIndexes = featurePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mapModel->itemFromIndex(selectedIndex);
        if (selectedItem->itemType() == GwmLayerItem::Group)
        {
            gwpcaOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem));
        }
        else if (selectedItem->itemType() == GwmLayerItem::Origin)
        {
            gwpcaOptionDialog->setSelectedLayer(static_cast<GwmLayerGroupItem*>(selectedItem->parentItem()));
        }
    }
    if (gwpcaOptionDialog->exec() == QDialog::Accepted)
    {
        gwpcaOptionDialog->updateFields();
        GwmLayerGroupItem* selectedItem = gwpcaOptionDialog->selectedLayer();
        const QModelIndex selectedIndex = mapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwpcaTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwpcaTaskThread->resultLayer();
            GwmLayerGWPCAItem * gwrItem = new GwmLayerGWPCAItem(selectedItem, resultLayer, gwpcaTaskThread);
            mapModel->appentItem(gwrItem, selectedIndex);
        }
    }
}

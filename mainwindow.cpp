#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMenuBar>
#include <QAction>
#include <QFileDialog>

#include <QtWidgets>
#include <QFileInfo>
#include <QMouseEvent>
#include <QDebug>

#include <qgsvectorlayer.h>
#include <qgsrenderer.h>

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
#include "gwmsaveascsvdialog.h"
#include "qgsvectorfilewriter.h"
#include "TaskThread/gwmcsvtodatthread.h"
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

#include "TaskThread/gwmbasicgwralgorithm.h"
#include "TaskThread/gwmgeneralizedgwralgorithm.h"
#include "Model/gwmlayergwssitem.h"
#include "gwmgwssoptionsdialog.h"
#include "SpatialWeight/gwmcrsdistance.h"

#include "Model/gwmlayerbasicgwritem.h"
#include "Model/gwmlayercollinearitygwritem.h"

#include "TaskThread/gwmgwpcataskthread.h"

#include "gwmgwpcaoptionsdialog.h"
#include "Model/gwmlayergwpcaitem.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupMenus();
    setAttribute(Qt::WA_QuitOnClose);
    setupMapPanel();
    setupToolbar();
    setupFeaturePanel();
    setupPropertyPanel();
    QgsGui::editorWidgetRegistry()->initEditors(mMapCanvas);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMenus()
{
    connect(ui->action_ESRI_Shapefile, &QAction::triggered, this, &MainWindow::openFileImportShapefile);
    connect(ui->actionGeo_Json, &QAction::triggered, this, &MainWindow::openFileImportJson);
    connect(ui->action_CSV, &QAction::triggered, this, &MainWindow::openFileImportCsv);
    connect(ui->action_CsvToDat, &QAction::triggered, this, &MainWindow::onCsvToDat);
    connect(ui->actionRobustGWR,&QAction::triggered,this,&MainWindow::onRobustGWR);
    connect(ui->actionScalable_GWR, &QAction::triggered,this,&MainWindow::onScalableGWRBtnClicked);
    connect(ui->action_GGWR,&QAction::triggered, this, &MainWindow::onGGWRBtnClicked);
    connect(ui->actionLocal_collinearity_GWR,&QAction::triggered, this, &MainWindow::onLcrGWRBtnClicked);
    connect(ui->actionMultiscale_GWR,&QAction::triggered, this, &MainWindow::onMultiscaleGWRBtnClicked);
    connect(ui->actionBasic_GWR,&QAction::triggered, this, &MainWindow::onGWRBtnClicked);
}

void MainWindow::toggleToolbarGeneral(bool flag)
{
    ui->actionZoom_to_Layer->setEnabled(flag);
    ui->actionZoom_to_Area->setEnabled(flag);
    ui->actionSave_Layer->setEnabled(flag);
    ui->actionSave_Layer_As->setEnabled(flag);
}

void MainWindow::openFileImportShapefile(){
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open ESRI Shapefile"), tr(""), tr("ESRI Shapefile (*.shp)"));
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists())
    {
        QString fileName = fileInfo.baseName();
        createLayerToModel(filePath, fileName, "ogr");
    }
}

void MainWindow::openFileImportJson()
{
    QFileDialog::getOpenFileName(this, tr("Open GeoJson"), tr(""), tr("GeoJson (*.json *.geojson)"));
}

void MainWindow::openFileImportCsv()
{
    GwmOpenXYEventLayerDialog* dialog = new GwmOpenXYEventLayerDialog(this);
    connect(dialog, &GwmOpenXYEventLayerDialog::addVectorLayerSignal, this, &MainWindow::createLayerToModel);
    dialog->show();
}

void MainWindow::onCsvToDat()
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
                qDebug() << "[MainWindow::onCsvToDat]"
                         << "Finished";
            }
        }
    }
}

void MainWindow::onSelectMode()
{
    mMapCanvas->setMapTool(mMapIdentifyTool);
}

void MainWindow::onNavigateMode()
{
    mMapCanvas->setMapTool(mMapPanTool);
}

void MainWindow::onEditMode()
{
//    QgsVectorLayer* layer = (QgsVectorLayer*) mapLayerList[0];
//    layer->selectAll();
    if ( QMessageBox::question( this,tr( "Warning" ),
                                          tr( "Are you sure to delete the features?" ) ) == QMessageBox::Yes ){
        for(int i = 0; i < mMapLayerList.size(); i++){
            ((QgsVectorLayer *)mMapLayerList[i])->startEditing();
            qDebug() << ((QgsVectorLayer *)mMapLayerList[i])->deleteSelectedFeatures();
            qDebug() << ((QgsVectorLayer *)mMapLayerList[i])->commitChanges();
            onMapSelectionChanged((QgsVectorLayer *)mMapLayerList[i]);
        }
        mMapCanvas->refresh();
    }
}

void MainWindow::setupToolbar()
{
    // 连接信号槽
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFileImportShapefile);
    connect(ui->actionOpen_XY_Coordinate_Layer, &QAction::triggered, this, &MainWindow::openFileImportCsv);
//    connect(ui->action, &QAction::triggered, this, &MainWindow::openFileImportJson);
//    connect(ui-, &QAction::triggered, this, &MainWindow::openFileImportCsv);
    connect(ui->actionSelect_Feature, &QAction::triggered, this, &MainWindow::onSelectMode);
    connect(ui->actionPan, &QAction::triggered, this, &MainWindow::onNavigateMode);
    connect(ui->actionEdit, &QAction::triggered, this, &MainWindow::onEditMode);
    connect(ui->actionSave_Layer, &QAction::triggered, this, &MainWindow::onSaveLayer);
    connect(ui->actionSave_Layer_As, &QAction::triggered, this, &MainWindow::onExportLayerAsShpfile);
    connect(ui->actionZoom_to_Area, &QAction::triggered,this,&MainWindow::onZoomToSelection);
    connect(ui->actionZoom_to_Layer, &QAction::triggered,this,&MainWindow::onZoomToLayerBtn);
    connect(ui->actionZoom_Full_Extent, &QAction::triggered, this, &MainWindow::onFullScreen);

    connect(ui->actionGWR, &QAction::triggered,this,&MainWindow::onGWRBtnClicked);
    connect(ui->actionGWSS, &QAction::triggered,this,&MainWindow::onGWSSBtnClicked);
    connect(ui->actionGWPCA, &QAction::triggered,this,&MainWindow::onGWPCABtnClicked);
}

void MainWindow::setupFeaturePanel()
{
    mFeaturePanel = ui->featurePanel;
    mFeaturePanel->setMapModel(mMapModel);
    // 连接信号槽
    connect(mFeaturePanel, &GwmFeaturePanel::showAttributeTableSignal,this, &MainWindow::onShowAttributeTable);
    connect(mFeaturePanel, &GwmFeaturePanel::zoomToLayerSignal, this, &MainWindow::onZoomToLayer);
    connect(mFeaturePanel, &GwmFeaturePanel::showLayerPropertySignal, this, &MainWindow::onShowLayerProperty);
    connect(mFeaturePanel, &GwmFeaturePanel::rowOrderChangedSignal, this, &MainWindow::onFeaturePanelRowOrderChanged);
    connect(mFeaturePanel, &GwmFeaturePanel::showSymbolSettingSignal, this, &MainWindow::onShowSymbolSetting);
    connect(mFeaturePanel, &GwmFeaturePanel::showCoordinateTransDlg,this,&MainWindow::onShowCoordinateTransDlg);
    connect(mFeaturePanel, &GwmFeaturePanel::currentChanged,this,&MainWindow::onFeaturePanelCurrentChanged);
    connect(mFeaturePanel,&GwmFeaturePanel::sendDataSigCsv,this,&MainWindow::onExportLayerAsCsv);
    connect(ui->featureSortUpBtn, &QAbstractButton::clicked, mFeaturePanel, &GwmFeaturePanel::onSortUpBtnClicked);
    connect(ui->featureSortDownBtn, &QAbstractButton::clicked, mFeaturePanel, &GwmFeaturePanel::onSortDownBtnClicked);
    connect(ui->featureRemoveBtn, &QAbstractButton::clicked, mFeaturePanel, &GwmFeaturePanel::removeLayer);
    connect(ui->featureSymbolBtn, &QAbstractButton::clicked, mFeaturePanel, &GwmFeaturePanel::symbol);
}

void MainWindow::setupPropertyPanel()
{
    mPropertyPanel = ui->propertyPanel;
    mPropertyPanel->setMapModel(mMapModel);
}

void MainWindow::setupMapPanel()
{
    mMapCanvas = ui->mapCanvas;
    mMapCanvas->setDestinationCrs(QgsProject::instance()->crs());
    // 工具
    mMapPanTool = new QgsMapToolPan(mMapCanvas);
    mMapIdentifyTool = new GwmMapToolIdentifyFeature(mMapCanvas);
    mMapCanvas->setMapTool(mMapIdentifyTool);

    // 连接信号槽
    connect(mMapModel, &GwmLayerItemModel::layerAddedSignal, this, &MainWindow::onMapModelChanged);
    connect(mMapModel, &GwmLayerItemModel::layerRemovedSignal, this, &MainWindow::onMapModelChanged);
    connect(mMapModel, &GwmLayerItemModel::layerItemChangedSignal, this, &MainWindow::onMapModelChanged);
    connect(mMapModel, &GwmLayerItemModel::layerItemMovedSignal, this, &MainWindow::onMapModelChanged);
    connect(mMapCanvas, &QgsMapCanvas::selectionChanged, this, &MainWindow::onMapSelectionChanged);
}

void MainWindow::addLayerToModel(QgsVectorLayer *layer)
{
    if (layer->isValid())
    {
        mMapModel->appendItem(layer, layer->dataProvider()->dataSourceUri(), layer->providerType());
    }
}

void MainWindow::createLayerToModel(const QString &uri, const QString &layerName, const QString &providerKey)
{
    qDebug() << "[MainWindow::addLayerToModel]"
             << uri << layerName << providerKey;
    QgsVectorLayer* vectorLayer = new QgsVectorLayer(uri, layerName, providerKey);
    if (vectorLayer->isValid())
    {
        mMapModel->appendItem(vectorLayer,uri,providerKey);
    }
    else delete vectorLayer;
}

void MainWindow::onFeaturePanelCurrentChanged(const QModelIndex &current,const QModelIndex &previous){
//    qDebug() << current;
//    qDebug() << previous;
    if(current.isValid()){
        GwmLayerVectorItem* layerItem;
        GwmLayerItem* item = mMapModel->itemFromIndex(current);
        switch (item->itemType()) {
        case GwmLayerItem::GwmLayerItemType::Group:
            layerItem = ((GwmLayerGroupItem*)item)->originChild();
            break;
        case GwmLayerItem::GwmLayerItemType::Vector:
        case GwmLayerItem::GwmLayerItemType::Origin:
        case GwmLayerItem::GwmLayerItemType::GWR:
        case GwmLayerItem::GwmLayerItemType::GeneralizedGWR:
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
            this->toggleToolbarGeneral(true);
        }
        else{
            this->toggleToolbarGeneral(false);
        }
        // 设置图层组件工具按钮的状态
        ui->featureSortUpBtn->setEnabled(mMapModel->canMoveUp(current));
        ui->featureSortDownBtn->setEnabled(mMapModel->canMoveDown(current));
        ui->featureRemoveBtn->setEnabled(mMapModel->canRemove(current));
        ui->featureSymbolBtn->setEnabled(mMapModel->canSetSymbol(current));
    }
    else{
        this->toggleToolbarGeneral(false);
    }
}

void MainWindow::onZoomToLayer(const QModelIndex &index)
{
    qDebug() << "[MainWindow::onZoomToLayer]"
             << "index:" << index;
    GwmLayerItem* item = mMapModel->itemFromIndex(index);
    QgsVectorLayer* layer = mMapModel->layerFromItem(item);
    if (layer)
    {
        QgsCoordinateTransform transform;
        transform.setSourceCrs(layer->crs());
        transform.setDestinationCrs(QgsProject::instance()->crs());
        QgsRectangle extent = transform.transformBoundingBox(layer->extent());
        mMapCanvas->setExtent(extent);
        mMapCanvas->refresh();
    }
}

void MainWindow::onFullScreen()
{
    auto extent = mMapCanvas->fullExtent();
    mMapCanvas->setExtent(extent);
    mMapCanvas->refresh();
}

void MainWindow::onSaveLayer()
{
    QModelIndexList selected = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex index : selected)
    {
        GwmLayerVectorItem* layerItem;
        GwmLayerItem* item = mMapModel->itemFromIndex(index);
        switch (item->itemType()) {
        case GwmLayerItem::GwmLayerItemType::Group:
            layerItem = ((GwmLayerGroupItem*)item)->originChild();
            break;
        case GwmLayerItem::GwmLayerItemType::Vector:
        case GwmLayerItem::GwmLayerItemType::Origin:
        case GwmLayerItem::GwmLayerItemType::GWR:
        case GwmLayerItem::GwmLayerItemType::GeneralizedGWR:
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
                 mMapModel->layerFromItem(item)->commitChanges();
            }
        }
    }
}

void MainWindow::onExportLayerAsShpfile()
{
    onExportLayer(tr("ESRI Shapefile (*.shp)"));
}

void MainWindow::onExportLayerAsCsv(const QModelIndex &index)
{
//    onExportLayer(tr("CSV (*.csv)"));
    GwmLayerItem* item = mMapModel->itemFromIndex(index);
    GwmLayerVectorItem* layerItem;
    switch (item->itemType()) {
    case GwmLayerItem::GwmLayerItemType::Group:
        layerItem = ((GwmLayerGroupItem*)item)->originChild();
        break;
    case GwmLayerItem::GwmLayerItemType::Vector:
    case GwmLayerItem::GwmLayerItemType::Origin:
    case GwmLayerItem::GwmLayerItemType::GWR:
    case GwmLayerItem::GwmLayerItemType::GeneralizedGWR:
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

void MainWindow::onExportLayer(QString filetype)
{
    QModelIndexList selected = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex index : selected)
    {
        GwmLayerVectorItem* layerItem;
        GwmLayerItem* item = mMapModel->itemFromIndex(index);
        switch (item->itemType()) {
        case GwmLayerItem::GwmLayerItemType::Group:
            layerItem = ((GwmLayerGroupItem*)item)->originChild();
            break;
        case GwmLayerItem::GwmLayerItemType::Vector:
        case GwmLayerItem::GwmLayerItemType::Origin:
        case GwmLayerItem::GwmLayerItemType::GWR:
        case GwmLayerItem::GwmLayerItemType::GeneralizedGWR:
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

void MainWindow::onZoomToLayerBtn(){
    QModelIndexList selected = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex index : selected){
        onZoomToLayer(index);
    }
}

void MainWindow::onZoomToSelection(){
    QModelIndexList selected = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex index : selected){
        GwmLayerItem* item = mMapModel->itemFromIndex(index);
        QgsVectorLayer* layer = mMapModel->layerFromItem(item);
        mMapCanvas->zoomToSelected(layer);
        mMapCanvas->refresh();
    }
}

bool MainWindow::askUserForDatumTransfrom(const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QgsMapLayer *layer)
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

    return QgsDatumTransformDialog::run( sourceCrs, destinationCrs, this, mMapCanvas, title );
}

void MainWindow::onMapSelectionChanged(QgsVectorLayer *layer)
{
    qDebug() << "[MainWindow]" << "Map Selection Changed: (layer " << layer->name() << ")";
    // 移除旧的橡皮条
    QList<QgsRubberBand*> rubbers0 = mMapLayerRubberDict[layer];
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
        QgsRubberBand* rubber = new QgsRubberBand(mMapCanvas, geometry.type());
        rubber->addGeometry(geometry, layer);
        rubber->setStrokeColor(QColor(255, 0, 0));
        rubber->setFillColor(QColor(255, 0, 0, 144));
        mMapLayerRubberDict[layer] += rubber;
        qDebug() << "[MainWindow::onMapSelectionChanged]"
                 << "selected" << geometry.asWkt();
    }
}

void MainWindow::onMapModelChanged()
{
    mMapLayerList = mMapModel->toMapLayerList();
    mMapCanvas->setLayers(mMapLayerList);
    if (mMapLayerList.size() == 1)
    {
        QgsMapLayer* firstLayer = mMapLayerList.first();
        QgsRectangle extent = mMapCanvas->mapSettings().layerExtentToOutputExtent(firstLayer, firstLayer->extent());
        qDebug() << "[MainWindow::onMapModelChanged]"
                 << "origin extent: " << firstLayer->extent()
                 << "trans extent: " << extent;
        mMapCanvas->setExtent(extent);
    }
    mMapCanvas->refresh();
}

void MainWindow::onShowLayerProperty(const QModelIndex &index)
{
    mPropertyPanel->addPropertyTab(index);
}


void MainWindow::onFeaturePanelRowOrderChanged(int from, int dest)
{
    qDebug() << "[MainWindow::onFeaturePanelRowOrderChanged]"
             << "layer list size" << mMapLayerList.size();
    mMapLayerList = mMapModel->toMapLayerList();
    mMapCanvas->setLayers(mMapLayerList);
    mMapCanvas->refresh();
}

// 属性表
void MainWindow::onShowAttributeTable(const QModelIndex &index)
{
    // qDebug() << 123;
    qDebug() << "[MainWindow::onShowAttributeTable]"
             << "index:" << index;
    GwmLayerItem* item = mMapModel->itemFromIndex(index);
    QgsVectorLayer* currentLayer = mMapModel->layerFromItem(item);
    if (currentLayer)
    {
        GwmAttributeTableDialog *d = new GwmAttributeTableDialog(currentLayer,mMapCanvas,this,Qt::Dialog);
        d->show();
    }
}

void MainWindow::onAttributeTableSelected(QgsVectorLayer* layer, QList<QgsFeatureId> list)
{
    for (QgsFeatureId id : list)
    {
        qDebug() << "[MainWindow::receiveSigAttriToMap]"
                 << "id:" << id;
    }
}
void MainWindow::onShowSymbolSetting(const QModelIndex &index)
{
    createSymbolWindow(index);
    connect(mSymbolWindow,&GwmSymbolWindow::canvasRefreshSingal,this,&MainWindow::refreshCanvas);
    mSymbolWindow->show();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == mMapCanvas)
    {
        switch (e->type()) {
        case QEvent::MouseMove:
            qDebug() << "[MainWindow::eventFilter]"
                     << "Cur map position: (" << mMapCanvas->getCoordinateTransform()->toMapCoordinates(((QMouseEvent*)e)->pos()).asWkt();
            return true;
        default:
            break;
        }
    }
    return false;
}

void MainWindow::refreshCanvas(){
    mMapCanvas->refresh();
}

void MainWindow::createSymbolWindow(const QModelIndex &index)
{
    GwmLayerItem* item = mMapModel->itemFromIndex(index);
    QgsVectorLayer* layer = mMapModel->layerFromItem(item);
    if (layer)
    {
        mSymbolWindow = new GwmSymbolWindow(layer);
    }
}

// 投影到坐标系
void MainWindow::onShowCoordinateTransDlg(const QModelIndex &index)
{
    // qDebug() << index;
    // 获取当前矢量图层
    GwmLayerItem* item = mMapModel->itemFromIndex(index);
    QgsVectorLayer* currentLayer = mMapModel->layerFromItem(item);
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
                qDebug() << "[MainWindow::onShowCoordinateTransDlg]"
                         << "Finished";
                addLayerToModel(thread->getWorkLayer());
            }
        }
    }
}


void MainWindow::onGWRBtnClicked()
{
    GwmBasicGWRAlgorithm* gwrTaskThread = new GwmBasicGWRAlgorithm();
    GwmGWROptionsDialog* gwrOptionDialog = new GwmGWROptionsDialog(mMapModel->rootChildren(), gwrTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
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
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwrTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwrTaskThread->resultLayer();
            GwmLayerBasicGWRItem* gwrItem = new GwmLayerBasicGWRItem(selectedItem, resultLayer, gwrTaskThread);
            mMapModel->appentItem(gwrItem, selectedIndex);
        }
    }
}

void MainWindow::onGWRNewBtnClicked()
{
    GwmBasicGWRAlgorithm* algorithm = new GwmBasicGWRAlgorithm();
    QgsVectorLayer* dataLayer = static_cast<GwmLayerGroupItem*>(mMapModel->rootChildren()[0])->originChild()->layer();
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

void MainWindow::onGWSSBtnClicked()
{
    GwmGWSSTaskThread* gwssTaskThread = new GwmGWSSTaskThread();
    GwmGWSSOptionsDialog* gwssOptionDialog = new GwmGWSSOptionsDialog(mMapModel->rootChildren(), gwssTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
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
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwssTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwssTaskThread->resultLayer();
            GwmLayerGWSSItem* gwssItem = new GwmLayerGWSSItem(selectedItem, resultLayer, gwssTaskThread);
            mMapModel->appentItem(gwssItem, selectedIndex);
        }
    }

}

void MainWindow::onScalableGWRBtnClicked()
{
    GwmScalableGWRAlgorithm* gwrTaskThread = new GwmScalableGWRAlgorithm();
    GwmScalableGWROptionsDialog* gwrOptionDialog = new GwmScalableGWROptionsDialog(mMapModel->rootChildren(), gwrTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
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
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwrTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwrTaskThread->resultLayer();
            GwmLayerScalableGWRItem* gwrItem = new GwmLayerScalableGWRItem(selectedItem, resultLayer, gwrTaskThread);
            mMapModel->appentItem(gwrItem, selectedIndex);
        }
    }
}

void MainWindow::onMultiscaleGWRBtnClicked()
{
    GwmMultiscaleGWRAlgorithm* gwrTaskThread = new GwmMultiscaleGWRAlgorithm();
    GwmMultiscaleGWROptionsDialog* gwrOptionDialog = new GwmMultiscaleGWROptionsDialog(mMapModel->rootChildren(), gwrTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
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
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwrTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwrTaskThread->resultLayer();
            GwmLayerMultiscaleGWRItem* gwrItem = new GwmLayerMultiscaleGWRItem(selectedItem, resultLayer, gwrTaskThread);
            mMapModel->appentItem(gwrItem, selectedIndex);
        }
    }
}

void MainWindow::onRobustGWR()
{
    GwmRobustGWRAlgorithm* gwrRobustTaskThread = new GwmRobustGWRAlgorithm();
    GwmRobustGWROptionsDialog* gwrRobustOptionDialog = new GwmRobustGWROptionsDialog(mMapModel->rootChildren(), gwrRobustTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
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
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwrRobustTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwrRobustTaskThread->resultLayer();
            GwmLayerBasicGWRItem* gwrItem = new GwmLayerBasicGWRItem(selectedItem, resultLayer, gwrRobustTaskThread);
            mMapModel->appentItem(gwrItem, selectedIndex);
        }
    }
}

void MainWindow::onRobustGWRBtnClicked()
{
    GwmRobustGWRAlgorithm* gwrRobustTaskThread = new GwmRobustGWRAlgorithm();
    GwmRobustGWROptionsDialog* gwrRobustOptionDialog = new GwmRobustGWROptionsDialog(mMapModel->rootChildren(), gwrRobustTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
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
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwrRobustTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwrRobustTaskThread->resultLayer();
            GwmLayerBasicGWRItem* gwrItem = new GwmLayerBasicGWRItem(selectedItem, resultLayer, gwrRobustTaskThread);
            mMapModel->appentItem(gwrItem, selectedIndex);
        }
    }
}

void MainWindow::onLcrGWRBtnClicked()
{
    GwmLocalCollinearityGWRAlgorithm * lcrGWRTaskThread = new GwmLocalCollinearityGWRAlgorithm();
    GwmLcrGWROptionsDialog* gwrLcrOptionDialog = new GwmLcrGWROptionsDialog(mMapModel->rootChildren(), lcrGWRTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
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
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(lcrGWRTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = lcrGWRTaskThread->resultLayer();
            GwmLayerCollinearityGWRItem* gwrItem = new GwmLayerCollinearityGWRItem(selectedItem, resultLayer, lcrGWRTaskThread);
            mMapModel->appentItem(gwrItem, selectedIndex);
        }
    }
}

void MainWindow::onGGWRBtnClicked(){
    GwmGeneralizedGWRAlgorithm* ggwrTaskThread = new GwmGeneralizedGWRAlgorithm();
    GwmGGWROptionsDialog* ggwrOptionDialog = new GwmGGWROptionsDialog(this->mMapModel->rootChildren(), ggwrTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
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
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(ggwrTaskThread); //
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = ggwrTaskThread->resultLayer();
            GwmLayerGGWRItem* ggwrItem = new GwmLayerGGWRItem(selectedItem, resultLayer, ggwrTaskThread);
            mMapModel->appentItem(ggwrItem, selectedIndex);
        }
    }
}

void MainWindow::onGWPCABtnClicked()
{
    GwmGWPCATaskThread* gwpcaTaskThread = new GwmGWPCATaskThread();
    GwmGWPCAOptionsDialog* gwpcaOptionDialog = new GwmGWPCAOptionsDialog(mMapModel->rootChildren(), gwpcaTaskThread);
    QModelIndexList selectedIndexes = mFeaturePanel->selectionModel()->selectedIndexes();
    for (QModelIndex selectedIndex : selectedIndexes)
    {
        GwmLayerItem* selectedItem = mMapModel->itemFromIndex(selectedIndex);
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
        const QModelIndex selectedIndex = mMapModel->indexFromItem(selectedItem);
        GwmProgressDialog* progressDlg = new GwmProgressDialog(gwpcaTaskThread);
        if (progressDlg->exec() == QDialog::Accepted)
        {
            QgsVectorLayer* resultLayer = gwpcaTaskThread->resultLayer();
            GwmLayerGWPCAItem * gwrItem = new GwmLayerGWPCAItem(selectedItem, resultLayer, gwpcaTaskThread);
            mMapModel->appentItem(gwrItem, selectedIndex);
        }
    }
}

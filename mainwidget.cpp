#include <QtWidgets>
#include <QFileInfo>
#include <qgsvectorlayer.h>
#include "mainwidget.h"

#include "gwmodelmappanel.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , mainLayout(new QVBoxLayout)
    , toolBar(new GWmodelToolbar)
{
    mapModel = new QStandardItemModel();

    createMainZone();
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(mainZone);
    setLayout(mainLayout);
    mainLayout->setStretchFactor(mainZone, 1);

    connect(toolBar, &GWmodelToolbar::openFileImportShapefileSignal, this, &MainWidget::openFileImportShapefile);
    connect(toolBar, &GWmodelToolbar::openFileImportJsonSignal, this, &MainWidget::openFileImportJson);
    connect(toolBar, &GWmodelToolbar::openFileImportCsvSignal, this, &MainWidget::openFileImportCsv);
    connect(toolBar, &GWmodelToolbar::openByXYBtnSingnal, this, &MainWidget::openFileImportCsv);
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
    mapPanel = new GWmodelMapPanel(mainZone, mapModel);

    layout->addWidget(featurePanel);
    layout->addWidget(mapPanel);
    layout->addWidget(propertyPanel);

    layout->setStretchFactor(mapPanel, 1);
}

void MainWidget::createFeaturePanel()
{
    featurePanel = new QTreeView(mainZone);
    featurePanel->setColumnWidth(0, 320);
    QStringList headerLabels = QStringList() << tr("Features");
    mapModel->setHorizontalHeaderLabels(headerLabels);
    featurePanel->setModel(mapModel);
    // 设置右键菜单
    featurePanel->setContextMenuPolicy(Qt::CustomContextMenu);
    //connect(featurePanel, &QTreeView::clicked, this, &MainWidget::ShowContextMenu);
    // 建立信号槽的连接
    connect(featurePanel, SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(showContextMenu(const QPoint&)));
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

// 官网示例(支持左键事件)
void MainWidget::ShowContext(const QModelIndex &index)
{
    QStandardItem *item = mapModel->itemFromIndex(index);
    qDebug() << index;
}
// 发出信号
void MainWidget::customContextMenuRequested(const QPoint &pos)
{
    emit showContextMenu(pos);
}
// 槽函数
void MainWidget::showContextMenu(const QPoint &pos)
{
    // 获取要素区列表索引值
    QModelIndex index = featurePanel->indexAt(pos);
    // qDebug() << index;
    if (index.isValid())
    {
        QMenu *menu = new QMenu(this);
        QAction *pShow = new QAction("显示",this);
        menu->addAction(pShow);
        // 处理事件
        connect(pShow, SIGNAL(triggered()),this, SLOT(showLayer()));

        // 改为"五个字的 缩放至图层"会报错, 原因未知
        QAction *pZoom = new QAction("缩放图层",this);
        menu->addAction(pZoom);

        QAction *pAttribute = new QAction("属性表",this);
//        pAttribute->setCheckable(true);
//        pAttribute->setChecked(true);
        menu->addAction(pAttribute);

        QAction *pProj = new QAction("投影到坐标系",this);
        menu->addAction(pProj);

        QAction *pSymbol = new QAction("符号",this);
        menu->addAction(pSymbol);
        // 导出是二级菜单
        QAction *pExport = new QAction("导出",this);
        // menu->addAction("导出");
        // 二级菜单制作
        QMenu *subMenu = new QMenu(this);
        QAction *pESRI = new QAction("ESRI Shapefile",subMenu);
        subMenu->addAction(pESRI);

        QAction *pGeo = new QAction("GeoJSON",subMenu);
        subMenu->addAction(pGeo);

        QAction *pCsv = new QAction("csv",subMenu);
        subMenu->addAction(pGeo);

        QAction *pXls = new QAction("Excel",subMenu);
        subMenu->addAction(pXls);
        // 设置二级菜单
        pExport->setMenu(subMenu);
        menu->addMenu(subMenu);
        // QCursor::pos()让menu的位置在鼠标点击的的位置
        menu->exec(QCursor::pos());
    }
}
// 显示图层信号
//void MainWidget::showLayerSig(const QModelIndex &index)
//{
//    emit showLayer(index);
//}
// 显示图层
void MainWidget::showLayer()
{
    QModelIndexList selected = featurePanel->selectionModel()->selectedIndexes();
    qDebug() << selected[0];
}



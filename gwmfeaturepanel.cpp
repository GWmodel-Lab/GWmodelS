#include "gwmfeaturepanel.h"
#include "QVBoxLayout"
#include "QMenu"

GwmFeaturePanel::GwmFeaturePanel(QWidget *parent, QStandardItemModel* model) :
    QWidget(parent),
    mapModel(model)
{
    setupUi();
}

GwmFeaturePanel::~GwmFeaturePanel()
{
}

void GwmFeaturePanel::setupUi()
{
    mapModel->setHorizontalHeaderLabels(QStringList() << tr("Features"));
    // 创建 Feature Panel
    featurePanel = new QTreeView(this);
    featurePanel->setModel(mapModel);
    featurePanel->setColumnWidth(0, 315);
    featurePanel->setModel(mapModel);
    // 添加到布局中
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(featurePanel);
    setLayout(layout);
    // 设置上下文菜单
    featurePanel->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(featurePanel, &QTreeView::customContextMenuRequested, this, &GwmFeaturePanel::showContextMenu);
}

//void GwmFeaturePanel::customContextMenuRequested(const QPoint &pos)
//{
//    emit showContextMenu(pos);
//}

void GwmFeaturePanel::showContextMenu(const QPoint &pos)
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
        connect(pShow, &QAction::triggered, this, &GwmFeaturePanel::showLayer);

        // 改为"五个字的 缩放至图层"会报错, 原因未知
        QAction *pZoom = new QAction("缩放图层",this);
        menu->addAction(pZoom);
        // 处理事件
        connect(pZoom, &QAction::triggered, this, &GwmFeaturePanel::zoomLayer);

        QAction *pAttribute = new QAction("属性表",this);
        menu->addAction(pAttribute);
        connect(pAttribute, &QAction::triggered,this,&GwmFeaturePanel::attributeTable);

        QAction *pProj = new QAction("投影到坐标系",this);
        menu->addAction(pProj);
        connect(pProj, &QAction::triggered,this,&GwmFeaturePanel::proj);

        QAction *pSymbol = new QAction("符号",this);
        menu->addAction(pSymbol);
        connect(pSymbol, &QAction::triggered,this, &GwmFeaturePanel::symbol);
        // 导出是二级菜单
        QAction *pExport = new QAction("导出",this);
        // menu->addAction("导出");
        // 二级菜单制作
        QMenu *subMenu = new QMenu(this);
        QAction *pESRI = new QAction("ESRI Shapefile",subMenu);
        subMenu->addAction(pESRI);
        connect(pESRI, &QAction::triggered,this,&GwmFeaturePanel::esrishp);

        QAction *pGeo = new QAction("GeoJSON",subMenu);
        subMenu->addAction(pGeo);
        connect(pGeo, &QAction::triggered,this,&GwmFeaturePanel::geojson);

        QAction *pCsv = new QAction("csv",subMenu);
        subMenu->addAction(pCsv);
        connect(pCsv, &QAction::triggered,this,&GwmFeaturePanel::csv);

        QAction *pXls = new QAction("Excel",subMenu);
        subMenu->addAction(pXls);
        connect(pXls, &QAction::triggered,this,&GwmFeaturePanel::excel);
        // 设置二级菜单
        pExport->setMenu(subMenu);
        menu->addMenu(subMenu);
        // QCursor::pos()让menu的位置在鼠标点击的的位置
        menu->exec(QCursor::pos());
    }
}

// 显示图层
void GwmFeaturePanel::showLayer()
{
    QModelIndexList selected = featurePanel->selectionModel()->selectedIndexes();
    //qDebug() << selected[0];
    emit sendDataSigShowLayer(selected[0]);
}

// 缩放至图层
void GwmFeaturePanel::zoomLayer()
{
    QModelIndexList selected = featurePanel->selectionModel()->selectedIndexes();
    //qDebug() << selected[0];
    emit sendDataSigZoomLayer(selected[0]);
}

// 属性表
void GwmFeaturePanel::attributeTable()
{
    QModelIndexList selected = featurePanel->selectionModel()->selectedIndexes();
    emit sendDataSigAttributeTable(selected[0]);
}

// 投影到坐标系
void GwmFeaturePanel::proj()
{
    QModelIndexList selected = featurePanel->selectionModel()->selectedIndexes();
    emit sendDataSigProj(selected[0]);
}

// 符号
void GwmFeaturePanel::symbol()
{
    QModelIndexList selected = featurePanel->selectionModel()->selectedIndexes();
    emit sendDataSigSymbol(selected[0]);
}

// 导出shp
void GwmFeaturePanel::esrishp()
{
    QModelIndexList selected = featurePanel->selectionModel()->selectedIndexes();
    emit sendDataSigEsriShp(selected[0]);
}

// 导出GeoJSON
void GwmFeaturePanel::geojson()
{
    QModelIndexList selected = featurePanel->selectionModel()->selectedIndexes();
    emit sendDataSigGeoJson(selected[0]);
}

// 导出Csv
void GwmFeaturePanel::csv()
{
    QModelIndexList selected = featurePanel->selectionModel()->selectedIndexes();
    emit sendDataSigCsv(selected[0]);
}

// 导出Excel
void GwmFeaturePanel::excel()
{
    QModelIndexList selected = featurePanel->selectionModel()->selectedIndexes();
    emit sendDataSigExcel(selected[0]);
}

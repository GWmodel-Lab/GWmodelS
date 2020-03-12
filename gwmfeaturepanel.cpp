#include "gwmfeaturepanel.h"
#include "QVBoxLayout"
#include "QMenu"

GwmFeaturePanel::GwmFeaturePanel(QWidget *parent, QStandardItemModel* model) :
    QTreeView(parent),
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
    this->setModel(mapModel);
    this->setColumnWidth(0, 315);
    this->setModel(mapModel);
    // 设置上下文菜单
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeView::customContextMenuRequested, this, &GwmFeaturePanel::showContextMenu);
}

//void GwmFeaturePanel::customContextMenuRequested(const QPoint &pos)
//{
//    emit showContextMenu(pos);
//}

void GwmFeaturePanel::showContextMenu(const QPoint &pos)
{
    // 获取要素区列表索引值
    QModelIndex index = this->indexAt(pos);
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

        // 显示属性
        QAction *pProperty = new QAction("Layer Property",this);
        menu->addAction(pProperty);
        connect(pProperty, &QAction::triggered,this, &GwmFeaturePanel::layerProperty);

        // QCursor::pos()让menu的位置在鼠标点击的的位置
        menu->addMenu(subMenu);
        menu->exec(QCursor::pos());
    }
}

// 显示图层
void GwmFeaturePanel::showLayer()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    //qDebug() << selected[0];
    emit sendDataSigShowLayer(selected[0]);
}

// 缩放至图层
void GwmFeaturePanel::zoomLayer()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    //qDebug() << selected[0];
    emit sendDataSigZoomLayer(selected[0]);
}

// 属性表
void GwmFeaturePanel::attributeTable()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    emit sendDataSigAttributeTable(selected[0]);
}

// 投影到坐标系
void GwmFeaturePanel::proj()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    emit sendDataSigProj(selected[0]);
}

// 符号
void GwmFeaturePanel::symbol()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    emit sendDataSigSymbol(selected[0]);
}

// 导出shp
void GwmFeaturePanel::esrishp()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    emit sendDataSigEsriShp(selected[0]);
}

// 导出GeoJSON
void GwmFeaturePanel::geojson()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    emit sendDataSigGeoJson(selected[0]);
}

// 导出Csv
void GwmFeaturePanel::csv()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    emit sendDataSigCsv(selected[0]);
}

// 导出Excel
void GwmFeaturePanel::excel()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    emit sendDataSigExcel(selected[0]);
}

void GwmFeaturePanel::layerProperty()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    emit showLayerPropertySignal(selected[0]);
}

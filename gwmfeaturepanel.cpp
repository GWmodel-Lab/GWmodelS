#include "gwmfeaturepanel.h"
#include "QApplication"
#include "QVBoxLayout"
#include "QDragEnterEvent"
#include "QMenu"
#include "QDebug"
#include "QMimeData"
#include "QDrag"
#include "QHeaderView"

GwmFeaturePanel::GwmFeaturePanel(QWidget *parent, QStandardItemModel* model) :
    QTreeView(parent),
    mMapModel(model)
{
    setupUi();
}

GwmFeaturePanel::~GwmFeaturePanel()
{
}

void GwmFeaturePanel::setupUi()
{
    mMapModel->setHorizontalHeaderLabels(QStringList() << tr("Features"));
    // 创建 Feature Panel
    this->setModel(mMapModel);
    this->setColumnWidth(0, 315);
    this->setModel(mMapModel);
    this->setDragEnabled(true);
    this->setAcceptDrops(true);
    // 设置上下文菜单
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeView::customContextMenuRequested, this, &GwmFeaturePanel::showContextMenu);
    // 设置拖动指示器
    mIndicator = new QLabel(this);
    mIndicator->setFixedHeight(2);
    mIndicator->setGeometry(1, 0, width(), 2);
    mIndicator->setStyleSheet("border: 1px solid #CCCCCC;");
    mIndicator->hide();
}

//void GwmFeaturePanel::customContextMenuRequested(const QPoint &pos)
//{
//    emit showContextMenu(pos);
//}

void GwmFeaturePanel::showContextMenu(const QPoint &pos)
{
    // 获取要素区列表索引值
    QModelIndex index = this->indexAt(pos);
    QStandardItem* item = mMapModel->itemFromIndex(index);
    // qDebug() << index;
    if (index.isValid())
    {
        QMenu *menu = new QMenu(this);

        // 显示/隐藏
        QAction *pShow = new QAction(tr("Show/Hide"),this);
        menu->addAction(pShow);
        pShow->setCheckable(true);
        pShow->setChecked(item->checkState() == Qt::CheckState::Checked);
        connect(pShow, &QAction::triggered, this, &GwmFeaturePanel::showLayer);

        QAction *pRemove = new QAction(tr("Remove"), this);
        menu->addAction(pRemove);
        connect(pRemove, &QAction::triggered, this, &GwmFeaturePanel::removeLayer);

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
        QAction *pProperty = new QAction(tr("Layer Property"),this);
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
    for (QModelIndex index : selected) {
        QStandardItem* item = mMapModel->itemFromIndex(index);
        Qt::CheckState checkState = item->checkState();
        switch (checkState) {
        case Qt::CheckState::Checked:
            item->setCheckState(Qt::CheckState::Unchecked);
            break;
        case Qt::CheckState::Unchecked:
            item->setCheckState(Qt::CheckState::Checked);
            break;
        default:
            break;
        }
    }
}

void GwmFeaturePanel::removeLayer()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    emit removeLayerSignal(selected[0]);
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

int GwmFeaturePanel::sumRowHeight(QModelIndex index)
{
    qDebug() << "GwmFeaturePanel::sumRowHeight"
             << "index isvalid" << index.isValid();
    int height = 0;
    int nCountRow = index.isValid() ? index.row() : mMapModel->rowCount();
    for (int i = 0; i < nCountRow; ++i)
    {
        QStandardItem* item = mMapModel->item(i);
        height += rowHeight(mMapModel->indexFromItem(item));
    }
    return height;
}

void GwmFeaturePanel::mousePressEvent(QMouseEvent *e)
{
    qDebug() << "[GwmFeaturePanel - mousePressEvent]";
    if (e->button() == Qt::LeftButton)
    {
        QModelIndex index = indexAt(e->pos());
        if (index.isValid())
        {
            mValidPress = true;
            mDragPoint = e->pos();
            mDragText = mMapModel->item(index.row())->text();
            mDragPointAtItem = mDragPoint - QPoint(0, sumRowHeight(index));
            mRowFrom = index.row();
        }
    }
    QTreeView::mousePressEvent(e);
}

void GwmFeaturePanel::mouseMoveEvent(QMouseEvent *e)
{
//    qDebug() << "[GwmFeaturePanel - mouseMoveEvent]";
    if (mValidPress)
    {
        if (e->buttons() & Qt::LeftButton)
        {
            if ((e->pos() - mDragPoint).manhattanLength() < QApplication::startDragDistance())
            {
                mIndicator->show();
                this->doDrag();
                mIndicator->hide();
                mValidPress = false;
            }
        }
    }
}

void GwmFeaturePanel::dragEnterEvent(QDragEnterEvent *e)
{
    qDebug() << "[GwmFeaturePanel - dragEnterEvent]";
    if (e->mimeData()->hasText())
    {
        e->acceptProposedAction();
        emit beginDragDropSignal();
    }
    else
    {
        e->ignore();
    }

}

void GwmFeaturePanel::dragMoveEvent(QDragMoveEvent *e)
{
    if (e->mimeData()->hasText())
    {
        int nCurRow = 0;
        QModelIndex index = indexAt(e->pos());
        if (index.isValid())
        {
            int rowHeightPre = sumRowHeight(index);
            int rowHeightCur = rowHeight(index);
            if (e->pos().y() - rowHeightPre > rowHeightCur/2)
                nCurRow = index.row() + 1;
            else nCurRow = index.row();
            qDebug() << "[GwmFeaturePanel - dragMoveEvent]"
                     << "fromRow:" << mRowFrom
                     << "nCurRow:" << nCurRow
                     << QString("rowHeightPre %1 rowHeightCur %2").arg(rowHeightPre).arg(rowHeightCur)
                     << "pos.y" << e->pos().y();
        }
        else
        {
            nCurRow = mMapModel->rowCount();
        }
        if (true)
        {
            mRowDest = nCurRow;
            int labelY = sumRowHeight(mMapModel->indexFromItem(mMapModel->item(mRowDest)))
                    + header()->height();
            qDebug() << "[GwmFeaturePanel - dragMoveEvent]"
                     << "labelY" << labelY;
            mIndicator->setGeometry(1, labelY, width(), 2);
        }
        e->acceptProposedAction();
    }
    else
    {
        e->ignore();
    }
}

void GwmFeaturePanel::dropEvent(QDropEvent *e)
{
    if(e->mimeData()->hasText())
    {
        if (mRowDest != mRowFrom)
        {
            qDebug() << "[GwmFeaturePanel - dropEvent] from" << mRowFrom << "dest" << mRowDest;
            doMoveRow(mRowFrom, mRowDest);
        }
        e->acceptProposedAction();
    }
    else
    {
        e->ignore();
    }
    emit endDragDropSignal();
}

void GwmFeaturePanel::doDrag()
{
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;
    mimeData->setText(mDragText);
    drag->setMimeData(mimeData);
    if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
    {
        qDebug() << "[GwmFeaturePanel - doDrag]" << QString("Move row %1 to %2").arg(mRowFrom).arg(mRowDest);
    }
}

void GwmFeaturePanel::resetOrder()
{
    qDebug() << "[GwmFeaturePanel - resetOrder]";
}

void GwmFeaturePanel::doMoveRow(int from, int dest)
{
    qDebug() << "[GwmFeaturePanel - doMoveRow]"
             << "remove item" << (from < dest ? from : from + 1);
    if (from != dest)
    {
        QStandardItem* item = mMapModel->takeItem(from);
        qDebug() << "[GwmFeaturePanel - doMoveRow]"
                 << "taken itemCount" << mMapModel->rowCount();
        if (item)
        {
            mMapModel->insertRow(dest, item);
            qDebug() << "[GwmFeaturePanel - doMoveRow]"
                     << "inserted itemCount" << mMapModel->rowCount();
            mMapModel->removeRow(from < dest ? from : from + 1);
            qDebug() << "[GwmFeaturePanel - doMoveRow]"
                     << "removed itemCount" << mMapModel->rowCount();
        }
        emit rowOrderChangedSignal(from, dest);
    }
}

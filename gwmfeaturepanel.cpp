#include "gwmfeaturepanel.h"
#include "QApplication"
#include "QVBoxLayout"
#include "QDragEnterEvent"
#include "QMenu"
#include "QDebug"
#include "QMimeData"
#include "QDrag"
#include "QHeaderView"


GwmFeaturePanel::GwmFeaturePanel(QWidget *parent)
    : QTreeView(parent)
    , mMapModel(new GwmLayerItemModel)
    , isMapModelSetted(false)
{
    setupUi();
    setHeaderHidden(true);
}

GwmFeaturePanel::~GwmFeaturePanel()
{
}

GwmLayerItemModel *GwmFeaturePanel::mapModel() const
{
    return mMapModel;
}

void GwmFeaturePanel::setMapModel(GwmLayerItemModel *mapModel)
{
    if (!isMapModelSetted)
        delete mMapModel;
    mMapModel = mapModel;
    isMapModelSetted = true;
    this->setModel(mMapModel);
}

void GwmFeaturePanel::onSortUpBtnClicked()
{
    QModelIndex selected = this->selectionModel()->selectedIndexes().first();
    mMapModel->moveUp(selected);
}

void GwmFeaturePanel::onSortDownBtnClicked()
{
    QModelIndex selected = this->selectionModel()->selectedIndexes().first();
    mMapModel->moveDown(selected);
}

void GwmFeaturePanel::setupUi()
{
    //    mMapModel->setHorizontalHeaderLabels(QStringList() << tr("Features"));
    // 创建 Feature Panel
    this->setColumnWidth(0, 315);
    this->setModel(mMapModel);
    this->setDragEnabled(true);
    this->setAcceptDrops(true);
    // 设置上下文菜单
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeView::customContextMenuRequested, this, &GwmFeaturePanel::showContextMenu);
    // 设置拖动指示器
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
        GwmLayerItem* item = (mMapModel->itemFromIndex(index));
        if (item->itemType() == GwmLayerItem::Symbol)
            return;

        QMenu *menu = new QMenu(this);
        // 显示/隐藏
        QAction *pShow = new QAction(tr("Show/Hide"),this);
        menu->addAction(pShow);
        pShow->setCheckable(true);
        pShow->setChecked(item->checkState() == Qt::CheckState::Checked);
        connect(pShow, &QAction::triggered, this, &GwmFeaturePanel::showLayer);

        if (!(item->itemType() == GwmLayerItem::Origin))
        {
            QAction *pRemove = new QAction(tr("Remove"), this);
            pRemove->setIcon(QIcon(QStringLiteral(":/images/themes/default/mActionRemoveLayer.svg")));
            pRemove->setEnabled(mMapModel->canRemove(index));
            menu->addAction(pRemove);
            connect(pRemove, &QAction::triggered, this, &GwmFeaturePanel::removeLayer);

            // 上下移动
            QAction *pMoveUp = new QAction(tr("Move Up"), this);
            pMoveUp->setIcon(QIcon(QStringLiteral(":/images/themes/default/mActionCollapseTree.svg")));
            pMoveUp->setEnabled(mMapModel->canMoveUp(index));
            menu->addAction(pMoveUp);
            connect(pMoveUp, &QAction::triggered, this, &GwmFeaturePanel::onSortUpBtnClicked);

            QAction *pMoveDown = new QAction(tr("Move Down"), this);
            pMoveDown->setIcon(QIcon(QStringLiteral(":/images/themes/default/mActionExpandTree.svg")));
            pMoveDown->setEnabled(mMapModel->canMoveDown(index));
            menu->addAction(pMoveDown);
            connect(pMoveDown, &QAction::triggered, this, &GwmFeaturePanel::onSortUpBtnClicked);
        }

        // 缩放到图层
        QAction *pZoom = new QAction(tr("Zoom to this layer"),this);
        pZoom->setIcon(QIcon(QStringLiteral(":/images/themes/default/mActionZoomToLayer.svg")));
        menu->addAction(pZoom);
        connect(pZoom, &QAction::triggered, this, &GwmFeaturePanel::zoomLayer);

        QAction *pAttribute = new QAction(tr("Attribute Table"),this);
        menu->addAction(pAttribute);
        connect(pAttribute, &QAction::triggered,this,&GwmFeaturePanel::attributeTable);

        if (item->itemType() == GwmLayerItem::Group || item->itemType() == GwmLayerItem::Origin)
        {
            QAction *pProj = new QAction(tr("Reproject"),this);
            menu->addAction(pProj);
            connect(pProj, &QAction::triggered,this,&GwmFeaturePanel::proj);
        }

        QAction *pSymbol = new QAction(tr("Symbology"),this);
        pSymbol->setIcon(QIcon(QStringLiteral(":/images/themes/default/propertyicons/symbology.svg")));
        menu->addAction(pSymbol);
        connect(pSymbol, &QAction::triggered,this, &GwmFeaturePanel::symbol);

        // 导出是二级菜单
        QAction *pExport = new QAction(tr("Export"),this);
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

//        QAction *pXls = new QAction("Excel",subMenu);
//        subMenu->addAction(pXls);
//        connect(pXls, &QAction::triggered,this,&GwmFeaturePanel::excel);
        // 设置二级菜单
        pExport->setMenu(subMenu);

        // 显示属性
        QAction *pProperty = new QAction(tr("Property"),this);
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
        GwmLayerItem* item = mMapModel->itemFromIndex(index);
        qDebug() << "[GwmFeaturePanel::showLayer]"
                 << "index" << "(" << index.row() << "," << index.column() << "," << item->itemType() << ")"
                 << "parent" << "(" << index.parent().row() << "," << index.parent().column() << ")";
        Qt::CheckState checkState = item->checkState();
        switch (checkState) {
        case Qt::CheckState::Checked:
            mMapModel->setData(index, Qt::CheckState::Unchecked, Qt::CheckStateRole);
            break;
        case Qt::CheckState::Unchecked:
            mMapModel->setData(index, Qt::CheckState::Checked, Qt::CheckStateRole);
            break;
        default:
            break;
        }
    }
}

void GwmFeaturePanel::mousePressEvent(QMouseEvent *event)
{
    if( event->button() == Qt::LeftButton ){
//        QModelIndex lastindex = this->currentIndex();
        QModelIndex index = indexAt(event->pos());  //取出按下点的元素索引index
        if( !index.isValid() )                   //判断index是否有效
        {
            this->clearSelection();
            this->setCurrentIndex(index);   //设置CurrentIndex为空
//            emit currentChanged(index,lastindex);
        }
    }
    return QTreeView::mousePressEvent(event);
}

void GwmFeaturePanel::removeLayer()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    for (QModelIndex& index : selected)
    {
        mMapModel->removeRow(index.row(), index.parent());
    }
}

// 缩放至图层
void GwmFeaturePanel::zoomLayer()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    for (QModelIndex index : selected)
    {
        emit zoomToLayerSignal(index);
    }
}

// 属性表
void GwmFeaturePanel::attributeTable()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    emit showAttributeTableSignal(selected[0]);
}

// 投影到坐标系
void GwmFeaturePanel::proj()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    emit showCoordinateTransDlg(selected[0]);
}

// 符号
void GwmFeaturePanel::symbol()
{
    QModelIndexList selected = this->selectionModel()->selectedIndexes();
    emit showSymbolSettingSignal(selected[0]);
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

//int GwmFeaturePanel::sumRowHeight(QModelIndex index)
//{
//    qDebug() << "GwmFeaturePanel::sumRowHeight"
//             << "index isvalid" << index.isValid();
//    int height = 0;
//    int nCountRow = index.isValid() ? index.row() : mMapModel->rowCount();
//    for (int i = 0; i < nCountRow; ++i)
//    {
//        GwmLayerItem* item = mMapModel->item(i);
//        height += rowHeight(mMapModel->indexFromItem(item));
//    }
//    return height;
//}

//void GwmFeaturePanel::mousePressEvent(QMouseEvent *e)
//{
//    qDebug() << "[GwmFeaturePanel - mousePressEvent]";
//    if (e->button() == Qt::LeftButton)
//    {
//        QModelIndex index = indexAt(e->pos());
//        if (index.isValid())
//        {
//            mValidPress = true;
//            mDragPoint = e->pos();
//            mDragText = mMapModel->itemFromIndex(index)->text();
//            mDragPointAtItem = mDragPoint - QPoint(0, sumRowHeight(index));
//            mRowFrom = index.row();
//        }
//    }
//    QTreeView::mousePressEvent(e);
//}

//void GwmFeaturePanel::mouseMoveEvent(QMouseEvent *e)
//{
//    qDebug() << "[GwmFeaturePanel - mouseMoveEvent]";
//    if (mValidPress)
//    {
//        if (e->buttons() & Qt::LeftButton)
//        {
//            if ((e->pos() - mDragPoint).manhattanLength() < QApplication::startDragDistance())
//            {
//                mIndicator->show();
//                this->doDrag();
//                mIndicator->hide();
//                mValidPress = false;
//            }
//        }
//    }
//}

//void GwmFeaturePanel::dragEnterEvent(QDragEnterEvent *e)
//{
//    qDebug() << "[GwmFeaturePanel - dragEnterEvent]";
//    if (e->mimeData()->hasText())
//    {
//        e->acceptProposedAction();
//        emit beginDragDropSignal();
//    }
//    else
//    {
//        e->ignore();
//    }

//}

//void GwmFeaturePanel::dragMoveEvent(QDragMoveEvent *e)
//{
//    if (e->mimeData()->hasText())
//    {
//        int nCurRow = 0;
//        QModelIndex index = indexAt(e->pos());
//        if (index.isValid())
//        {
//            int rowHeightPre = sumRowHeight(index);
//            int rowHeightCur = rowHeight(index);
//            if (e->pos().y() - rowHeightPre > rowHeightCur/2)
//                nCurRow = index.row() + 1;
//            else nCurRow = index.row();
//            qDebug() << "[GwmFeaturePanel - dragMoveEvent]"
//                     << "fromRow:" << mRowFrom
//                     << "nCurRow:" << nCurRow
//                     << QString("rowHeightPre %1 rowHeightCur %2").arg(rowHeightPre).arg(rowHeightCur)
//                     << "pos.y" << e->pos().y();
//        }
//        else
//        {
//            nCurRow = mMapModel->rowCount();
//        }
//        if (true)
//        {
//            mRowDest = nCurRow;
//            int labelY = sumRowHeight(mMapModel->indexFromItem(mMapModel->item(mRowDest)))
//                    + header()->height();
//            qDebug() << "[GwmFeaturePanel - dragMoveEvent]"
//                     << "labelY" << labelY;
//            mIndicator->setGeometry(1, labelY, width(), 2);
//        }
//        e->acceptProposedAction();
//    }
//    else
//    {
//        e->ignore();
//    }
//}

//void GwmFeaturePanel::dropEvent(QDropEvent *e)
//{
//    if(e->mimeData()->hasText())
//    {
//        if (mRowDest != mRowFrom)
//        {
//            qDebug() << "[GwmFeaturePanel - dropEvent] from" << mRowFrom << "dest" << mRowDest;
//            doMoveRow(mRowFrom, mRowDest);
//        }
//        e->acceptProposedAction();
//    }
//    else
//    {
//        e->ignore();
//    }
//    emit endDragDropSignal();
//}

//void GwmFeaturePanel::doDrag()
//{
//    QDrag* drag = new QDrag(this);
//    QMimeData* mimeData = new QMimeData;
//    mimeData->setText(mDragText);
//    drag->setMimeData(mimeData);
//    if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
//    {
//        qDebug() << "[GwmFeaturePanel - doDrag]" << QString("Move row %1 to %2").arg(mRowFrom).arg(mRowDest);
//    }
//}

//void GwmFeaturePanel::resetOrder()
//{
//    qDebug() << "[GwmFeaturePanel - resetOrder]";
//}

//void GwmFeaturePanel::doMoveRow(int from, int dest)
//{
//    qDebug() << "[GwmFeaturePanel - doMoveRow]"
//             << "remove item" << (from < dest ? from : from + 1);
//    if (from != dest)
//    {
//        QStandardItem* item = mMapModel->takeItem(from);
//        qDebug() << "[GwmFeaturePanel - doMoveRow]"
//                 << "taken itemCount" << mMapModel->rowCount();
//        if (item)
//        {
//            mMapModel->insertRow(dest, item);
//            qDebug() << "[GwmFeaturePanel - doMoveRow]"
//                     << "inserted itemCount" << mMapModel->rowCount();
//            mMapModel->removeRow(from < dest ? from : from + 1);
//            qDebug() << "[GwmFeaturePanel - doMoveRow]"
//                     << "removed itemCount" << mMapModel->rowCount();
//        }
//        emit rowOrderChangedSignal(from, dest);
//    }
//}

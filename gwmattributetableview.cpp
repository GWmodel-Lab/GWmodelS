#include "gwmattributetableview.h"
#include "qgsattributetableview.h"
#include <QEvent>
#include <QDebug>
#include <qgsvectorlayercache.h>
#include <qgsattributetablemodel.h>
#include <qgsattributetablefiltermodel.h>

GwmAttributeTableView::GwmAttributeTableView(QWidget *parent)
    : QgsAttributeTableView(parent)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
}

GwmAttributeTableView::~GwmAttributeTableView()
{
    qDebug() << "[GwmAttributeTableView::~GwmAttributeTableView]";
    delete mFilterTableModel;
    delete mTableModel;
    delete mLayerCache;
}

//bool GwmAttributeTableView::eventFilter(QObject *object, QEvent *event)
//{
//    switch (event->type())
//    {
//    case QEvent::MouseButtonRelease:
//    case QEvent::Drop:
//        emit attributeTableSelectedSignal(this->mLayer, this->selectedFeaturesIds());
//        break;
//    default:
//        break;
//    }
//    return QgsAttributeTableView::eventFilter(object, event);
//}

void GwmAttributeTableView::setDisplayMapLayer(QgsMapCanvas* canvas, QgsVectorLayer *layer)
{
    this->mMapCanvas = canvas;
    this->mLayer = layer;
    this->mLayerCache = new QgsVectorLayerCache(layer, layer->featureCount());
    this->mTableModel = new QgsAttributeTableModel( mLayerCache );
    this->mTableModel->loadLayer();
    mFilterTableModel = new QgsAttributeTableFilterModel( this->mMapCanvas, mTableModel, mTableModel );
    this->setModel(mFilterTableModel);
}

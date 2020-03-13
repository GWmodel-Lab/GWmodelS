#ifndef QGSATTRIBUTETABLEVIEWEXTEND_H
#define QGSATTRIBUTETABLEVIEWEXTEND_H

#include "prefix.h"
#include "qgsattributetableview.h"
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>

class GwmAttributeTableView: public QgsAttributeTableView
{
    Q_OBJECT

public:
    explicit GwmAttributeTableView(QgsAttributeTableView *parent = nullptr);
    ~GwmAttributeTableView();
//    virtual bool eventFilter( QObject* object, QEvent* event );

signals:
    // 发送信号给地图区主窗口
    void attributeTableSelectedSignal(QgsVectorLayer* layer, QList<QgsFeatureId> list);

private:
    QgsMapCanvas* mMapCanvas;
    QgsVectorLayer* mLayer;
    QgsVectorLayerCache* mLayerCache;
    QgsAttributeTableModel* mTableModel;
    QgsAttributeTableFilterModel* mFilterTableModel;

public:
    void setDisplayMapLayer(QgsMapCanvas* canvas, QgsVectorLayer* layer);
};

#endif // QGSATTRIBUTETABLEVIEWEXTEND_H

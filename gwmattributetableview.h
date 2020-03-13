#ifndef QGSATTRIBUTETABLEVIEWEXTEND_H
#define QGSATTRIBUTETABLEVIEWEXTEND_H

#include "qgsattributetableview.h"

class GwmAttributeTableView: public QgsAttributeTableView
{
    Q_OBJECT

public:
    explicit GwmAttributeTableView(QgsAttributeTableView *parent = nullptr);
    ~GwmAttributeTableView();
    virtual bool eventFilter( QObject* object, QEvent* event );
signals:
    // 发送信号给地图区主窗口
    void sendSigAttriToMap(QList<QgsFeatureId> list);
};

#endif // QGSATTRIBUTETABLEVIEWEXTEND_H

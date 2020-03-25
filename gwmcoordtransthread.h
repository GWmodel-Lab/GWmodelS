#ifndef GWMCOORDTRANSTHREAD_H
#define GWMCOORDTRANSTHREAD_H

#include "prefix.h"
#include <QObject>
#include <QThread>
#include <QWidget>
#include "qgsprojectionselectionwidget.h"
#include "qgsprojectionselectiondialog.h"
#include <qgsmapcanvas.h>
#include "gwmtaskthread.h"

class GwmCoordTransThread: public GwmTaskThread
{
    Q_OBJECT
public:
    GwmCoordTransThread(QgsVectorLayer *handleLayer,QgsCoordinateReferenceSystem desCrs);
    //void transformCoordinate(const QgsCoordinateReferenceSystem des, QgsVectorLayer handleLayer);
    int cancelFlag;

    QgsVectorLayer *getHandleLayer() const;
    void setHandleLayer(QgsVectorLayer *value);

    QgsCoordinateReferenceSystem getDesCrs() const;
    void setDesCrs(const QgsCoordinateReferenceSystem &value);

    QgsVectorLayer *getWorkLayer() const;

protected:
    void run() override; //新线程入口

public slots:
    void onCancelTrans(int canceledFlag);

private:
    QgsVectorLayer *handleLayer;
    QgsCoordinateReferenceSystem desCrs;
    QgsVectorLayer* workLayer;
};

#endif // GWMCOORDTRANSTHREAD_H

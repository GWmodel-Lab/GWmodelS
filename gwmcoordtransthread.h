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

    QgsCoordinateReferenceSystem desCrs;

    QgsVectorLayer *handleLayer;

    //void transformCoordinate(const QgsCoordinateReferenceSystem des, QgsVectorLayer handleLayer);
    int cancelFlag;
protected:
    void run() override; //新线程入口

public slots:
    void onCancelTrans(int canceledFlag);
};

#endif // GWMCOORDTRANSTHREAD_H

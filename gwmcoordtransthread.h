#ifndef GWMCOORDTRANSTHREAD_H
#define GWMCOORDTRANSTHREAD_H

#include <QObject>
#include <QThread>
#include <QWidget>
#include "qgsprojectionselectionwidget.h"
#include "qgsprojectionselectiondialog.h"
#include <qgsmapcanvas.h>

class GwmCoordTransThread: public QThread
{
    Q_OBJECT
public:
    GwmCoordTransThread(QgsVectorLayer *handleLayer,QgsCoordinateReferenceSystem desCrs);

    QgsCoordinateReferenceSystem desCrs;

    QgsVectorLayer *handleLayer;

    //void transformCoordinate(const QgsCoordinateReferenceSystem des, QgsVectorLayer handleLayer);
protected:
    void run() override; //新线程入口
signals:
    void percentTransd(int progress,int total);
};

#endif // GWMCOORDTRANSTHREAD_H

#include "gwmcoordtransthread.h"

#include <QObject>
#include <QWidget>
#include "qgsprojectionselectionwidget.h"
#include "qgsprojectionselectiondialog.h"
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>

GwmCoordTransThread::GwmCoordTransThread(QgsVectorLayer *handleLayer,QgsCoordinateReferenceSystem desCrs)
{
    this->handleLayer = handleLayer;
    this->desCrs = desCrs;
}

void GwmCoordTransThread::run()
{
    emit message(QStringLiteral("Projecting ..."));
    this->cancelFlag = 0;
    QgsCoordinateTransform myTransform;
    myTransform.setSourceCrs(this->handleLayer->sourceCrs());
    myTransform.setDestinationCrs(this->desCrs);

    // 创建新的矢量图层
    // 类型
    QString newLayerType = QgsWkbTypes::displayString(this->handleLayer->wkbType());
    QString newLayerProperties = newLayerType.append("?");
    QString newLayerName = handleLayer->name().append(" ").append(desCrs.authid());
    workLayer = new QgsVectorLayer(newLayerProperties, newLayerName, QStringLiteral("memory"));
    workLayer->setCrs(desCrs);
    QgsVectorDataProvider* newLayerDataProvider = workLayer->dataProvider();
    // 属性构造
    QList<QgsField> tmp;
    for(int i=0;i<this->handleLayer->fields().size();i++){
        tmp.append(this->handleLayer->fields()[i]);
    }
    newLayerDataProvider->addAttributes(tmp);
    workLayer->updateFields();

    // 添加feature
    QgsFeatureIterator featureIt = this->handleLayer->getFeatures();
    QgsFeature f;
    workLayer->startEditing();

    // 进度条显示需要
    int progress = 0;
    int total = 0;
    QgsFeatureIterator featureIt2 = this->handleLayer->getFeatures();
    QgsFeature f2;
    while(featureIt2.nextFeature(f2)){
        total++;
    }

    while(featureIt.nextFeature(f)){

        if(this->cancelFlag == 0){
            QgsGeometry g = f.geometry();
            if(g.transform(myTransform) == 0)
            {
                f.setGeometry(g);
                newLayerDataProvider->addFeature(f);
            }
//            sleep(0.2);
            emit tick(progress,total);
            progress++;
        }
    }
    workLayer->commitChanges();
    emit success();
}

void GwmCoordTransThread::onCancelTrans(int flag){
    this->cancelFlag = flag;
}

QgsCoordinateReferenceSystem GwmCoordTransThread::getDesCrs() const
{
    return desCrs;
}

void GwmCoordTransThread::setDesCrs(const QgsCoordinateReferenceSystem &value)
{
    desCrs = value;
}

QgsVectorLayer *GwmCoordTransThread::getWorkLayer() const
{
    return workLayer;
}

QgsVectorLayer *GwmCoordTransThread::getHandleLayer() const
{
    return handleLayer;
}

void GwmCoordTransThread::setHandleLayer(QgsVectorLayer *value)
{
    handleLayer = value;
}

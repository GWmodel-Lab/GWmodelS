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
    this->cancelFlag = 0;
    //新线程入口
    //初始化和操作放在这里
//    qDebug() << this->handleLayer->fields().names();
//    qDebug() << this->desCrs.authid();
    // 构造转换
    QgsCoordinateTransform myTransform;
    myTransform.setSourceCrs(this->handleLayer->sourceCrs());
    myTransform.setDestinationCrs(this->desCrs);

    //输出当前图层的属性
    qDebug() << this->handleLayer->fields().names();
    //输出当前图层的属性类型
    for(int i=0;i<this->handleLayer->fields().size();i++){
        qDebug() << this->handleLayer->fields()[i].typeName();
    }

    // 创建新的矢量图层
    // 类型
    QString newLayerType = QgsWkbTypes::displayString(this->handleLayer->wkbType());
    // EPSG
    //qDebug() << currentLayer->crs().authid();
    QString newLayerProperties = newLayerType.append("?").append(this->handleLayer->crs().authid());
    // 构造图层
    QgsVectorLayer *newLayer = new QgsVectorLayer(newLayerProperties,QString("测试图层"),QString("memory"));
    QgsVectorDataProvider* newLayerDataProvider = newLayer->dataProvider();
    // 属性构造
    QList<QgsField> tmp;
    for(int i=0;i<this->handleLayer->fields().size();i++){
        tmp.append(this->handleLayer->fields()[i]);
    }
    newLayerDataProvider->addAttributes(tmp);
    newLayer->updateFields();

    // 添加feature
    QgsFeatureIterator featureIt = this->handleLayer->getFeatures();
    QgsFeature f;
    newLayer->startEditing();

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
            }
            else
            {
                f.clearGeometry();
            }
            newLayerDataProvider->addFeature(f);
            sleep(1);
            emit percentTransd(progress,total);
            progress ++;
        }
    }
    newLayer->commitChanges();

    // 输出测试结果
    qDebug() << newLayer->fields().names();
    for(int i=0;i<newLayer->fields().size();i++){
        qDebug() << newLayer->fields()[i].typeName();
    }
    qDebug() << newLayer->crs().authid() <<1;

    qDebug() << this->handleLayer->getFeature(1).geometry().asJson() << "Before trans";
    qDebug() << newLayer->getFeature(1).geometry().asJson() << "trans";
}

void GwmCoordTransThread::cancelTransSlo(int flag){
    this->cancelFlag = flag;
}

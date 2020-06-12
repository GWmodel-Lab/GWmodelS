#ifndef GWMSPATIALALGORITHM_H
#define GWMSPATIALALGORITHM_H

#include <QObject>

#include <qgsvectorlayer.h>

#include "TaskThread/gwmtaskthread.h"

class GwmSpatialAlgorithm : public GwmTaskThread
{
    Q_OBJECT
public:  // 构造与属性
    GwmSpatialAlgorithm();

    QgsVectorLayer *dataLayer() const;
    void setDataLayer(QgsVectorLayer *dataLayer);

    QgsVectorLayer *resultLayer() const;

protected:  // 方法
    virtual bool isValid() = 0;

protected:  // 成员
    QgsVectorLayer* mDataLayer = nullptr;
    QgsVectorLayer* mResultLayer = nullptr;
};

#endif // GWMSPATIALALGORITHM_H

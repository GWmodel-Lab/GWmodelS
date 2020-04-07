#include "gwmgwrtaskthread.h"

#include "GWmodel/GWmodel.h"

GwmGWRTaskThread::GwmGWRTaskThread(QgsVectorLayer* layer, GwmLayerAttributeItem* depVar, QList<GwmLayerAttributeItem*> indepVars)
    : GwmTaskThread()
    , mLayer(layer)
    , mDepVar(depVar)
    , mIndepVars(indepVars)
{
    QgsFeatureIterator it = mLayer->getFeatures();
    QgsFeature feature;
    while (it.nextFeature(feature))
    {
        mFeatureIds.append(feature.id());
    }

    // 计算所需的矩阵
    mX = mat(mFeatureIds.size(), indepVars.size() + 1);
    mY = vec(mFeatureIds.size());
    mBetas = mat(indepVars.size() + 1, mFeatureIds.size());
    mDataPoints = mat(mFeatureIds.size(), 2);

    // 获取自变量和因变量所属的属性列索引
    mDepVarIndex = mDepVar->attributeIndex();
    for (GwmLayerAttributeItem* item : mIndepVars)
    {
        int iIndepVar = item->attributeIndex();
        mIndepVarsIndex.append(iIndepVar);
    }
}

void GwmGWRTaskThread::run()
{
    if (setXY())
    {
        return;
    }
    for (int i = 0; i < mFeatureIds.size(); i++)
    {
        QgsFeatureId id = mFeatureIds[i];
        mat dist = distance(id);
        mat weight = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
        auto result = gwReg(mX, mY, weight, false, i);
        mBetas.col(i) = result[RegressionResult::Beta];
        emit tick(i, mFeatureIds.size());
    }
}

bool GwmGWRTaskThread::isValid(QString &message)
{
    // [TODO]: 实现验证 GWR 是否可以进行的代码
    return true;
}

bool GwmGWRTaskThread::isNumeric(QVariant::Type type)
{
    switch (type)
    {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::UInt:
    case QVariant::Double:
        return true;
    default:
        return false;
    }
}

bool GwmGWRTaskThread::setXY()
{
    QgsField depField = mLayer->fields()[mDepVarIndex];
    if (!isNumeric(depField.type()))
    {
        emit error(tr("Dependent variable is not numeric."));
        return false;
    }
    for (int iIndepVar : mIndepVarsIndex)
    {
        QgsField indepField = mLayer->fields()[iIndepVar];
        if (!isNumeric(indepField.type()))
        {
            emit error(tr("Independent variable \"") + indepField.name() + tr("\" is not numeric."));
            return false;
        }
    }

    int row = 0;
    bool* ok = nullptr;
    for (QgsFeatureId featureId : mFeatureIds)
    {
        QgsFeature feature = mLayer->getFeature(featureId);
        double vY = feature.attribute(mDepVarIndex).toDouble(ok);
        if (ok)
        {
            mY.at(row, 0) = vY;
            delete ok;
            ok = nullptr;

            // 设置 X 矩阵
            for (int index : mIndepVarsIndex)
            {
                double vX = feature.attribute(index).toDouble(ok);
                if (ok)
                {
                    mX.at(row, index + 1) = vX;
                    delete ok;
                    ok = nullptr;
                }
                else
                {
                    emit error(tr("Independent variable value cannot convert to a number. Set to 0."));
                }
            }
            // 设置坐标
            QgsPointXY centroPoint = feature.geometry().centroid().asPoint();
            mDataPoints.at(row, 0) = centroPoint.x();
            mDataPoints.at(row, 1) = centroPoint.y();
        }
        else
        {
            emit error(tr("Dependent variable value cannot convert to a number. Set to 0."));
        }
    }
    // 坐标旋转
    if (mDistSrcType == DistanceSourceType::Minkowski)
    {
        QMap<QString, QVariant> parameters = mDistSrcParameters.toMap();
        double theta = parameters["theta"].toDouble();
        mDataPoints = coordinateRotate(mDataPoints, theta);
    }
    return true;
}

vec GwmGWRTaskThread::distance(const QgsFeatureId &id)
{
    switch (mDistSrcType)
    {
    case DistanceSourceType::Minkowski:
        return distanceMinkowski(id);
    default:
        return distanceCRS(id);
    }
}

vec GwmGWRTaskThread::distanceCRS(const QgsFeatureId &id)
{
    QgsFeature fSrc = mLayer->getFeature(id);
    QgsGeometry gSrc = fSrc.geometry();
    vec dist(mFeatureIds.size(), fill::zeros);
    for (int i = 0; i < mFeatureIds.size(); i++)
    {
        QgsFeature fDes = mLayer->getFeature(mFeatureIds[i]);
        dist.at(i) = fDes.geometry().distance(gSrc);
    }
    return dist;
}

vec GwmGWRTaskThread::distanceMinkowski(const QgsFeatureId &id)
{
    QMap<QString, QVariant> parameters = mDistSrcParameters.toMap();
    double p = parameters["p"].toDouble();
    QgsFeature fSrc = mLayer->getFeature(id);
    QgsPointXY gSrc = fSrc.geometry().centroid().asPoint();
    vec srcLoc(gSrc.x(), gSrc.y());
    return mkDistVec(mDataPoints, srcLoc, p);
}

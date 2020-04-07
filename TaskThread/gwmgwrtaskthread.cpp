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
    mX = mat(mFeatureIds.size(), indepVars.size() + 1);
    mY = vec(mFeatureIds.size());
    mBetas = mat(indepVars.size() + 1, mFeatureIds.size());
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
            delete ok;
            ok = nullptr;
        }
        else
        {
            emit error(tr("Dependent variable value cannot convert to a number. Set to 0."));
        }
    }
    return true;
}

vec GwmGWRTaskThread::distance(const QgsFeatureId &id)
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

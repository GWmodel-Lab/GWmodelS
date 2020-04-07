#include "gwmgwrtaskthread.h"

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
        mFeatureIds.insert(feature.id());
    }
    mX = mat(mFeatureIds.size(), indepVars.size() + 1);
    mY = vec(mFeatureIds.size());
    mBetas = mat(mFeatureIds.size(), indepVars.size() + 1);
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

#include "gwmgwrtaskthread.h"

#include <exception>
#include "GWmodel/GWmodel.h"

using namespace std;

QMap<QString, double> GwmGWRTaskThread::fixedBwUnitDict = QMap<QString, double>();
QMap<QString, double> GwmGWRTaskThread::adaptiveBwUnitDict = QMap<QString, double>();

void GwmGWRTaskThread::initUnitDict()
{
    // 设置静态变量
    fixedBwUnitDict["m"] = 1.0;
    fixedBwUnitDict["km"] = 1000.0;
    fixedBwUnitDict["mile"] = 1609.344;
    adaptiveBwUnitDict["x1"] = 1;
    adaptiveBwUnitDict["x10"] = 10;
    adaptiveBwUnitDict["x100"] = 100;
    adaptiveBwUnitDict["x1000"] = 1000;
}

GwmGWRTaskThread::GwmGWRTaskThread()
    : GwmTaskThread()
{

}

void GwmGWRTaskThread::run()
{
    if (!setXY())
    {
        return;
    }
    emit message(tr("Calibrating GWR model..."));
    emit tick(0, mFeatureList.size());
    for (int i = 0; i < mFeatureList.size(); i++)
    {
        mat dist = distance(i);
        mat weight = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
        try {
            auto result = gwReg(mX, mY, weight, false, i);
            mBetas.col(i) = result[RegressionResult::Beta];
            emit tick(i + 1, mFeatureList.size());
        } catch (exception e) {
            emit error(e.what());
        }
    }
    mBetas = trans(mBetas);
    createResultLayer();
    emit success();
}

QString GwmGWRTaskThread::name() const
{
    return tr("GWR");
}

bool GwmGWRTaskThread::isValid(QString &message)
{
    if (!mLayer)
    {
        message = (tr("Layer is not selected."));
        return false;
    }
    if (!mDepVar)
    {
        message = (tr("Dependent variable is not selected."));
        return false;
    }
    QgsField depField = mLayer->fields()[mDepVarIndex];
    if (!isNumeric(depField.type()))
    {
        message = (tr("Dependent variable is not numeric."));
        return false;
    }
    for (int iIndepVar : mIndepVarsIndex)
    {
        QgsField indepField = mLayer->fields()[iIndepVar];
        if (!isNumeric(indepField.type()))
        {
            message = (tr("Independent variable \"") + indepField.name() + tr("\" is not numeric."));
            return false;
        }
    }

    return true;
}

QgsVectorLayer *GwmGWRTaskThread::layer() const
{
    return mLayer;
}

void GwmGWRTaskThread::setLayer(QgsVectorLayer *layer)
{
    mLayer = layer;
    mFeatureList.clear();
}

QList<GwmLayerAttributeItem *> GwmGWRTaskThread::indepVars() const
{
    return mIndepVars;
}

void GwmGWRTaskThread::setIndepVars(const QList<GwmLayerAttributeItem *> &indepVars)
{
    mIndepVars = indepVars;
    mIndepVarsIndex.clear();
    for (GwmLayerAttributeItem* item : mIndepVars)
    {
        int iIndepVar = item->attributeIndex();
        mIndepVarsIndex.append(iIndepVar);
    }
}

GwmGWRTaskThread::BandwidthType GwmGWRTaskThread::bandwidthType() const
{
    return mBandwidthType;
}

void GwmGWRTaskThread::setBandwidthType(const BandwidthType &bandwidthType)
{
    mBandwidthType = bandwidthType;
}

bool GwmGWRTaskThread::getIsBandwidthSizeAutoSel() const
{
    return isBandwidthSizeAutoSel;
}

void GwmGWRTaskThread::setIsBandwidthSizeAutoSel(bool value)
{
    isBandwidthSizeAutoSel = value;
}

GwmGWRTaskThread::KernelFunction GwmGWRTaskThread::getBandwidthKernelFunction() const
{
    return mBandwidthKernelFunction;
}

void GwmGWRTaskThread::setBandwidthKernelFunction(const KernelFunction &bandwidthKernelFunction)
{
    mBandwidthKernelFunction = bandwidthKernelFunction;
}

GwmGWRTaskThread::DistanceSourceType GwmGWRTaskThread::getDistSrcType() const
{
    return mDistSrcType;
}

void GwmGWRTaskThread::setDistSrcType(const DistanceSourceType &distSrcType)
{
    mDistSrcType = distSrcType;
}

QVariant GwmGWRTaskThread::getDistSrcParameters() const
{
    return mDistSrcParameters;
}

void GwmGWRTaskThread::setDistSrcParameters(const QVariant &distSrcParameters)
{
    mDistSrcParameters = distSrcParameters;
}

GwmGWRTaskThread::ParallelMethod GwmGWRTaskThread::getParallelMethodType() const
{
    return mParallelMethodType;
}

void GwmGWRTaskThread::setParallelMethodType(const ParallelMethod &parallelMethodType)
{
    mParallelMethodType = parallelMethodType;
}

QVariant GwmGWRTaskThread::getParallelParameter() const
{
    return mParallelParameter;
}

void GwmGWRTaskThread::setParallelParameter(const QVariant &parallelParameter)
{
    mParallelParameter = parallelParameter;
}

QgsVectorLayer *GwmGWRTaskThread::getResultLayer() const
{
    return mResultLayer;
}

bool GwmGWRTaskThread::enableIndepVarAutosel() const
{
    return isEnableIndepVarAutosel;
}

void GwmGWRTaskThread::setEnableIndepVarAutosel(bool value)
{
    isEnableIndepVarAutosel = value;
}

GwmLayerAttributeItem *GwmGWRTaskThread::depVar() const
{
    return mDepVar;
}

void GwmGWRTaskThread::setDepVar(GwmLayerAttributeItem *depVar)
{
    mDepVar = depVar;
    mDepVarIndex = mDepVar->attributeIndex();
}

void GwmGWRTaskThread::setBandwidth(GwmGWRTaskThread::BandwidthType type, double size, QString unit)
{
    mBandwidthSizeOrigin = size;
    mBandwidthSize = size;
    mBandwidthType = type;
    mBandwidthUnit = unit;
    if (type == BandwidthType::Fixed)
    {
        mBandwidthSize = size * fixedBwUnitDict[unit];
    }
    else
    {
        mBandwidthSize = size * adaptiveBwUnitDict[unit];
    }
    qDebug() << "[GwmGWRTaskThread::setBandwidth]"
             << "mBandwidthSizeOrigin" << mBandwidthSizeOrigin
             << "mBandwidthUnit" << unit
             << "mBandwidthSize" << mBandwidthSize
             << "mBandwidthType" << mBandwidthType;
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
    // 提取 FeatureID
    emit message("Setting matrices.");
    QgsFeatureIterator it = mLayer->getFeatures();
    QgsFeature f;
    while (it.nextFeature(f))
    {
        mFeatureList.append(f);
    }
    // 初始化所需矩阵
    mX = mat(mFeatureList.size(), mIndepVarsIndex.size() + 1);
    mY = vec(mFeatureList.size());
    mBetas = mat(mIndepVarsIndex.size() + 1, mFeatureList.size());
    mDataPoints = mat(mFeatureList.size(), 2);

    bool ok = false;
    int row = 0, total = mFeatureList.size();
    it.rewind();
    for (row = 0; row < total; row++)
    {
        QgsFeature feature = mFeatureList[row];
        double vY = feature.attribute(mDepVarIndex).toDouble(&ok);
        if (ok)
        {
            mY.at(row, 0) = vY;

            // 设置 X 矩阵
            for (int i = 0; i < mIndepVarsIndex.size(); i++)
            {
                int index = mIndepVarsIndex[i];
                double vX = feature.attribute(index).toDouble(&ok);
                if (ok)
                {
                    mX.at(row, i + 1) = vX;
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
        else emit error(tr("Dependent variable value cannot convert to a number. Set to 0."));
    }
    // 坐标旋转
    if (mDistSrcType == DistanceSourceType::Minkowski)
    {
        QMap<QString, QVariant> parameters = mDistSrcParameters.toMap();
        double theta = parameters["theta"].toDouble();
        mDataPoints = coordinateRotate(mDataPoints, theta);
    }
    emit tick(total, total);
    return true;
}

vec GwmGWRTaskThread::distance(int focus)
{
    switch (mDistSrcType)
    {
    case DistanceSourceType::Minkowski:
        return distanceMinkowski(focus);
    default:
        return distanceCRS(focus);
    }
}

vec GwmGWRTaskThread::distanceCRS(int focus)
{
    bool longlat = mLayer->crs().isGeographic();
    return gwDist(mDataPoints, mDataPoints, focus, 2.0, 0.0, longlat, false);
}

vec GwmGWRTaskThread::distanceMinkowski(int focus)
{
    QMap<QString, QVariant> parameters = mDistSrcParameters.toMap();
    double p = parameters["p"].toDouble();
    return gwDist(mDataPoints, mDataPoints, focus, p, 0.0, false, false);
}

void GwmGWRTaskThread::createResultLayer()
{
    emit message("Creating result layer...");
    QString layerFileName = QgsWkbTypes::displayString(mLayer->wkbType()) + QStringLiteral("?");
    QString layerName = mLayer->name();
    if (mBandwidthType == BandwidthType::Fixed)
    {
        layerName += QString("_B%1%2").arg(mBandwidthSizeOrigin, 0, 'f', 3).arg(mBandwidthSize);
    }
    else
    {
        layerName += QString("_B%1").arg(int(mBandwidthSize));
    }
    mResultLayer = new QgsVectorLayer(layerFileName, layerName, QStringLiteral("memory"));
    mResultLayer->setCrs(mLayer->crs());

    QgsFields fields;
    fields.append(QgsField(QStringLiteral("Intercept"), QVariant::Double, QStringLiteral("double")));
    for (GwmLayerAttributeItem* item : mIndepVars)
    {
        QString srcName = item->attributeName();
        QString name = QString("Beta_") + srcName;
        fields.append(QgsField(name, QVariant::Double, QStringLiteral("double")));
    }
    mResultLayer->dataProvider()->addAttributes(fields.toList());
    mResultLayer->updateFields();

    mResultLayer->startEditing();
    for (int f = 0; f < mFeatureList.size(); f++)
    {
        QgsFeature srcFeature = mFeatureList[f];
        QgsFeature feature(fields);
        feature.setGeometry(srcFeature.geometry());
        for (int a = 0; a < fields.size(); a++)
        {
            QString attributeName = fields[a].name();
            double attributeValue = mBetas(f, a);
            feature.setAttribute(attributeName, attributeValue);
        }
        mResultLayer->addFeature(feature);
    }
    mResultLayer->commitChanges();
}

#include "gwmgwrtaskthread.h"

#include <exception>
#include "GWmodel/GWmodel.h"

#include "TaskThread/gwmbandwidthselecttaskthread.h"
#include "TaskThread/gwmgwrmodelselectionthread.h"

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
    mX = mat(uword(0), uword(0));
    mY = vec(uword(0));
    mDataPoints = mat(uword(0), 2);
    mBetas = mat(uword(0), uword(0));
    mRowSumBetasSE = mat(uword(0), uword(0));
    mBetasSE = mat(uword(0), uword(0));
    mBetasTV = mat(uword(0), uword(0));
    mSHat = vec(uword(0));
    mQDiag = vec(uword(0));
    mYHat = vec(uword(0));
    mResidual = vec(uword(0));
    mStudentizedResidual = vec(uword(0));
    mLocalRSquare = vec(uword(0));
}

GwmGWRTaskThread::GwmGWRTaskThread(const GwmGWRTaskThread &taskThread)
{
    mLayer = taskThread.mLayer;
    mDepVar = taskThread.mDepVar;
    mIndepVars = taskThread.mIndepVars;
    mDepVarIndex = taskThread.mDepVarIndex;
    mIndepVarsIndex = taskThread.mIndepVarsIndex;
    mModelSelThreshold = taskThread.mModelSelThreshold;
    isEnableIndepVarAutosel = taskThread.isEnableIndepVarAutosel;
    mFeatureList = taskThread.mFeatureList;
    hasHatMatrix = taskThread.hasHatMatrix;
    mBandwidthType = taskThread.mBandwidthType;
    mBandwidthSize = taskThread.mBandwidthSize;
    mBandwidthSizeOrigin = taskThread.mBandwidthSizeOrigin;
    mBandwidthUnit = taskThread.mBandwidthUnit;
    isBandwidthSizeAutoSel = taskThread.isBandwidthSizeAutoSel;
    mBandwidthSelectionApproach = taskThread.mBandwidthSelectionApproach;
    mBandwidthKernelFunction = taskThread.mBandwidthKernelFunction;
    mDistSrcType = taskThread.mDistSrcType;
    mDistSrcParameters = taskThread.mDistSrcParameters;
    mCRSRotateTheta = taskThread.mCRSRotateTheta;
    mCRSRotateP = taskThread.mCRSRotateP;
    mParallelMethodType = taskThread.mParallelMethodType;
    mParallelParameter = taskThread.mParallelParameter;
    mX = mat(taskThread.mX);
    mY = vec(taskThread.mY);
    mDataPoints = mat(taskThread.mDataPoints);
    mBetas = mat(uword(0), uword(0));
    mRowSumBetasSE = mat(uword(0), uword(0));
    mBetasSE = mat(uword(0), uword(0));
    mBetasTV = mat(uword(0), uword(0));
    mSHat = vec(uword(0));
    mQDiag = vec(uword(0));
    mYHat = vec(uword(0));
    mResidual = vec(uword(0));
    mStudentizedResidual = vec(uword(0));
    mLocalRSquare = vec(uword(0));
}

void GwmGWRTaskThread::run()
{
    // 优选模型
    if (isEnableIndepVarAutosel)
    {
        emit message(tr("Selecting optimized model..."));
        GwmGWRModelSelectionThread modelSelThread(*this);
        connect(&modelSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
        connect(&modelSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
        connect(&modelSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
        modelSelThread.start();
        modelSelThread.wait();
        disconnect(&modelSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
        disconnect(&modelSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
        disconnect(&modelSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
        QPair<QList<int>, double> optimizedModel = modelSelThread.modelSelection();
        if (optimizedModel.second != DBL_MAX)
        {
            mIndepVarsIndex = optimizedModel.first;
            mModelSelModels = modelSelThread.getModelInDepVars();
            mModelSelAICcs = modelSelThread.getModelAICcs();

            // 绘图
            QMap<QString, QVariant> data;
            QList<QVariant> indepVarNames, modelSelModels, modelSelAICcs;
            for (GwmLayerAttributeItem* item : mIndepVars)
            {
                indepVarNames.append(item->attributeName());
            }
            data["indepVarNames"] = indepVarNames;
            for (QStringList modelStringList : mModelSelModels)
            {
                modelSelModels.append(modelStringList);
            }
            data["modelSelModels"] = modelSelModels;
            for (double aic : mModelSelAICcs)
            {
                modelSelAICcs.append(aic);
            }
            data["modelSelAICcs"] = modelSelAICcs;
            emit plot(data, &GwmGWRModelSelectionThread::plotModelOrder);
            emit plot(data, &GwmGWRModelSelectionThread::plotModelAICcs);
        }
        else
        {
            emit error(tr("Cannot select optimized model."));
            return;
        }
    }

    // 设置矩阵
    if (!setXY())
    {
        return;
    }

    // 优选带宽
    if (isBandwidthSizeAutoSel)
    {
        emit message(tr("Selecting optimized bandwidth..."));
        GwmBandwidthSelectTaskThread bwSelThread(*this);
        connect(&bwSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
        connect(&bwSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
        connect(&bwSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
        bwSelThread.start();
        bwSelThread.wait();
        disconnect(&bwSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
        disconnect(&bwSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
        disconnect(&bwSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
        mBandwidthSize = bwSelThread.getBandwidthSize();
        mBandwidthSelScore = bwSelThread.getBwScore();
        // 绘图
        QList<QVariant> plotData;
        for (auto i = mBandwidthSelScore.constBegin(); i != mBandwidthSelScore.constEnd(); i++)
        {
            plotData.append(QVariant(QPointF(i.key(), i.value())));
        }
        emit plot(QVariant(plotData), &GwmBandwidthSelectTaskThread::plotBandwidthResult);
    }

    // 解算模型
    emit message(tr("Calibrating GWR model..."));
    emit tick(0, mFeatureList.size());
    bool isAllCorrect = true;
    if (hasHatMatrix)
    {
        for (int i = 0; i < mFeatureList.size(); i++)
        {
            try
            {
                vec dist = distance(i);
                vec weight = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
                mat ci, si;
                mBetas.col(i) = gwRegHatmatrix(mX, mY, weight, i, ci, si);
                mBetasSE.col(i) = (ci % ci) * mRowSumBetasSE;
                mSHat(0) += si(0, i);
                mSHat(1) += det(si * trans(si));
                vec p = -trans(si);
                p(i) += 1.0;
                mQDiag += p % p;
                emit tick(i + 1, mFeatureList.size());
            } catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        mBetas = trans(mBetas);
        mBetasSE = trans(mBetasSE);
    }
    else
    {
        for (int i = 0; i < mFeatureList.size(); i++)
        {
            try {
                vec dist = distance(i);
                vec weight = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
                mBetas.col(i) = gwReg(mX, mY, weight, i);
                emit tick(i + 1, mFeatureList.size());
            } catch (exception e) {
                emit error(e.what());
            }
        }
        mBetas = trans(mBetas);
    }
    if (isAllCorrect)
    {
        diagnostic();
        createResultLayer();
        emit success();
    }
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

QgsFeatureList GwmGWRTaskThread::getFeatureList() const
{
    return mFeatureList;
}

GwmGWRDiagnostic GwmGWRTaskThread::getDiagnostic() const
{
    return mDiagnostic;
}

mat GwmGWRTaskThread::getBetas() const
{
    return mBetas;
}

GwmGWRTaskThread::BandwidthSelectionApproach GwmGWRTaskThread::getBandwidthSelectionApproach() const
{
    return mBandwidthSelectionApproach;
}

void GwmGWRTaskThread::setBandwidthSelectionApproach(const BandwidthSelectionApproach &bandwidthSelectionApproach)
{
    mBandwidthSelectionApproach = bandwidthSelectionApproach;
}

QList<QStringList> GwmGWRTaskThread::getModelSelModels() const
{
    return mModelSelModels;
}

QList<double> GwmGWRTaskThread::getModelSelAICcs() const
{
    return mModelSelAICcs;
}

int GwmGWRTaskThread::getDepVarIndex() const
{
    return mDepVarIndex;
}

QList<int> GwmGWRTaskThread::getIndepVarsIndex() const
{
    return mIndepVarsIndex;
}

QMap<double, double> GwmGWRTaskThread::getBwScore() const
{
    return mBandwidthSelScore;
}

double GwmGWRTaskThread::getModelSelThreshold() const
{
    return mModelSelThreshold;
}

void GwmGWRTaskThread::setModelSelThreshold(double modelSelThreshold)
{
    mModelSelThreshold = modelSelThreshold;
}

double GwmGWRTaskThread::getBandwidthSize() const
{
    return mBandwidthSize;
}

double GwmGWRTaskThread::getBandwidthSizeOrigin() const
{
    return mBandwidthSizeOrigin;
}

QString GwmGWRTaskThread::getBandwidthUnit() const
{
    return mBandwidthUnit;
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
    mX = mat(mFeatureList.size(), mIndepVarsIndex.size() + 1, fill::zeros);
    mY = vec(mFeatureList.size(), fill::zeros);
    mBetas = mat(mIndepVarsIndex.size() + 1, mFeatureList.size(), fill::zeros);
    if (hasHatMatrix)
    {
        mBetasSE = mat(mIndepVarsIndex.size() + 1, mFeatureList.size(), fill::zeros);
        mSHat = vec(2, fill::zeros);
        mQDiag = vec(mFeatureList.size(), fill::zeros);
        mRowSumBetasSE = mat(mFeatureList.size(), 1, fill::ones);
    }
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
            mY(row, 0) = vY;
            mX(row, 0) = 1.0;
            // 设置 X 矩阵
            for (int i = 0; i < mIndepVarsIndex.size(); i++)
            {
                int index = mIndepVarsIndex[i];
                double vX = feature.attribute(index).toDouble(&ok);
                if (ok)
                {
                    mX(row, i + 1) = vX;
                }
                else
                {
                    emit error(tr("Independent variable value cannot convert to a number. Set to 0."));
                }
            }
            // 设置坐标
            QgsPointXY centroPoint = feature.geometry().centroid().asPoint();
            mDataPoints(row, 0) = centroPoint.x();
            mDataPoints(row, 1) = centroPoint.y();
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

void GwmGWRTaskThread::diagnostic()
{
    emit message(tr("Calculating diagnostic informations..."));

    // 诊断信息
    vec vDiags = gwrDiag(mY, mX, mBetas, mSHat);
    mDiagnostic = GwmGWRDiagnostic(vDiags);
    mYHat = fitted(mX, mBetas);
    mResidual = mY - mYHat;
    double trS = mSHat(0), trStS = mSHat(1);
    double nDp = mFeatureList.size();
    double sigmaHat = mDiagnostic.RSS / (nDp - 2 * trS + trStS);
    mStudentizedResidual = mResidual / sqrt(sigmaHat * mQDiag);
    mBetasSE = sqrt(sigmaHat * mBetasSE);
    mBetasTV = mBetas / mBetasSE;
    vec dybar2 = (mY - sum(mY) / nDp) % (mY - sum(mY) / nDp);
    vec dyhat2 = (mY - mYHat) % (mY - mYHat);

    // Local RSquare
    mLocalRSquare = vec(mFeatureList.size(), fill::zeros);
    for (int i = 0; i < mFeatureList.size(); i++)
    {
        vec dist = distance(i);
        mat w = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
        double tss = sum(dybar2 % w);
        double rss = sum(dyhat2 % w);
        mLocalRSquare(i) = (tss - rss) / tss;
    }
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
    for (int index : mIndepVarsIndex)
    {
        QString srcName = mLayer->fields().field(index).name();
        QString name = srcName;
        fields.append(QgsField(name, QVariant::Double, QStringLiteral("double")));
    }
    if (hasHatMatrix)
    {
        fields.append(QgsField(QStringLiteral("y"), QVariant::Double, QStringLiteral("double")));
        fields.append(QgsField(QStringLiteral("yhat"), QVariant::Double, QStringLiteral("double")));
        fields.append(QgsField(QStringLiteral("residual"), QVariant::Double, QStringLiteral("double")));
        fields.append(QgsField(QStringLiteral("Stud_residual"), QVariant::Double, QStringLiteral("double")));
        fields.append(QgsField(QStringLiteral("Intercept_SE"), QVariant::Double, QStringLiteral("double")));
        for (int index : mIndepVarsIndex)
        {
            QString srcName = mLayer->fields().field(index).name();
            QString name = srcName + QStringLiteral("_SE");
            fields.append(QgsField(name, QVariant::Double, QStringLiteral("double")));
        }
        fields.append(QgsField(QStringLiteral("Intercept_TV"), QVariant::Double, QStringLiteral("double")));
        for (int index : mIndepVarsIndex)
        {
            QString srcName = mLayer->fields().field(index).name();
            QString name = srcName + QStringLiteral("_TV");
            fields.append(QgsField(name, QVariant::Double, QStringLiteral("double")));
        }
        fields.append(QgsField(QStringLiteral("Local_R2"), QVariant::Double, QStringLiteral("double")));
    }
    mResultLayer->dataProvider()->addAttributes(fields.toList());
    mResultLayer->updateFields();

    mResultLayer->startEditing();
    if (hasHatMatrix)
    {
        int indepSize = mIndepVarsIndex.size() + 1;
        for (int f = 0; f < mFeatureList.size(); f++)
        {
            int curCol = 0;
            QgsFeature srcFeature = mFeatureList[f];
            QgsFeature feature(fields);
            feature.setGeometry(srcFeature.geometry());
            for (int a = 0; a < indepSize; a++)
            {
                int fieldIndex = a + curCol;
                QString attributeName = fields[fieldIndex].name();
                double attributeValue = mBetas(f, a);
                feature.setAttribute(attributeName, attributeValue);
            }
            curCol += indepSize;
            feature.setAttribute(fields[curCol++].name(), mY(f));
            feature.setAttribute(fields[curCol++].name(), mYHat(f));
            feature.setAttribute(fields[curCol++].name(), mResidual(f));
            feature.setAttribute(fields[curCol++].name(), mStudentizedResidual(f));
            for (int a = 0; a < indepSize; a++)
            {
                int fieldIndex = a + curCol;
                QString attributeName = fields[fieldIndex].name();
                double attributeValue = mBetasSE(f, a);
                feature.setAttribute(attributeName, attributeValue);
            }
            curCol += indepSize;
            for (int a = 0; a < indepSize; a++)
            {
                int fieldIndex = a + curCol;
                QString attributeName = fields[fieldIndex].name();
                double attributeValue = mBetasTV(f, a);
                feature.setAttribute(attributeName, attributeValue);
            }
            curCol += indepSize;
            feature.setAttribute(fields[curCol++].name(), mLocalRSquare(f));
            mResultLayer->addFeature(feature);
        }
    }
    else
    {
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
    }
    mResultLayer->commitChanges();
}

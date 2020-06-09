#include "gwmmultiscalegwrtaskthread.h"

#include <exception>
#include "GWmodel/GWmodel.h"

//#include <iomanip>

using namespace std;

void preview(string filename, const mat& obj, string title, bool append = false)
{
    ofstream fout(filename, append ? ios::app : ios::out);
    if (fout.is_open())
    {
        fout.setf(ios::fixed);
        fout.precision(3);
        fout.width(12);
        fout << title << std::endl;
        obj.raw_print(fout);
    }
    fout.close();
}

GwmMultiscaleGWRTaskThread::GwmMultiscaleGWRTaskThread()
    : GwmGWRTaskThread()
{
    mBandwidthSizePS = vec(1, fill::zeros) + 100.0;
    mInitialBandwidthSize = vec(1, fill::zeros) + 100.0;
    mBandwidthUnit = { "x1" };
    mBandwidthSeled = { BandwidthSeledType::Null };
    mBandwidthType = { GwmGWRTaskThread::BandwidthType::Adaptive };
    mBandwidthSelectionApproach = { GwmGWRTaskThread::BandwidthSelectionApproach::CV };
    mBandwidthKernelFunction = { GwmGWRTaskThread::KernelFunction::Gaussian };
    mDistanceSource = { GwmGWRTaskThread::DistanceSourceType::CRS };
    mDistSrcParameters = { QVariant() };
    mPreditorCentered = { false };
    mBandwidthSelectThreshold = vec(1, fill::zeros) + 1e-6;
    mS0 = mat(0, 0);
    mSArray = cube(0, 0, 0);
    mC = cube(0, 0, 0);
}

void GwmMultiscaleGWRTaskThread::run()
{
    if (!setXY())
    {
        return;
    }
    uword nDp = mX.n_rows, nVar = mX.n_cols;

    // ********************************
    // Centering and scaling predictors
    // ********************************
    mX0 = mX;
    mY0 = mY;
    for (uword i = 1; i < nVar; i++)
    {
        if (mPreditorCentered[i])
        {
            mX.col(i) = mX.col(i) - mean(mX.col(i));
        }
    }
    preview("PSDM/x1.txt", mX, "x1");

    // ***********************
    // Intialize the bandwidth
    // ***********************
    for (uword i = 0; i < nVar; i++)
    {
        if (mBandwidthSeled[i] == BandwidthSeledType::Null)
        {
            GwmBandwidthSelectTaskThread bwSelThread;
            bwSelThread.setLayer(mLayer);
            bwSelThread.setDepVar(mDepVar);
            QList<GwmLayerAttributeItem*> selIndepVars = QList<GwmLayerAttributeItem*>();
            if (i > 0)
                selIndepVars.append(mIndepVars[i - 1]);
            bwSelThread.setIndepVars(selIndepVars);
            bwSelThread.setBandwidthType(mBandwidthType[i]);
            bwSelThread.setBandwidthKernelFunction(mBandwidthKernelFunction[i]);
            bwSelThread.setBandwidthSelectionApproach(mBandwidthSelectionApproach[i]);
            double bw = selectOptimizedBandwidth(bwSelThread, false);
            mInitialBandwidthSize(i) = bw;
        }
    }

    // *****************************************************
    // Calculate the initial beta0 from the above bandwidths
    // *****************************************************
    GwmBandwidthSelectTaskThread bwSelThread(*this);
    bwSelThread.setBandwidthType(mBandwidthType[0]);
    bwSelThread.setBandwidthSelectionApproach(mBandwidthSelectionApproach[0]);
    bwSelThread.setBandwidthKernelFunction(mBandwidthKernelFunction[0]);
    bwSelThread.setDistSrcType(mDistanceSource[0]);
    bwSelThread.setDistSrcParameters(mDistSrcParameters[0]);
    if (mBandwidthType[0] == GwmGWRTaskThread::BandwidthType::Adaptive)
    {
        bwSelThread.setLower(mAdaptiveLower);
    }
    double bwInit0 = selectOptimizedBandwidth(bwSelThread);
    mBandwidthSize = bwInit0;

    // 初始化诊断信息矩阵
    if (hasHatMatrix)
    {
        mS0 = mat(nDp, nDp, fill::zeros);
        mSArray = cube(nDp, nDp, nVar, fill::zeros);
        mC = cube(nVar, nDp, nDp, fill::zeros);
    }

    bool isAllCorrect = GwmMultiscaleGWRTaskThread::regressionAllSerial();
    if (!isAllCorrect)
        return;
    if (hasHatMatrix)
    {
        mat idm(nVar, nVar, fill::eye);
        for (uword i = 0; i < nVar; ++i)
        {
            for (uword j = 0; j < nDp; ++j)
            {
                mSArray.slice(i).row(j) = mX(j, i) * (idm.row(i) * mC.slice(j));
            }
        }
    }

    preview("PSDM/betas.txt", mBetas, "betas");

    // ***********************************************************
    // Select the optimum bandwidths for each independent variable
    // ***********************************************************
    emit message(QString("-------- Select the Optimum Bandwidths for each Independent Varialbe --------"));
    uvec bwChangeNo(nVar, fill::zeros);
    vec resid = mY - fitted(mX, mBetas);
    preview("PSDM/resid.txt", resid, "resid");
    double RSS0 = sum(resid % resid), RSS = DBL_MAX;
    double criterion = DBL_MAX;
    for (int iteration = 1; iteration <= mMaxIteration && criterion > mCriterionThreshold; iteration++)
    {
        for (int i = 0; i < nVar; i++)
        {
            QString varName = i == 0 ? QStringLiteral("Intercept") : mIndepVars[i-1]->attributeName();
            double bwi = DBL_MAX;
            vec fi = mBetas.col(i) % mX.col(i);
            preview("PSDM/fi.txt", fi, QString("Iteration %1 Variable %2").arg(iteration - 1).arg(varName).toStdString(), !(iteration == 1 && i == 0));
            vec yi = resid + fi;
            preview("PSDM/yi.txt", yi, QString("Iteration %1 Variable %2").arg(iteration - 1).arg(varName).toStdString(), !(iteration == 1 && i == 0));
            if (mBandwidthSeled[i] == BandwidthSeledType::Specified)
            {
                bwi = mInitialBandwidthSize[i];
            }
            else
            {
                QString varName = i == 0 ? QStringLiteral("Intercept") : mIndepVars[i-1]->attributeName();
                emit message(QString("Now select an optimum bandwidth for the variable: %1").arg(varName));
                GwmBandwidthSelectTaskThread bwSelThread(*this);
                bwSelThread.setX(mX.col(i));
                bwSelThread.setY(yi);
                bwSelThread.setBandwidth(mBandwidthType[i], mInitialBandwidthSize[i], mBandwidthUnit[i]);
                bwSelThread.setBandwidthSelectionApproach(mBandwidthSelectionApproach[i]);
                bwSelThread.setBandwidthKernelFunction(mBandwidthKernelFunction[i]);
                bwSelThread.setDistSrcType(mDistanceSource[i]);
                bwSelThread.setDistSrcParameters(mDistSrcParameters[i]);
                if (mBandwidthType[0] == GwmGWRTaskThread::BandwidthType::Adaptive)
                {
                    bwSelThread.setLower(mAdaptiveLower);
                }
                bwi = selectOptimizedBandwidth(bwSelThread, false);
                double bwi0 = mInitialBandwidthSize[i];
                emit message(QString("The newly selected bandwidth for variable %1 is %2 (last is %3, difference is %4)")
                             .arg(varName).arg(bwi).arg(bwi0).arg(abs(bwi - bwi0)));
                if (abs(bwi - mInitialBandwidthSize[i]) > mBandwidthSelectThreshold(i))
                {
                    bwChangeNo(i) = 0;
                    emit message(QString("The bandwidth for variable %1 will be continually selected in the next iteration").arg(varName));
                }
                else
                {
                    bwChangeNo(i) += 1;
                    if (bwChangeNo(i) >= mBandwidthSelectRetryTimes)
                    {
                        mBandwidthSeled[i] = BandwidthSeledType::Specified;
                        emit message(QString("The bandwidth for variable %1 seems to be converged and will be kept the same in the following iterations.").arg(varName));
                    }
                    else
                    {
                        emit message(QString("The bandwidth for variable %1 seems to be converged for %2 times. It will be continually optimized in the next %3 times.")
                                     .arg(varName).arg(bwChangeNo(i)).arg(mBandwidthSelectRetryTimes - bwChangeNo(i)));
                    }
                }
            }
            mInitialBandwidthSize[i] = bwi;

            mat S = hasHatMatrix ? mat(nDp, nDp, fill::zeros) : mat(0, 0);
            mBetas.col(i) = regressionVar(mX.col(i), yi, i, bwi, S);
            if (hasHatMatrix)
            {
                mat SArrayi = mSArray.slice(i);
                mSArray.slice(i) = S * SArrayi + S - S * mS0;
                mS0 = mS0 - SArrayi + mSArray.slice(i);
            }
            resid = mY - fitted(mX, mBetas);
            preview("PSDM/resid.txt", resid, QString("Iteration %1 Variable %2").arg(iteration - 1).arg(varName).toStdString(), true);
        }
        preview("PSDM/betas.txt", mBetas, QString("Iteration %1").arg(iteration - 1).toStdString(), true);
        RSS = rss(mY, mX, mBetas);
        criterion = (mCriterionType == CriterionType::CVR) ?
                    abs(RSS - RSS0) :
                    sqrt(abs(RSS - RSS0) / RSS);
        QString criterionName = mCriterionType == CriterionType::CVR ? "change value of RSS (CVR)" : "differential change value of RSS (dCVR)";
        emit message(QString("Iteration %1 the %2 is %3").arg(iteration).arg(criterionName).arg(criterion));
        RSS0 = RSS;
        emit message(QString("---- End of Iteration %1 ----").arg(iteration));
    }
    emit message(QString("-------- [End] Select the Optimum Bandwidths for each Independent Varialbe --------"));
    isBandwidthSizeAutoSel = false;
    if (hasHatMatrix)
    {
        GwmMultiscaleGWRTaskThread::diagnostic(RSS0);
    }
    createResultLayer();

    emit success();
}

void GwmMultiscaleGWRTaskThread::setIndepVars(const QList<GwmLayerAttributeItem *> &indepVars)
{
    GwmGWRTaskThread::setIndepVars(indepVars);
    int nVar = indepVars.size() + 1;
    mBandwidthSizePS = vec(nVar, fill::zeros) + 100.0;
    mInitialBandwidthSize = vec(nVar, fill::zeros) + 100.0;
    mBandwidthSelectThreshold = vec(nVar, fill::zeros) + 1e-6;
    mBandwidthUnit.clear();
    mBandwidthSeled.clear();
    mBandwidthType.clear();
    mBandwidthSelectionApproach.clear();
    mBandwidthKernelFunction.clear();
    mDistanceSource.clear();
    mDistSrcParameters.clear();
    mPreditorCentered.clear();
    for (int i = 0; i < nVar; i++)
    {
        mBandwidthUnit.append(QString("x1"));
        mBandwidthSeled.append(BandwidthSeledType::Null);
        mBandwidthType.append(GwmGWRTaskThread::Adaptive);
        mBandwidthSelectionApproach.append(GwmGWRTaskThread::CV);
        mBandwidthKernelFunction.append(GwmGWRTaskThread::Gaussian);
        mDistanceSource.append(GwmGWRTaskThread::CRS);
        mDistSrcParameters.append(QVariant());
        mPreditorCentered.append(true);
    }
}

void GwmMultiscaleGWRTaskThread::diagnostic(double RSS)
{
    emit message(tr("Calculating diagnostic informations..."));

    // 诊断信息
    double nDp = mX.n_rows;
    mYHat = fitted(mX, mBetas);
    mResidual = mY - mYHat;
    double RSSg = RSS;
    double sigmaHat21 = RSSg / nDp;
    double TSS = sum((mY - mean(mY)) % (mY - mean(mY)));
    double Rsquare = 1 - RSSg / TSS;

    double trS = trace(mS0);
    double trStS = trace(mS0.t() * mS0);
    double edf = nDp - 2 * trS + trStS;
    double AICc = nDp * log(sigmaHat21) + nDp * log(2 * M_PI) + nDp * ((nDp + trS) / (nDp - 2 - trS));
    double adjustRsquare = 1 - (1 - Rsquare) * (nDp - 1) / (edf - 1);

    // 保存结果
    mDiagnostic.RSS = RSSg;
    mDiagnostic.AICc = AICc;
    mDiagnostic.EDF = edf;
    mDiagnostic.RSquareAdjust = adjustRsquare;
    mDiagnostic.RSquare = Rsquare;
}

void GwmMultiscaleGWRTaskThread::createResultLayer()
{
    emit message("Creating result layer...");
    QgsVectorLayer* srcLayer = mRegressionLayer ? mRegressionLayer : mLayer;
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    layerName += QString("_PSDM");
    mResultLayer = new QgsVectorLayer(layerFileName, layerName, QStringLiteral("memory"));
    mResultLayer->setCrs(srcLayer->crs());

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

double GwmMultiscaleGWRTaskThread::selectOptimizedBandwidth(GwmBandwidthSelectTaskThread &bwSelThread, bool verbose)
{
    if (verbose)
    {
        connect(&bwSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
        connect(&bwSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
        connect(&bwSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
    }
    bwSelThread.start();
    bwSelThread.wait();
    if (verbose)
    {
        disconnect(&bwSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
        disconnect(&bwSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
        disconnect(&bwSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
    }
    return bwSelThread.getBandwidthSize();
}

vec GwmMultiscaleGWRTaskThread::distance(int focus, int variable)
{
    switch (mDistanceSource[variable])
    {
    case DistanceSourceType::Minkowski:
        return distanceMinkowski(focus, variable);
    case DistanceSourceType::DMatFile:
        return distanceDmat(focus, variable);
    default:
        return distanceCRS(focus, variable);
    }
}

vec GwmMultiscaleGWRTaskThread::distanceCRS(int focus, int variable)
{
    bool longlat = mLayer->crs().isGeographic();
    return gwDist(mDataPoints, mRegPoints, focus, 2.0, 0.0, longlat, mRegressionLayer != nullptr);
}

vec GwmMultiscaleGWRTaskThread::distanceMinkowski(int focus, int variable)
{
    QMap<QString, QVariant> parameters = mDistSrcParameters[variable].toMap();
    double p = parameters["p"].toDouble();
    double theta = parameters["theta"].toDouble();
    return gwDist(mDataPoints, mRegPoints, focus, p, theta, false, mRegressionLayer != nullptr);
}

vec GwmMultiscaleGWRTaskThread::distanceDmat(int focus, int variable)
{
    QString filename = mDistSrcParameters[variable].toString();
    qint64 featureCount = mFeatureList.size();
    QFile dmat(filename);
    if (dmat.open(QFile::QIODevice::ReadOnly))
    {
        qint64 basePos = 2 * sizeof (int);
        dmat.seek(basePos + focus * featureCount * sizeof (double));
        QByteArray values = dmat.read(featureCount * sizeof (double));
        return vec((double*)values.data(), featureCount);
    }
    else
    {
        return vec(featureCount, fill::zeros);
    }
}

bool GwmMultiscaleGWRTaskThread::regressionAllSerial()
{
    bool isAllCorrect = true;
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    if (hasHatMatrix)
    {
        mat betas(nVar, nDp, fill::zeros), betasSE(nVar, nDp, fill::zeros);
        mat ci, si;
        vec shat(2, fill::zeros), q(nDp, fill::zeros);
        for (int i = 0; i < mFeatureList.size(); i++)
        {
            try
            {
                vec dist = distance(i, 0);
                vec weight = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction[0], mBandwidthType[0] == BandwidthType::Adaptive) % mWeightMask;
                betas.col(i) = gwRegHatmatrix(mX, mY, weight, i, ci, si);
                betasSE.col(i) = sum(ci % ci, 1);
                shat(0) += si(0, i);
                shat(1) += det(si * trans(si));
                vec p = -trans(si);
                p(i) += 1.0;
                q += p % p;
                mS0.row(i) = si;
                mC.slice(i) = ci;
                emit tick(i + 1, mFeatureList.size());
            } catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        if (isAllCorrect)
        {
            mSHat = shat;
            mQDiag = q;
            mBetas = betas.t();
            mBetasSE = betasSE.t();
        }
    }
    else
    {
        mat betas(nVar, nDp, fill::zeros);
        for (int i = 0; i < mFeatureList.size(); i++)
        {
            try {
                vec dist = distance(i, 0);
                vec weight = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction[0], mBandwidthType[0] == BandwidthType::Adaptive);
                betas.col(i) = gwReg(mX, mY, weight, i);
                emit tick(i + 1, mFeatureList.size());
            } catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        if (isAllCorrect)
        {
            mBetas = betas.t();
        }
    }
    return isAllCorrect;
}

vec GwmMultiscaleGWRTaskThread::regressionVar(const mat &x, const vec &y, const int var, double bw, mat &S)
{
    bool isAllCorrect = true;
    arma::uword nDp = x.n_rows, nVar = x.n_cols;
    if (hasHatMatrix)
    {
        mat betas(nVar, nDp, fill::zeros);
        mat ci, si;
        vec shat(2, fill::zeros), q(nDp, fill::zeros);
        for (int i = 0; i < mFeatureList.size(); i++)
        {
            try
            {
                vec d = distance(i, var);
                vec w = gwWeight(d, bw, mBandwidthKernelFunction[var], mBandwidthType[var] == BandwidthType::Adaptive) % mWeightMask;
                betas.col(i) = gwRegHatmatrix(x, y, w, i, ci, si);
                shat(0) += si(0, i);
                shat(1) += det(si * trans(si));
                vec p = -trans(si);
                p(i) += 1.0;
                q += p % p;
                S.row(i) = si;
            } catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        if (isAllCorrect)
        {
            return betas.t();
        }
    }
    else
    {
        mat betas(nVar, nDp, fill::zeros);
        for (int i = 0; i < mFeatureList.size(); i++)
        {
            try {
                vec d = distance(i, var);
                vec w = gwWeight(d, bw, mBandwidthKernelFunction[var], mBandwidthType[var] == BandwidthType::Adaptive);
                betas.col(i) = gwReg(x, y, w, i);
            } catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        if (isAllCorrect)
        {
            return betas.t();
        }
    }
    return vec(nDp, fill::zeros);
}

int GwmMultiscaleGWRTaskThread::adaptiveLower() const
{
    return mAdaptiveLower;
}

void GwmMultiscaleGWRTaskThread::setAdaptiveLower(int adaptiveLower)
{
    mAdaptiveLower = adaptiveLower;
}

QList<QVariant> GwmMultiscaleGWRTaskThread::distanceParameter() const
{
    return mDistSrcParameters;
}

void GwmMultiscaleGWRTaskThread::setDistanceParameter(const QList<QVariant> &distanceParameter)
{
    mDistSrcParameters = distanceParameter;
}

void GwmMultiscaleGWRTaskThread::setDistanceParameter(const int index, const QVariant value)
{
    mDistSrcParameters[index] = value;
}

QList<GwmGWRTaskThread::DistanceSourceType> GwmMultiscaleGWRTaskThread::distanceSource() const
{
    return mDistanceSource;
}

void GwmMultiscaleGWRTaskThread::setDistanceSource(const QList<GwmGWRTaskThread::DistanceSourceType> &distanceSource)
{
    mDistanceSource = distanceSource;
}

void GwmMultiscaleGWRTaskThread::setDistanceSource(const int index, const GwmGWRTaskThread::DistanceSourceType value)
{
    mDistanceSource[index] = value;
}

QList<GwmGWRTaskThread::KernelFunction> GwmMultiscaleGWRTaskThread::bandwidthKernel() const
{
    return mBandwidthKernelFunction;
}

void GwmMultiscaleGWRTaskThread::setBandwidthKernel(const QList<GwmGWRTaskThread::KernelFunction> &bandwidthKernel)
{
    mBandwidthKernelFunction = bandwidthKernel;
}

void GwmMultiscaleGWRTaskThread::setBandwidthKernel(const int index, const GwmGWRTaskThread::KernelFunction value)
{
    mBandwidthKernelFunction[index] = value;
}

QList<GwmGWRTaskThread::BandwidthSelectionApproach> GwmMultiscaleGWRTaskThread::bandwidthSelectionApproach() const
{
    return mBandwidthSelectionApproach;
}

void GwmMultiscaleGWRTaskThread::setBandwidthSelectionApproach(const QList<GwmGWRTaskThread::BandwidthSelectionApproach> &bandwidthSelectionApproach)
{
    mBandwidthSelectionApproach = bandwidthSelectionApproach;
}

void GwmMultiscaleGWRTaskThread::setBandwidthSelectionApproach(const int index, const GwmGWRTaskThread::BandwidthSelectionApproach value)
{
    mBandwidthSelectionApproach[index] = value;
}

QList<GwmGWRTaskThread::BandwidthType> GwmMultiscaleGWRTaskThread::bandwidthType() const
{
    return mBandwidthType;
}

void GwmMultiscaleGWRTaskThread::setBandwidthType(const QList<GwmGWRTaskThread::BandwidthType> &bandwidthType)
{
    mBandwidthType = bandwidthType;
}

void GwmMultiscaleGWRTaskThread::setBandwidthType(const int index, const GwmGWRTaskThread::BandwidthType value)
{
    mBandwidthType[index] = value;
}

QList<QString> GwmMultiscaleGWRTaskThread::bandwidthUnit() const
{
    return mBandwidthUnit;
}

void GwmMultiscaleGWRTaskThread::setBandwidthUnit(const QList<QString> &bandwidthUnit)
{
    mBandwidthUnit = bandwidthUnit;
}

void GwmMultiscaleGWRTaskThread::setBandwidthUnit(const int index, const QString value)
{
    mBandwidthUnit[index] = value;
}

int GwmMultiscaleGWRTaskThread::maxIteration() const
{
    return mMaxIteration;
}

void GwmMultiscaleGWRTaskThread::setMaxIteration(int maxIteration)
{
    mMaxIteration = maxIteration;
}

double GwmMultiscaleGWRTaskThread::criterionThreshold() const
{
    return mCriterionThreshold;
}

void GwmMultiscaleGWRTaskThread::setCriterionThreshold(double criterionThreshold)
{
    mCriterionThreshold = criterionThreshold;
}

GwmMultiscaleGWRTaskThread::CriterionType GwmMultiscaleGWRTaskThread::criterionType() const
{
    return mCriterionType;
}

void GwmMultiscaleGWRTaskThread::setCriterionType(const GwmMultiscaleGWRTaskThread::CriterionType &criterionType)
{
    mCriterionType = criterionType;
}

int GwmMultiscaleGWRTaskThread::bandwidthSelectRetryTimes() const
{
    return mBandwidthSelectRetryTimes;
}

void GwmMultiscaleGWRTaskThread::setBandwidthSelectRetryTimes(int bandwidthSelectRetryTimes)
{
    mBandwidthSelectRetryTimes = bandwidthSelectRetryTimes;
}

vec GwmMultiscaleGWRTaskThread::bandwidthSelectThreshold() const
{
    return mBandwidthSelectThreshold;
}

void GwmMultiscaleGWRTaskThread::setBandwidthSelectThreshold(const vec &bandwidthSelectThreshold)
{
    mBandwidthSelectThreshold = bandwidthSelectThreshold;
}

void GwmMultiscaleGWRTaskThread::setBandwidthSelectThreshold(const int index, const double value)
{
    mBandwidthSelectThreshold[index] = value;
}

QList<bool> GwmMultiscaleGWRTaskThread::preditorCentered() const
{
    return mPreditorCentered;
}

void GwmMultiscaleGWRTaskThread::setPreditorCentered(const QList<bool> &preditorCentered)
{
    mPreditorCentered = preditorCentered;
}

void GwmMultiscaleGWRTaskThread::setPreditorCentered(const int index, const bool value)
{
    mPreditorCentered[index] = value;
}

QList<GwmMultiscaleGWRTaskThread::BandwidthSeledType> GwmMultiscaleGWRTaskThread::bandwidthSeled() const
{
    return mBandwidthSeled;
}

void GwmMultiscaleGWRTaskThread::setBandwidthSeled(const QList<BandwidthSeledType> &bandwidthSeled)
{
    mBandwidthSeled = bandwidthSeled;
}

void GwmMultiscaleGWRTaskThread::setBandwidthSeled(const int index, const GwmMultiscaleGWRTaskThread::BandwidthSeledType value)
{
    mBandwidthSeled[index] = value;
}

vec GwmMultiscaleGWRTaskThread::initialBandwidthSize() const
{
    return mInitialBandwidthSize;
}

void GwmMultiscaleGWRTaskThread::setInitialBandwidthSize(const vec &initialBandwidthSize)
{
    mInitialBandwidthSize = initialBandwidthSize;
}

void GwmMultiscaleGWRTaskThread::setInitialBandwidthSize(const int index, const double value)
{
    mInitialBandwidthSize(index) = value;
}

#include "gwmmultiscalegwrtaskthread.h"

#include <exception>
#include "GWmodel/GWmodel.h"

using namespace std;

GwmDiagnostic GwmMultiscaleGWRTaskThread::CalcDiagnostic(const mat &x, const vec &y, const mat &S0, double RSS)
{
    // 诊断信息
    double nDp = x.n_rows;
    double RSSg = RSS;
    double sigmaHat21 = RSSg / nDp;
    double TSS = sum((y - mean(y)) % (y - mean(y)));
    double Rsquare = 1 - RSSg / TSS;

    double trS = trace(S0);
    double trStS = trace(S0.t() * S0);
    double edf = nDp - 2 * trS + trStS;
    double AICc = nDp * log(sigmaHat21) + nDp * log(2 * M_PI) + nDp * ((nDp + trS) / (nDp - 2 - trS));
    double adjustRsquare = 1 - (1 - Rsquare) * (nDp - 1) / (edf - 1);

    // 保存结果
    GwmDiagnostic diagnostic;
    diagnostic.RSS = RSSg;
    diagnostic.AICc = AICc;
    diagnostic.EDF = edf;
    diagnostic.RSquareAdjust = adjustRsquare;
    diagnostic.RSquare = Rsquare;
    return diagnostic;
}

GwmMultiscaleGWRTaskThread::GwmMultiscaleGWRTaskThread()
    : GwmSpatialMultiscaleAlgorithm()
{
}

void GwmMultiscaleGWRTaskThread::run()
{

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

    // ***********************
    // Intialize the bandwidth
    // ***********************
    for (uword i = 0; i < nVar; i++)
    {
        if (mBandwidthInitilize[i] == BandwidthInitilizeType::Null)
        {
            GwmBandwidthWeight* bw0 = bandwidth(i);
            bool adaptive = bw0->adaptive();
            GwmBandwidthSizeSelector selector;
            selector.setBandwidth(bw0);
            selector.setLower(adaptive ? mAdaptiveLower : 0.0);
            selector.setUpper(adaptive ? mDataLayer->featureCount() : findMaxDistance(i));
            mBandwidthSizeCriterion = &GwmMultiscaleGWRTaskThread::mBandwidthSizeCriterionVarCVSerial;
            mBandwidthSelectionCurrentIndex = i;
            GwmBandwidthWeight* bw = selector.optimize(this);
            if (bw)
            {
                mSpatialWeights[i].setWeight(bw);
            }
        }
    }

    // *****************************************************
    // Calculate the initial beta0 from the above bandwidths
    // *****************************************************
    mInitSpatialWeight.setDistance(mSpatialWeights[0].distance());
    GwmBandwidthWeight* bw0 = bandwidth(0);
    bool adaptive = bw0->adaptive();
    mBandwidthSizeCriterion = &GwmMultiscaleGWRTaskThread::mBandwidthSizeCriterionAllCVSerial;
    GwmBandwidthSizeSelector initBwSelector;
    initBwSelector.setBandwidth(bw0);
    initBwSelector.setLower(adaptive ? mAdaptiveLower : 0.0);
    initBwSelector.setUpper(adaptive ? mDataLayer->featureCount() : findMaxDistance(0));
    GwmBandwidthWeight* initBw = initBwSelector.optimize(this);
    if (!initBw)
    {
        emit error(tr("Cannot select initial bandwidth."));
        return;
    }
    mInitSpatialWeight.setWeight(initBw);

    // 初始化诊断信息矩阵
    if (mHasHatMatrix)
    {
        mS0 = mat(nDp, nDp, fill::zeros);
        mSArray = cube(nDp, nDp, nVar, fill::zeros);
        mC = cube(nVar, nDp, nDp, fill::zeros);
    }

    mBetas = regression(mX, mY);

    if (mHasHatMatrix)
    {
        mDiagnostic = CalcDiagnostic(mX, mY, mS0, mRSS0);
        vec yhat = fitted(mX, mBetas);
        vec residual = mY - yhat;
        createResultLayer({
            qMakePair(QString("%1"), mBetas),
            qMakePair(QString("yhat"), yhat),
            qMakePair(QString("residual"), residual)
        });
    }
    else
    {
        createResultLayer({
            qMakePair(QString("%1"), mBetas)
        });
    }

    emit success();
}

mat GwmMultiscaleGWRTaskThread::regression(const mat &x, const vec &y)
{
    uword nDp = x.n_rows, nVar = x.n_cols;
    mat betas = (this->*mRegressionAll)(x, y);

    if (mHasHatMatrix)
    {
        mat idm(nVar, nVar, fill::eye);
        for (uword i = 0; i < nVar; ++i)
        {
            for (uword j = 0; j < nDp; ++j)
            {
                mSArray.slice(i).row(j) = x(j, i) * (idm.row(i) * mC.slice(j));
            }
        }
    }

    // ***********************************************************
    // Select the optimum bandwidths for each independent variable
    // ***********************************************************
    emit message(QString("-------- Select the Optimum Bandwidths for each Independent Varialbe --------"));
    uvec bwChangeNo(nVar, fill::zeros);
    vec resid = y - fitted(x, betas);
    double RSS0 = sum(resid % resid), RSS1 = DBL_MAX;
    double criterion = DBL_MAX;
    for (int iteration = 1; iteration <= mMaxIteration && criterion > mCriterionThreshold; iteration++)
    {
        for (uword i = 0; i < nVar; i++)
        {
            QString varName = i == 0 ? QStringLiteral("Intercept") : mIndepVars[i-1].name;
            GwmBandwidthWeight* bwi;
            vec fi = betas.col(i) % x.col(i);
            vec yi = resid + fi;
            if (mBandwidthInitilize[i] == BandwidthInitilizeType::Specified)
            {
                bwi = bandwidth(i);
            }
            else
            {
                emit message(QString("Now select an optimum bandwidth for the variable: %1").arg(varName));
                GwmBandwidthWeight* bwi0 = bandwidth(i);
                bool adaptive = bwi0->adaptive();
                GwmBandwidthSizeSelector selector;
                selector.setBandwidth(bwi0);
                selector.setLower(adaptive ? mAdaptiveLower : 0.0);
                selector.setUpper(adaptive ? mDataLayer->featureCount() : findMaxDistance(i));
                mBandwidthSizeCriterion = &GwmMultiscaleGWRTaskThread::mBandwidthSizeCriterionVarCVSerial;
                mBandwidthSelectionCurrentIndex = i;
                bwi = selector.optimize(this);
                double bwi0s = bwi0->bandwidth(), bwi1s = bwi->bandwidth();
                emit message(QString("The newly selected bandwidth for variable %1 is %2 (last is %3, difference is %4)")
                             .arg(varName).arg(bwi1s).arg(bwi0s).arg(abs(bwi1s - bwi0s)));
                if (abs(bwi1s - bwi0s) > mBandwidthSelectThreshold[i])
                {
                    bwChangeNo(i) = 0;
                    emit message(QString("The bandwidth for variable %1 will be continually selected in the next iteration").arg(varName));
                }
                else
                {
                    bwChangeNo(i) += 1;
                    if (bwChangeNo(i) >= mBandwidthSelectRetryTimes)
                    {
                        mBandwidthInitilize[i] = BandwidthInitilizeType::Specified;
                        emit message(QString("The bandwidth for variable %1 seems to be converged and will be kept the same in the following iterations.").arg(varName));
                    }
                    else
                    {
                        emit message(QString("The bandwidth for variable %1 seems to be converged for %2 times. It will be continually optimized in the next %3 times.")
                                     .arg(varName).arg(bwChangeNo(i)).arg(mBandwidthSelectRetryTimes - bwChangeNo(i)));
                    }
                }
            }
            mSpatialWeights[i].setWeight(bwi);

            mat S;
            mBetas.col(i) = (this->*mRegressionVar)(x.col(i), yi, i, S);
            if (mHasHatMatrix)
            {
                mat SArrayi = mSArray.slice(i);
                mSArray.slice(i) = S * SArrayi + S - S * mS0;
                mS0 = mS0 - SArrayi + mSArray.slice(i);
            }
            resid = betas - fitted(x, betas);
        }
        RSS1 = RSS(y, x, betas);
        criterion = (mCriterionType == BackFittingCriterionType::CVR) ?
                    abs(RSS1 - RSS0) :
                    sqrt(abs(RSS1 - RSS0) / RSS1);
        QString criterionName = mCriterionType == BackFittingCriterionType::CVR ? "change value of RSS (CVR)" : "differential change value of RSS (dCVR)";
        emit message(QString("Iteration %1 the %2 is %3").arg(iteration).arg(criterionName).arg(criterion));
        RSS0 = RSS1;
        emit message(QString("---- End of Iteration %1 ----").arg(iteration));
    }
    emit message(QString("-------- [End] Select the Optimum Bandwidths for each Independent Varialbe --------"));
    mRSS0 = RSS0;
    return betas;
}

bool GwmMultiscaleGWRTaskThread::isValid()
{
    int nVar = mIndepVars.size() + 1;
    if (mSpatialWeights.size() != nVar)
        return false;

    if (mBandwidthInitilize.size() != nVar)
        return false;

    if (mBandwidthSelectionApproach.size() != nVar)
        return false;

    if (mPreditorCentered.size() != nVar)
        return false;

    if (mBandwidthSelectThreshold.size() != nVar)
        return false;

    return true;
}

void GwmMultiscaleGWRTaskThread::initPoints()
{
    int nDp = mDataLayer->featureCount();
    mDataPoints = mat(nDp, 2, fill::zeros);
    QgsFeatureIterator iterator = mDataLayer->getFeatures();
    QgsFeature f;
    for (int i = 0; i < nDp && iterator.nextFeature(f); i++)
    {
        QgsPointXY centroPoint = f.geometry().centroid().asPoint();
        mDataPoints(i, 0) = centroPoint.x();
        mDataPoints(i, 1) = centroPoint.y();
    }
    // Regression Layer
    if (hasRegressionLayer())
    {
        int nRp = mRegressionLayer->featureCount();
        mRegressionPoints = mat(nRp, 2, fill::zeros);
        QgsFeatureIterator iterator = mRegressionLayer->getFeatures();
        QgsFeature f;
        for (int i = 0; i < nRp && iterator.nextFeature(f); i++)
        {
            QgsPointXY centroPoint = f.geometry().centroid().asPoint();
            mRegressionPoints(i, 0) = centroPoint.x();
            mRegressionPoints(i, 1) = centroPoint.y();
        }
    }
    else mRegressionPoints = mDataPoints;
}

void GwmMultiscaleGWRTaskThread::initXY(mat &x, mat &y, const GwmVariable &depVar, const QList<GwmVariable> &indepVars)
{
    int nDp = mDataLayer->featureCount(), nVar = indepVars.size() + 1;
    // Data layer and X,Y
    x = mat(nDp, nVar, fill::zeros);
    y = vec(nDp, fill::zeros);
    QgsFeatureIterator iterator = mDataLayer->getFeatures();
    QgsFeature f;
    bool ok = false;
    for (int i = 0; iterator.nextFeature(f); i++)
    {

        double vY = f.attribute(depVar.name).toDouble(&ok);
        if (ok)
        {
            y(i) = vY;
            x(i, 0) = 1.0;
            for (int k = 0; k < indepVars.size(); k++)
            {
                double vX = f.attribute(indepVars[k].name).toDouble(&ok);
                if (ok) x(i, k + 1) = vX;
                else emit error(tr("Independent variable value cannot convert to a number. Set to 0."));
            }
        }
        else emit error(tr("Dependent variable value cannot convert to a number. Set to 0."));
    }
}

void GwmMultiscaleGWRTaskThread::createResultLayer(initializer_list<CreateResultLayerDataItem> data)
{
    emit message("Creating result layer...");
    QgsVectorLayer* srcLayer = mRegressionLayer ? mRegressionLayer : mDataLayer;
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    layerName += QStringLiteral("_GWR");
    mResultLayer = new QgsVectorLayer(layerFileName, layerName, QStringLiteral("memory"));
    mResultLayer->setCrs(srcLayer->crs());

    // 设置字段
    QgsFields fields;
    for (QPair<QString, const mat&> item : data)
    {
        QString title = item.first;
        const mat& value = item.second;
        if (value.n_cols > 1)
        {
            for (uword k = 0; k < value.n_cols; k++)
            {
                QString variableName = k == 0 ? QStringLiteral("Intercept") : mIndepVars[k - 1].name;
                QString fieldName = title.arg(variableName);
                fields.append(QgsField(fieldName, QVariant::Double, QStringLiteral("double")));
            }
        }
        else
        {
            fields.append(QgsField(title, QVariant::Double, QStringLiteral("double")));
        }
    }
    mResultLayer->dataProvider()->addAttributes(fields.toList());
    mResultLayer->updateFields();

    // 设置要素几何
    mResultLayer->startEditing();
    QgsFeatureIterator iterator = srcLayer->getFeatures();
    QgsFeature f;
    for (int i = 0; iterator.nextFeature(f); i++)
    {
        QgsFeature feature(fields);
        feature.setGeometry(f.geometry());

        // 设置属性
        int k = 0;
        for (QPair<QString, const mat&> item : data)
        {
            for (uword d = 0; d < item.second.n_cols; d++)
            {
                feature.setAttribute(k, item.second(i, d));
                k++;
            }
        }

        mResultLayer->addFeature(feature);
    }
    mResultLayer->commitChanges();
}


mat GwmMultiscaleGWRTaskThread::regressionAllSerial(const mat& x, const vec& y)
{
    int nDp = x.n_rows, nVar = x.n_cols;
    mat betas(nVar, nDp, fill::zeros);
    if (mHasHatMatrix)
    {
        mat ci, si, betasSE(nVar, nDp, fill::zeros);
        for (int i = 0; i < nDp; i++)
        {
            vec w = mSpatialWeights[0].spatialWeight(mDataPoints.row(i), mDataPoints);
            mat xtw = trans(x.each_col() % w);
            mat xtwx = xtw * x;
            mat xtwy = xtw * y;
            try
            {
                mat xtwx_inv = inv_sympd(xtwx);
                betas.col(i) = xtwx_inv * xtwy;
                mat ci = xtwx_inv * xtw;
                betasSE.col(i) = sum(ci % ci, 1);
                mat si = x.row(i) * ci;
                mS0.row(i) = si;
                mC.slice(i) = ci;
                emit tick(i + 1, nDp);
            } catch (exception e) {
                emit error(e.what());
            }
        }
        mBetasSE = betasSE.t();
    }
    else
    {
        for (int i = 0; i < nDp; i++)
        {
            vec w = mSpatialWeights[0].spatialWeight(mDataPoints.row(i), mDataPoints);
            mat xtw = trans(x.each_col() % w);
            mat xtwx = xtw * x;
            mat xtwy = xtw * y;
            try
            {
                mat xtwx_inv = inv_sympd(xtwx);
                betas.col(i) = xtwx_inv * xtwy;
                emit tick(i + 1, nDp);
            } catch (exception e) {
                emit error(e.what());
            }
        }
    }
    mBetas = betas.t();
    return betas;
}

vec GwmMultiscaleGWRTaskThread::regressionVarSerial(const mat &x, const vec &y, const int var, mat &S)
{
    int nDp = x.n_rows, nVar = x.n_cols;
    mat betas(nVar, nDp, fill::zeros);
    if (mHasHatMatrix)
    {
        mat ci, si;
        S = mat(mHasHatMatrix ? nDp : 1, nDp, fill::zeros);
        for (int i = 0; i < nDp; i++)
        {
            vec w = mSpatialWeights[var].spatialWeight(mDataPoints.row(i), mDataPoints);
            mat xtw = trans(x.col(var) % w);
            mat xtwx = xtw * x.col(var);
            mat xtwy = xtw * y;
            try
            {
                mat xtwx_inv = inv_sympd(xtwx);
                betas.col(i) = xtwx_inv * xtwy;
                mat ci = xtwx_inv * xtw;
                mat si = x(i, var) * ci;
                S.row(i) = si;
                emit tick(i + 1, nDp);
            } catch (exception e) {
                emit error(e.what());
            }
        }
    }
    else
    {
        for (int i = 0; i < nDp; i++)
        {
            vec w = mSpatialWeights[var].spatialWeight(mDataPoints.row(i), mDataPoints);
            mat xtw = trans(x.col(var) % w);
            mat xtwx = xtw * x.col(var);
            mat xtwy = xtw * y;
            try
            {
                mat xtwx_inv = inv_sympd(xtwx);
                betas.col(i) = xtwx_inv * xtwy;
                emit tick(i + 1, nDp);
            } catch (exception e) {
                emit error(e.what());
            }
        }
    }
    return betas.t();
}

double GwmMultiscaleGWRTaskThread::findMaxDistance(int var)
{
    int nDp = mDataPoints.n_rows;
    double maxD = 0.0;
    for (int i = 0; i < nDp; i++)
    {
        double d = max(mSpatialWeights[var].distance()->distance(mDataPoints.row(i), mDataPoints));
        maxD = d > maxD ? d : maxD;
    }
    return maxD;
}

double GwmMultiscaleGWRTaskThread::mBandwidthSizeCriterionAllCVSerial(GwmBandwidthWeight *bandwidthWeight)
{
    uword nDp = mDataPoints.n_rows;
    vec shat(2, fill::zeros);
    double cv = 0.0;
    for (uword i = 0; i < nDp; i++)
    {
        vec d = mInitSpatialWeight.distance()->distance(mDataPoints.row(i), mDataPoints);
        vec w = bandwidthWeight->weight(d);
        w(i) = 0.0;
        mat xtw = trans(mX.each_col() % w);
        mat xtwx = xtw * mX;
        mat xtwy = xtw * mY;
        try
        {
            mat xtwx_inv = inv_sympd(xtwx);
            vec beta = xtwx_inv * xtwy;
            double res = mY(i) - det(mX.row(i) * beta);
            cv += res * res;
        }
        catch (...)
        {
            return DBL_MAX;
        }
    }
    QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
            .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
            .arg(bandwidthWeight->bandwidth())
            .arg(cv);
    emit message(msg);
    return cv;
}

double GwmMultiscaleGWRTaskThread::mBandwidthSizeCriterionAllAICSerial(GwmBandwidthWeight *bandwidthWeight)
{
    uword nDp = mDataPoints.n_rows, nVar = mIndepVars.size() + 1;
    mat betas(nVar, nDp, fill::zeros);
    vec shat(2, fill::zeros);
    for (uword i = 0; i < nDp; i++)
    {
        vec d = mInitSpatialWeight.distance()->distance(mDataPoints.row(i), mDataPoints);
        vec w = bandwidthWeight->weight(d);
        mat xtw = trans(mX.each_col() % w);
        mat xtwx = xtw * mX;
        mat xtwy = xtw * mY;
        try
        {
            mat xtwx_inv = inv_sympd(xtwx);
            betas.col(i) = xtwx_inv * xtwy;
            mat ci = xtwx_inv * xtw;
            mat si = mX.row(i) * ci;
            shat(0) += si(0, i);
            shat(1) += det(si * si.t());
        }
        catch (std::exception e)
        {
            return DBL_MAX;
        }
    }
    double value = GwmMultiscaleGWRTaskThread::AICc(mX, mY, betas.t(), shat);
    QString msg = QString(tr("%1 bandwidth: %2 (AIC Score: %3)"))
            .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
            .arg(bandwidthWeight->bandwidth())
            .arg(value);
    emit message(msg);
    return value;
}

double GwmMultiscaleGWRTaskThread::mBandwidthSizeCriterionVarCVSerial(GwmBandwidthWeight *bandwidthWeight)
{
    int var = mBandwidthSelectionCurrentIndex;
    uword nDp = mDataPoints.n_rows;
    vec shat(2, fill::zeros);
    double cv = 0.0;
    for (uword i = 0; i < nDp; i++)
    {
        vec d = mSpatialWeights[var].distance()->distance(mDataPoints.row(i), mDataPoints);
        vec w = bandwidthWeight->weight(d);
        w(i) = 0.0;
        mat xtw = trans(mX.col(var) % w);
        mat xtwx = xtw * mX.col(var);
        mat xtwy = xtw * mY;
        try
        {
            mat xtwx_inv = inv_sympd(xtwx);
            vec beta = xtwx_inv * xtwy;
            double res = mY(i) - det(mX(i, var) * beta);
            cv += res * res;
        }
        catch (...)
        {
            return DBL_MAX;
        }
    }
    QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
            .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
            .arg(bandwidthWeight->bandwidth())
            .arg(cv);
    emit message(msg);
    return cv;
}

double GwmMultiscaleGWRTaskThread::mBandwidthSizeCriterionVarAICSerial(GwmBandwidthWeight *bandwidthWeight)
{
    int var = mBandwidthSelectionCurrentIndex;
    uword nDp = mDataPoints.n_rows, nVar = mIndepVars.size() + 1;
    mat betas(nVar, nDp, fill::zeros);
    vec shat(2, fill::zeros);
    for (uword i = 0; i < nDp; i++)
    {
        vec d = mInitSpatialWeight.distance()->distance(mDataPoints.row(i), mDataPoints);
        vec w = bandwidthWeight->weight(d);
        mat xtw = trans(mX.col(var) % w);
        mat xtwx = xtw * mX.col(var);
        mat xtwy = xtw * mY;
        try
        {
            mat xtwx_inv = inv_sympd(xtwx);
            betas.col(i) = xtwx_inv * xtwy;
            mat ci = xtwx_inv * xtw;
            mat si = mX(i, var) * ci;
            shat(0) += si(0, i);
            shat(1) += det(si * si.t());
        }
        catch (std::exception e)
        {
            return DBL_MAX;
        }
    }
    double value = GwmMultiscaleGWRTaskThread::AICc(mX.col(var), mY, betas.t(), shat);
    QString msg = QString(tr("%1 bandwidth: %2 (AIC Score: %3)"))
            .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
            .arg(bandwidthWeight->bandwidth())
            .arg(value);
    emit message(msg);
    return value;
}

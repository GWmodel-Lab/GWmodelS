#include "gwmgtwralgorithm.h"

#include <omp.h>

#include <qgsmemoryproviderutils.h>

GwmDiagnostic GwmGTWRAlgorithm::CalcDiagnostic(const mat &x, const vec &y, const mat &betas, const vec &shat)
{
    vec r = y - sum(betas % x, 1);
    double rss = sum(r % r);
    int n = x.n_rows;
    double AIC = n * log(rss / n) + n * log(2 * datum::pi) + n + shat(0);
    double AICc = n * log(rss / n) + n * log(2 * datum::pi) + n * ((n + shat(0)) / (n - 2 - shat(0)));
    double edf = n - 2 * shat(0) + shat(1);
    double enp = 2 * shat(0) - shat(1);
    double yss = sum((y - mean(y)) % (y - mean(y)));
    double r2 = 1 - rss / yss;
    double r2_adj = 1 - (1 - r2) * (n - 1) / (edf - 1);
    return { rss, AIC, AICc, enp, edf, r2, r2_adj };
}

GwmGTWRAlgorithm::GwmGTWRAlgorithm() : GwmSpatialTemporalMonoscaleAlgorithm()
{

}

//void GwmGTWRAlgorithm::setCanceled(bool canceled)
//{
//    mBandwidthSizeSelector.setCanceled(canceled);
//    mSTWeight.distance()->setCanceled(canceled);
//    return GwmTaskThread::setCanceled(canceled);
//}

void GwmGTWRAlgorithm::run()
{
    if(!checkCanceled())
    {
        // 点位初始化
        emit message(QString(tr("Setting data points")) + (hasRegressionLayer() ? tr(" and regression points") : "") + ".");
        initPoints();
    }
    if(!checkCanceled())
    {
        emit message(QString(tr("Setting X and Y...")));
        initXY(mX, mY, mDepVar, mIndepVars);
    }

    // 优选带宽
    if (!hasRegressionLayer() && mIsAutoselectBandwidth && !checkCanceled())
    {
        emit message(QString(tr("Automatically selecting bandwidth ...")));
        emit tick(0, 0);
        GwmBandwidthWeight* bandwidthWeight0 = mSTWeight.weight<GwmBandwidthWeight>();
        mBandwidthSizeSelector.setBandwidth(bandwidthWeight0);
        double lower = bandwidthWeight0->adaptive() ? 20 : 0.0;
        double upper = bandwidthWeight0->adaptive() ? mDataPoints.n_rows : mSTWeight.distance()->maxDistance();
        mBandwidthSizeSelector.setLower(lower);
        mBandwidthSizeSelector.setUpper(upper);
        GwmBandwidthWeight* bandwidthWeight = mBandwidthSizeSelector.optimize(this);
        if (bandwidthWeight && !checkCanceled())
        {
            mSTWeight.setWeight(bandwidthWeight);
            // 绘图
            QVariant data = QVariant::fromValue(mBandwidthSizeSelector.bandwidthCriterion());
            emit plot(data, &GwmBandwidthSizeSelector::PlotBandwidthResult);
        }
    }

    if (mHasHatMatrix && !checkCanceled())
    {
        emit message(tr("Calibrating..."));
        uword nDp = mDataPoints.n_rows;
        mBetas = regression(mX, mY);
        mDiagnostic = CalcDiagnostic(mX, mY, mBetas, mSHat);
        double trS = mSHat(0), trStS = mSHat(1);
        double sigmaHat = mDiagnostic.RSS / (nDp - 2 * trS + trStS);
        mBetasSE = sqrt(sigmaHat * mBetasSE);
        vec yhat = Fitted(mX, mBetas);
        vec res = mY - yhat;
        vec stu_res = res / sqrt(sigmaHat * mQDiag);
        mat betasTV = mBetas / mBetasSE;
        vec dybar2 = (mY - mean(mY)) % (mY - mean(mY));
        vec dyhat2 = (mY - yhat) % (mY - yhat);
        vec localR2 = vec(nDp, fill::zeros);
        for (uword i = 0; i < nDp & !checkCanceled(); i++)
        {
            vec w = mSTWeight.weightVector(i);
            double tss = sum(dybar2 % w);
            double rss = sum(dyhat2 % w);
            localR2(i) = (tss - rss) / tss;
        }
        createResultLayer({
            qMakePair(QString("%1"), mBetas),
            qMakePair(QString("y"), mY),
            qMakePair(QString("yhat"), yhat),
            qMakePair(QString("residual"), res),
            qMakePair(QString("Stud_residual"), stu_res),
            qMakePair(QString("%1_SE"), mBetasSE),
            qMakePair(QString("%1_TV"), betasTV),
            qMakePair(QString("localR2"), localR2)
        });
    }
    else
    {
        if(!checkCanceled())
        {
            mBetas = regression(mX, mY);
            CreateResultLayerData resultLayerData;
            if (mHasRegressionLayerXY && mHasPredict)
            {
                vec yhat = Fitted(mRegressionLayerX, mBetas);
                vec residual = mRegressionLayerY - yhat;
                resultLayerData = {
                    qMakePair(QString(mDepVar.name), mRegressionLayerY),
                    qMakePair(QString("%1"), mBetas),
                    qMakePair(QString("yhat"), yhat),
                    qMakePair(QString("residual"), residual)
                };
            }
            else
            {
                resultLayerData = {
                    qMakePair(QString("%1"), mBetas)
                };
            }
            createResultLayer(resultLayerData);
        }
    }

    if(!checkCanceled()) emit success();
    else return;
}


bool GwmGTWRAlgorithm::isValid()
{
    return true;
}

arma::mat GwmGTWRAlgorithm::regression(const arma::mat &x, const arma::vec &y)
{
    if(!checkCanceled())
    {
        if (mHasHatMatrix)
            return regressionHatmatrixSerial(x, y, mBetasSE, mSHat, mQDiag);
        else return regressionSerial(x, y);
    }
}

void GwmGTWRAlgorithm::initPoints()
{
    int nDp = mDataLayer->featureCount();
    mDataPoints = mat(nDp, 2, fill::zeros);
    mDataTimeStamp = vec(nDp, fill::zeros);
    QgsFeatureIterator iterator = mDataLayer->getFeatures();
    QgsFeature f;
    for (int i = 0; i < nDp && iterator.nextFeature(f); i++)
    {
        QgsPointXY centroPoint = f.geometry().centroid().asPoint();
        mDataPoints(i, 0) = centroPoint.x();
        mDataPoints(i, 1) = centroPoint.y();
        // 提取时间
        mDataTimeStamp(i) = f.attribute(mTimeVar.index).toDouble();
    }
    // Regression Layer
    if (hasRegressionLayer())
    {
        int nRp = mRegressionLayer->featureCount();
        mRegressionPoints = mat(nRp, 2, fill::zeros);
        mRegTimeStamp = vec(nRp, fill::zeros);
        QgsFeatureIterator iterator = mRegressionLayer->getFeatures();
        QgsFeature f;
        for (int i = 0; i < nRp && iterator.nextFeature(f); i++)
        {
            QgsPointXY centroPoint = f.geometry().centroid().asPoint();
            mRegressionPoints(i, 0) = centroPoint.x();
            mRegressionPoints(i, 1) = centroPoint.y();
            // 提取时间
            mRegTimeStamp(i) = f.attribute(mTimeVar.index).toDouble();
        }
    }
    else
    {
        mRegressionPoints = mDataPoints;
        mRegTimeStamp = mDataTimeStamp;
    }
    // 设置空间距离中的数据指针
    if (mSTWeight.distance()->type() == GwmDistance::CRSDistance || mSTWeight.distance()->type() == GwmDistance::MinkwoskiDistance)
    {
        GwmCRSDistance* d = static_cast<GwmCRSDistance*>(mSTWeight.distance());
        d->setDataPoints(&mDataPoints);
        d->setFocusPoints(&mRegressionPoints);
        mSTWeight.setDataTimeStamp(mDataTimeStamp);
        mSTWeight.setFocusTimeStamp(mRegTimeStamp);
    }
}

void GwmGTWRAlgorithm::initXY(mat &x, mat &y, const GwmVariable &depVar, const QList<GwmVariable> &indepVars)
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

mat GwmGTWRAlgorithm::regressionSerial(const mat &x, const vec &y)
{
    emit message("Regression ...");
    uword nRp = mRegressionPoints.n_rows, nVar = x.n_cols;
    mat betas(nVar, nRp, fill::zeros);
    for (uword i = 0; i < nRp & !checkCanceled(); i++)
    {
        vec w = mSTWeight.weightVector(i);
        mat xtw = trans(x.each_col() % w);
        mat xtwx = xtw * x;
        mat xtwy = xtw * y;
        try
        {
            mat xtwx_inv = inv_sympd(xtwx);
            betas.col(i) = xtwx_inv * xtwy;
            emit tick(i + 1, nRp);
        }
        catch (exception e)
        {
            emit error(e.what());
        }
    }
    return betas.t();
}

mat GwmGTWRAlgorithm::regressionOmp(const mat &x, const vec &y)
{
    emit message("Regression ...");
    int nRp = mRegressionPoints.n_rows, nVar = x.n_cols;
    mat betas(nVar, nRp, fill::zeros);
    int current = 0;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < nRp; i++)
    {
        if(!checkCanceled())
        {
            vec w = mSTWeight.weightVector(i);
            mat xtw = trans(x.each_col() % w);
            mat xtwx = xtw * x;
            mat xtwy = xtw * y;
            try
            {
                mat xtwx_inv = inv_sympd(xtwx);
                betas.col(i) = xtwx_inv * xtwy;
                emit tick(current + 1, nRp);
            }
            catch (exception e)
            {
                emit error(e.what());
            }
            emit tick(++current, nRp);
        }
    }
    return betas.t();
}

mat GwmGTWRAlgorithm::regressionHatmatrixSerial(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag)
{
    uword nDp = x.n_rows, nVar = x.n_cols;
    mat betas(nVar, nDp, fill::zeros);
    betasSE = mat(nVar, nDp, fill::zeros);
    shat = vec(2, fill::zeros);
    qDiag = vec(nDp, fill::zeros);
    for (uword i = 0; i < nDp & !checkCanceled(); i++)
    {
        vec w = mSTWeight.weightVector(i);
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
            shat(0) += si(0, i);
            shat(1) += det(si * si.t());
            vec p = - si.t();
            p(i) += 1.0;
            qDiag += p % p;
        }
        catch (std::exception e)
        {
            emit error(e.what());
        }
        emit tick(i + 1, nDp);
    }
    betasSE = betasSE.t();
    return betas.t();
}

mat GwmGTWRAlgorithm::regressionHatmatrixOmp(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag)
{
    emit message("Regression ...");
    int nDp = x.n_rows, nVar = x.n_cols;
    mat betas(nVar, nDp, fill::zeros);
    betasSE = mat(nVar, nDp, fill::zeros);
    mat shat_all(2, mOmpThreadNum, fill::zeros);
    mat qDiag_all(nDp, mOmpThreadNum, fill::zeros);
    int current = 0;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < nDp; i++)
    {
        if(!checkCanceled())
        {
            int thread = omp_get_thread_num();
            vec w = mSTWeight.weightVector(i);
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
                shat_all(0, thread) += si(0, i);
                shat_all(1, thread) += det(si * si.t());
                vec p = - si.t();
                p(i) += 1.0;
                qDiag_all.col(thread) += p % p;
            }
            catch (std::exception e)
            {
                emit error(e.what());
            }
            emit tick(++current, nDp);
        }
    }
    shat = sum(shat_all, 1);
    qDiag = sum(qDiag_all, 1);
    betasSE = betasSE.t();
    return betas.t();
}

double GwmGTWRAlgorithm::bandwidthSizeCriterionCVSerial(GwmBandwidthWeight *bandwidthWeight)
{
    uword nDp = mDataPoints.n_rows;
    vec shat(2, fill::zeros);
    double cv = 0.0;
    for (uword i = 0; i < nDp & !checkCanceled(); i++)
    {
        vec d = mSTWeight.distanceVector(i);
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
    if(!checkCanceled())
    {
        if (isfinite(cv))
        {
            QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
                    .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
                    .arg(bandwidthWeight->bandwidth())
                    .arg(cv);
            emit message(msg);
            return cv;
        }
        else return DBL_MAX;
    }
    else return DBL_MAX;
}

double GwmGTWRAlgorithm::bandwidthSizeCriterionCVOmp(GwmBandwidthWeight *bandwidthWeight)
{
    int nDp = mDataPoints.n_rows;
    vec shat(2, fill::zeros);
    vec cv_all(mOmpThreadNum, fill::zeros);
    bool flag = true;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < nDp; i++)
    {
        if (flag && !checkCanceled())
        {
            int thread = omp_get_thread_num();
            vec d = mSTWeight.distance()->distance(i);
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
                if (isfinite(res))
                    cv_all(thread) += res * res;
                else
                    flag = false;
            }
            catch (...)
            {
                flag = false;
            }
        }
    }
    if (flag && !checkCanceled())
    {
        double cv = sum(cv_all);
        QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
                .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
                .arg(bandwidthWeight->bandwidth())
                .arg(cv);
        emit message(msg);
        return cv;
    }
    else return DBL_MAX;
}

double GwmGTWRAlgorithm::bandwidthSizeCriterionAICSerial(GwmBandwidthWeight *bandwidthWeight)
{
    uword nDp = mDataPoints.n_rows, nVar = mIndepVars.size() + 1;
    mat betas(nVar, nDp, fill::zeros);
    vec shat(2, fill::zeros);
    for (uword i = 0; i < nDp & !checkCanceled(); i++)
    {
        vec d = mSTWeight.distance()->distance(i);
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
    double value = AICc(mX, mY, betas.t(), shat);
    if(!checkCanceled())
    {
        if (isfinite(value))
        {
            QString msg = QString(tr("%1 bandwidth: %2 (AIC Score: %3)"))
                    .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
                    .arg(bandwidthWeight->bandwidth())
                    .arg(value);
            emit message(msg);
            return value;
        }
        else return DBL_MAX;
    }
    else return DBL_MAX;
}

double GwmGTWRAlgorithm::bandwidthSizeCriterionAICOmp(GwmBandwidthWeight *bandwidthWeight)
{
    int nDp = mDataPoints.n_rows, nVar = mIndepVars.size() + 1;
    mat betas(nVar, nDp, fill::zeros);
    mat shat_all(2, mOmpThreadNum, fill::zeros);
    bool flag = true;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < nDp; i++)
    {
        if (flag && !checkCanceled())
        {
            int thread = omp_get_thread_num();
            vec d = mSTWeight.distance()->distance(i);
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
                shat_all(0, thread) += si(0, i);
                shat_all(1, thread) += det(si * si.t());
            }
            catch (std::exception e)
            {
                flag = false;
            }
        }
    }
    if (flag && !checkCanceled())
    {
        vec shat = sum(shat_all, 1);
        double value = AICc(mX, mY, betas.t(), shat);
        if (isfinite(value))
        {
            QString msg = QString(tr("%1 bandwidth: %2 (AIC Score: %3)"))
                    .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
                    .arg(bandwidthWeight->bandwidth())
                    .arg(value);
            emit message(msg);
            return value;
        }
        else return DBL_MAX;
    }
    else return DBL_MAX;
}

void GwmGTWRAlgorithm::createResultLayer(GwmGTWRAlgorithm::CreateResultLayerData data, QString name)
{
    QgsVectorLayer* srcLayer = mRegressionLayer ? mRegressionLayer : mDataLayer;
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    layerName += name;

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

    mResultLayer = QgsMemoryProviderUtils::createMemoryLayer(layerName, fields, srcLayer->wkbType(), srcLayer->crs());

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

void GwmGTWRAlgorithm::setParallelType(const ParallelType &type)
{
    if (type & parallelAbility())
    {
        mParallelType = type;
        switch (type) {
        case IParallelalbe::ParallelType::SerialOnly:
            mRegressionFunction = &GwmGTWRAlgorithm::regressionSerial;
            mRegressionHatmatrixFunction = &GwmGTWRAlgorithm::regressionHatmatrixSerial;
            setBandwidthSelectionCriterionType(mBandwidthSelectionCriterionType);
            break;
        case IParallelalbe::ParallelType::OpenMP:
            mRegressionFunction = &GwmGTWRAlgorithm::regressionOmp;
            mRegressionHatmatrixFunction = &GwmGTWRAlgorithm::regressionHatmatrixOmp;
            setBandwidthSelectionCriterionType(mBandwidthSelectionCriterionType);
            break;
        default:
            mRegressionFunction = &GwmGTWRAlgorithm::regressionSerial;
            mRegressionHatmatrixFunction = &GwmGTWRAlgorithm::regressionHatmatrixSerial;
            setBandwidthSelectionCriterionType(mBandwidthSelectionCriterionType);
            break;
        }
    }
}

#include "gwmbasicgwralgorithm.h"

GwmBasicGWRAlgorithm::GwmBasicGWRAlgorithm() : GwmGeographicalWeightedRegressionAlgorithm()
{

}


void GwmBasicGWRAlgorithm::run()
{
    // 点位初始化
    initPoints();

    // 优选模型
    if (isAutoselectIndepVars)
    {
        mIndepVarSelector.setIndepVars(mIndepVars);
        mIndepVarSelector.setThreshold(mIndepVarSelectionThreshold);
        QList<GwmVariable> selectedIndepVars = mIndepVarSelector.optimize(this);
        if (selectedIndepVars.size() > 0)
        {
            mIndepVars = selectedIndepVars;
        }
    }

    // 初始化
    initXY(mX, mY, mDepVar, mIndepVars);

    // 优选带宽
    if (isAutoselectBandwidth)
    {
        GwmBandwidthWeight* bandwidthWeight0 = static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight());
        mBandwidthSizeSelector.setBandwidth(bandwidthWeight0);
        mBandwidthSizeSelector.setLower(bandwidthWeight0->adaptive() ? 20 : 0.0);
        mBandwidthSizeSelector.setUpper(bandwidthWeight0->adaptive() ? mDataPoints.n_rows : DBL_MAX);
        switch (mBandwidthSelectionCriterionType)
        {
        case BandwidthSelectionCriterionType::CV:
            mBandwidthSizeCriterion = &GwmBasicGWRAlgorithm::bandwidthSizeCriterionCV;
            break;
        case BandwidthSelectionCriterionType::AIC:
            mBandwidthSizeCriterion = &GwmBasicGWRAlgorithm::bandwidthSizeCriterionAIC;
            break;
        default:
            mBandwidthSizeCriterion = &GwmBasicGWRAlgorithm::bandwidthSizeCriterionCV;
            break;
        }
        GwmBandwidthWeight* bandwidthWeight = mBandwidthSizeSelector.optimize(this);
        if (bandwidthWeight)
        {
            mSpatialWeight.setWeight(bandwidthWeight);
        }
    }

    // 解算模型
    if (hasHatMatrix)
    {
        uword nDp = mDataPoints.n_rows, nVar = mIndepVars.size() + 1;
        mat betasSE, S(isStoreS() ? nDp : 1, nDp, fill::zeros);
        vec shat, qDiag;
        mBetas = regression(mX, mY, betasSE, shat, qDiag, S);
        // 诊断
        mDiagnostic = calcDiagnostic(mX, mY, mBetas, shat);
        double trS = shat(0), trStS = shat(1);
        double sigmaHat = mDiagnostic.RSS / (nDp - 2 * trS + trStS);
        betasSE = sqrt(sigmaHat * betasSE);
        vec yhat = Fitted(mX, mBetas);
        vec res = mY - yhat;
        vec stu_res = res / sqrt(sigmaHat * qDiag);
        mat betasTV = mBetas / betasSE;
        vec dybar2 = (mY - mean(mY)) % (mY - mean(mY));
        vec dyhat2 = (mY - yhat) % (mY - yhat);
        vec localR2 = vec(nDp, fill::zeros);
        for (int i = 0; i < nDp; i++)
        {
            vec w = mSpatialWeight.spatialWeight(mDataPoints.row(i), mDataPoints);
            double tss = sum(dybar2 % w);
            double rss = sum(dyhat2 % w);
            localR2(i) = (tss - rss) / tss;
        }

        QList<QPair<QString, const mat> > resultLayerData = {
            qMakePair(QString("%1"), mBetas),
            qMakePair(QString("y"), mY),
            qMakePair(QString("yhat"), yhat),
            qMakePair(QString("residual"), res),
            qMakePair(QString("Stud_residual"), stu_res),
            qMakePair(QString("%1_SE"), betasSE),
            qMakePair(QString("%1_TV"), betasTV),
            qMakePair(QString("localR2"), localR2)
        };

        createResultLayer(resultLayerData);
    }
    else
    {
        mBetas = regression(mX, mY);
        QList<QPair<QString, const mat> > resultLayerData = {
            qMakePair(QString("%1"), mBetas)
        };
        createResultLayer(resultLayerData);
    }

    emit success();
}

double GwmBasicGWRAlgorithm::criterion(QList<GwmVariable> indepVars)
{
    mat x;
    vec y;
    initXY(x, y, mDepVar, indepVars);
    uword nDp = x.n_rows, nVar = x.n_cols;
    mat betas(nVar, nDp, fill::zeros);
    vec shat(2, fill::zeros);
    for (uword i = 0; i < nDp; i++)
    {
        vec w(nDp, fill::ones);
        mat xtw = trans(x.each_col() % w);
        mat xtwx = xtw * x;
        mat xtwy = xtw * y;
        try
        {
            mat xtwx_inv = inv(xtwx);
            betas.col(i) = xtwx_inv * xtwy;
            mat ci = xtwx_inv * xtw;
            mat si = x.row(i) * ci;
            shat(0) += si(0, i);
            shat(1) += det(si * si.t());
        }
        catch (std::exception e)
        {
            return DBL_MAX;
        }
    }
    return GwmGeographicalWeightedRegressionAlgorithm::AICc(x, y, betas.t(), shat);
}

mat GwmBasicGWRAlgorithm::regression(const mat &x, const vec &y)
{
    uword nRp = mRegressionPoints.n_rows, nVar = x.n_cols;
    const mat& points = hasRegressionLayer() ? mRegressionPoints : mDataPoints;
    mat betas(nVar, nRp, fill::zeros);
    for (uword i = 0; i < nRp; i++)
    {
        vec w = mSpatialWeight.spatialWeight(points.row(i), mDataPoints);
        mat xtw = trans(x.each_col() % w);
        mat xtwx = xtw * x;
        mat xtwy = xtw * y;
        try
        {
            mat xtwx_inv = inv(xtwx);
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

mat GwmBasicGWRAlgorithm::regression(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S)
{
    uword nDp = x.n_rows, nVar = x.n_cols;
    mat betas(nVar, nDp, fill::zeros);
    betasSE = mat(nVar, nDp, fill::zeros);
    shat = vec(2, fill::zeros);
    qDiag = vec(nDp, fill::zeros);
    for (uword i = 0; i < nDp; i++)
    {
        vec w = mSpatialWeight.spatialWeight(mDataPoints.row(i), mDataPoints);
        mat xtw = trans(x.each_col() % w);
        mat xtwx = xtw * x;
        mat xtwy = xtw * y;
        try
        {
            mat xtwx_inv = inv(xtwx);
            betas.col(i) = xtwx_inv * xtwy;
            mat ci = xtwx_inv * xtw;
            betasSE.col(i) = sum(ci % ci, 1);
            mat si = x.row(i) * ci;
            shat(0) += si(0, i);
            shat(1) += det(si * si.t());
            vec p = - si.t();
            p(i) += 1.0;
            qDiag += p % p;
            S.row(isStoreS() ? i : 0) = si;
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

GwmDiagnostic GwmBasicGWRAlgorithm::calcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat)
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

void GwmBasicGWRAlgorithm::createResultLayer(QList<QPair<QString, const mat> > data)
{
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

double GwmBasicGWRAlgorithm::bandwidthSizeCriterionAIC(GwmBandwidthWeight* bandwidthWeight)
{
    uword nDp = mDataPoints.n_rows, nVar = mIndepVars.size() + 1;
    mat betas(nDp, nVar, fill::zeros);
    vec shat(2, fill::zeros);
    for (uword i = 0; i < nDp; i++)
    {
        vec d = mSpatialWeight.distance()->distance(mDataPoints.row(i), mDataPoints);
        vec w = bandwidthWeight->weight(d);
        mat xtw = trans(mX.each_col() % w);
        mat xtwx = xtw * mX;
        mat xtwy = xtw * mY;
        try
        {
            mat xtwx_inv = inv(xtwx);
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
    return GwmGeographicalWeightedRegressionAlgorithm::AICc(mX, mY, betas.t(), shat);
}

double GwmBasicGWRAlgorithm::bandwidthSizeCriterionCV(GwmBandwidthWeight *bandwidthWeight)
{
    uword nDp = mDataPoints.n_rows, nVar = mIndepVars.size() + 1;
    vec shat(2, fill::zeros);
    double cv = 0.0;
    for (uword i = 0; i < nDp; i++)
    {
        vec d = mSpatialWeight.distance()->distance(mDataPoints.row(i), mDataPoints);
        vec w = bandwidthWeight->weight(d);
        w(i) = 0.0;
        mat xtw = trans(mX.each_col() % w);
        mat xtwx = xtw * mX;
        mat xtwy = xtw * mY;
        try
        {
            mat xtwx_inv = inv(xtwx);
            vec beta = xtwx_inv * xtwy;
            double res = mY(i) - det(mX.row(i) * beta);
            cv += res * res;
        }
        catch (std::exception e)
        {
            return DBL_MAX;
        }
    }
    return cv;
}

double GwmBasicGWRAlgorithm::indepVarSelectionThreshold() const
{
    return mIndepVarSelectionThreshold;
}

void GwmBasicGWRAlgorithm::setIndepVarSelectionThreshold(double indepVarSelectionThreshold)
{
    mIndepVarSelectionThreshold = indepVarSelectionThreshold;
}

QgsVectorLayer *GwmBasicGWRAlgorithm::regressionLayer() const
{
    return mRegressionLayer;
}

void GwmBasicGWRAlgorithm::setRegressionLayer(QgsVectorLayer *layer)
{
    mRegressionLayer = layer;
}

bool GwmBasicGWRAlgorithm::isValid()
{
    if (GwmGeographicalWeightedRegressionAlgorithm::isValid())
    {
        if (mRegressionLayer && hasHatMatrix)
            return false;

        if (mRegressionLayer && hasFTest)
            return false;
    }
    else return false;
}

bool GwmBasicGWRAlgorithm::autoselectIndepVars() const
{
    return isAutoselectIndepVars;
}

void GwmBasicGWRAlgorithm::setIsAutoselectIndepVars(bool value)
{
    isAutoselectIndepVars = value;
}

bool GwmBasicGWRAlgorithm::autoselectBandwidth() const
{
    return isAutoselectBandwidth;
}

void GwmBasicGWRAlgorithm::setIsAutoselectBandwidth(bool value)
{
    isAutoselectBandwidth = value;
}

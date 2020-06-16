#include "gwmbasicgwralgorithm.h"
#include <SpatialWeight/gwmcrsdistance.h>
#include <SpatialWeight/gwmminkwoskidistance.h>
#include <gsl/gsl_cdf.h>
#include <omp.h>


GwmDiagnostic GwmBasicGWRAlgorithm::CalcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat)
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


GwmEnumValueNameMapper<GwmBasicGWRAlgorithm::BandwidthSelectionCriterionType> GwmBasicGWRAlgorithm::BandwidthSelectionCriterionTypeNameMapper = {
    std::make_pair(GwmBasicGWRAlgorithm::BandwidthSelectionCriterionType::CV, "CV"),
    std::make_pair(GwmBasicGWRAlgorithm::BandwidthSelectionCriterionType::AIC, "AIC")
};


GwmBasicGWRAlgorithm::GwmBasicGWRAlgorithm() : GwmGeographicalWeightedRegressionAlgorithm()
{

}


void GwmBasicGWRAlgorithm::run()
{
    // 点位初始化
    emit message(QString(tr("Setting data points")) + (hasRegressionLayer() ? tr(" and regression points") : "") + ".");
    initPoints();

    // 优选模型
    if (mIsAutoselectIndepVars)
    {
        emit message(QString(tr("Automatically selecting independent variables ...")));
        mIndepVarSelectModelsTotalNum = (mIndepVars.size() + 1) * (mIndepVars.size()) / 2;
        mIndepVarSelector.setIndepVars(mIndepVars);
        mIndepVarSelector.setThreshold(mIndepVarSelectionThreshold);
        emit tick(mIndepVarSelectModelsCurrent, mIndepVarSelectModelsTotalNum);
        QList<GwmVariable> selectedIndepVars = mIndepVarSelector.optimize(this);
        if (selectedIndepVars.size() > 0)
        {
            mIndepVars = selectedIndepVars;
            // 绘图
            QVariant data = QVariant::fromValue(mIndepVarSelector.indepVarsCriterion());
            emit plot(data, &GwmIndependentVariableSelector::PlotModelOrder);
            emit plot(data, &GwmIndependentVariableSelector::PlotModelAICcs);
        }
    }

    // 初始化
    emit message(QString(tr("Setting X and Y.")));
    initXY(mX, mY, mDepVar, mIndepVars);

    // 优选带宽
    if (mIsAutoselectBandwidth)
    {
        emit message(QString(tr("Automatically selecting bandwidth ...")));
        emit tick(0, 0);
        GwmBandwidthWeight* bandwidthWeight0 = static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight());
        mBandwidthSizeSelector.setBandwidth(bandwidthWeight0);
        double lower = bandwidthWeight0->adaptive() ? 20 : 0.0;
        double upper = bandwidthWeight0->adaptive() ? mDataPoints.n_rows : findMaxDistance();
        mBandwidthSizeSelector.setLower(lower);
        mBandwidthSizeSelector.setUpper(upper);
        GwmBandwidthWeight* bandwidthWeight = mBandwidthSizeSelector.optimize(this);
        if (bandwidthWeight)
        {
            mSpatialWeight.setWeight(bandwidthWeight);
            // 绘图
            QVariant data = QVariant::fromValue(mBandwidthSizeSelector.bandwidthCriterion());
            emit plot(data, &GwmBandwidthSizeSelector::PlotBandwidthResult);
        }
    }

    // 解算模型
    if (mHasHatMatrix)
    {
        uword nDp = mDataPoints.n_rows;
        mBetas = regression(mX, mY);
        // 诊断
        mDiagnostic = CalcDiagnostic(mX, mY, mBetas, mShat);
        double trS = mShat(0), trStS = mShat(1);
        double sigmaHat = mDiagnostic.RSS / (nDp - 2 * trS + trStS);
        mBetasSE = sqrt(sigmaHat * mBetasSE);
        vec yhat = Fitted(mX, mBetas);
        vec res = mY - yhat;
        vec stu_res = res / sqrt(sigmaHat * mQDiag);
        mat betasTV = mBetas / mBetasSE;
        vec dybar2 = (mY - mean(mY)) % (mY - mean(mY));
        vec dyhat2 = (mY - yhat) % (mY - yhat);
        vec localR2 = vec(nDp, fill::zeros);
        for (uword i = 0; i < nDp; i++)
        {
            vec w = mSpatialWeight.spatialWeight(distanceParam1(i), mDataPoints);
            double tss = sum(dybar2 % w);
            double rss = sum(dyhat2 % w);
            localR2(i) = (tss - rss) / tss;
        }

        CreateResultLayerData resultLayerData = {
            qMakePair(QString("%1"), mBetas),
            qMakePair(QString("y"), mY),
            qMakePair(QString("yhat"), yhat),
            qMakePair(QString("residual"), res),
            qMakePair(QString("Stud_residual"), stu_res),
            qMakePair(QString("%1_SE"), mBetasSE),
            qMakePair(QString("%1_TV"), betasTV),
            qMakePair(QString("localR2"), localR2)
        };
        createResultLayer(resultLayerData);

        if (mHasHatMatrix && mHasFTest)
        {
            double trQtQ = DBL_MAX;
            if (isStoreS())
            {
                mat EmS = eye(nDp, nDp) - mS;
                mat Q = trans(EmS) * EmS;
                trQtQ = sum(diagvec(trans(Q) * Q));
            }
            else
            {
                trQtQ = calcTrQtQ();
            }
            FTestParameters fTestParams;
            fTestParams.nDp = mDataLayer->featureCount();
            fTestParams.nVar = mIndepVars.size() + 1;
            fTestParams.trS = mShat(0);
            fTestParams.trStS = mShat(1);
            fTestParams.trQ = sum(mQDiag);
            fTestParams.trQtQ = trQtQ;
            fTestParams.gwrRSS = sum(res % res);
            fTest(fTestParams);
        }
    }
    else
    {
        mBetas = regression(mX, mY);
        CreateResultLayerData resultLayerData = {
            qMakePair(QString("%1"), mBetas)
        };
        createResultLayer(resultLayerData);
    }

    emit success();
}

void GwmBasicGWRAlgorithm::initCuda(IGWmodelCUDA* cuda, const mat& x, const vec& y)
{
    arma::uword nDp = mDataPoints.n_rows, nVar = x.n_cols, nRp = hasRegressionLayer() ? mRegressionPoints.n_rows : mDataPoints.n_rows;
    for (arma::uword r = 0; r < nDp; r++)
    {
        for (arma::uword c = 0; c < nVar; c++)
        {
            cuda->SetX(c, r, x(r, c));
        }
        cuda->SetY(r, y(r));
        cuda->SetDp(r, mDataPoints(r, 0), mDataPoints(r, 1));
    }
    if (hasRegressionLayer())
    {
        for (uword r = 0; r < nRp; r++)
        {
            cuda->SetRp(r, mRegressionPoints(r, 0), mRegressionPoints(r, 1));
        }
    }
    bool hasDmat = mSpatialWeight.distance()->type() == GwmDistance::DMatDistance;
    if (hasDmat)
    {
        for (arma::uword r = 0; r < nRp; r++)
        {
            vec dist = mSpatialWeight.distance()->distance(distanceParam1(r), mDataPoints);
            for (arma::uword d = 0; d < nDp; d++)
            {
                cuda->SetDmat(d, r, dist(d));
            }
        }
    }
}

mat GwmBasicGWRAlgorithm::regression(const mat &x, const vec &y)
{
    if (hasHatMatrix())
        return (this->*mRegressionHatmatrixFunction)(x, y, mBetasSE, mShat, mQDiag, mS);
    else return (this->*mRegressionFunction)(x, y);
}

double GwmBasicGWRAlgorithm::indepVarsSelectCriterionSerial(const QList<GwmVariable>& indepVars)
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
            mat xtwx_inv = inv_sympd(xtwx);
            betas.col(i) = xtwx_inv * xtwy;
            mat ci = xtwx_inv * xtw;
            mat si = x.row(i) * ci;
            shat(0) += si(0, i);
            shat(1) += det(si * si.t());
        }
        catch (...)
        {
            return DBL_MAX;
        }
    }
    emit tick(++mIndepVarSelectModelsCurrent, mIndepVarSelectModelsTotalNum);
    double value = GwmGeographicalWeightedRegressionAlgorithm::AICc(x, y, betas.t(), shat);
    QStringList names;
    for (const GwmVariable& v : indepVars)
        names << v.name;
    QString msg = QString("Model: %1 ~ %2 (AICc Value: %3)").arg(mDepVar.name).arg(names.join(" + ")).arg(value);
    emit message(msg);
    return value;
}

double GwmBasicGWRAlgorithm::indepVarsSelectCriterionOmp(const QList<GwmVariable> &indepVars)
{
    mat x;
    vec y;
    initXY(x, y, mDepVar, indepVars);
    int nDp = x.n_rows, nVar = x.n_cols;
    mat betas(nVar, nDp, fill::zeros);
    mat shat(2, mOmpThreadNum, fill::zeros);
    int flag = true;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < nDp; i++)
    {
        if (flag)
        {
            int thread = omp_get_thread_num();
            vec w(nDp, fill::ones);
            mat xtw = trans(x.each_col() % w);
            mat xtwx = xtw * x;
            mat xtwy = xtw * y;
            try
            {
                mat xtwx_inv = inv_sympd(xtwx);
                betas.col(i) = xtwx_inv * xtwy;
                mat ci = xtwx_inv * xtw;
                mat si = x.row(i) * ci;
                shat(0, thread) += si(0, i);
                shat(1, thread) += det(si * si.t());
            }
            catch (...)
            {
                flag = false;
            }
        }
    }
    emit tick(++mIndepVarSelectModelsCurrent, mIndepVarSelectModelsTotalNum);
    if (flag)
    {
        double value = GwmGeographicalWeightedRegressionAlgorithm::AICc(x, y, betas.t(), sum(shat, 1));
        QStringList names;
        for (const GwmVariable& v : indepVars)
            names << v.name;
        QString msg = QString("Model: %1 ~ %2 (AICc Value: %3)").arg(mDepVar.name).arg(names.join(" + ")).arg(value);
        emit message(msg);
        return value;
    }
    else return DBL_MAX;
}

double GwmBasicGWRAlgorithm::indepVarsSelectCriterionCuda(const QList<GwmVariable> &indepVars)
{
    mat x;
    vec y;
    initXY(x, y, mDepVar, indepVars);
    int nDp = mDataPoints.n_rows, nVar = indepVars.size() + 1;
    int nRp = hasRegressionLayer() ? mRegressionPoints.n_rows : mDataPoints.n_rows;
    bool hasDp = mSpatialWeight.distance()->type() == GwmDistance::DMatDistance;
    IGWmodelCUDA* cuda = GWCUDA_Create(nDp, nVar, hasRegressionLayer(), nRp, hasDp);
    initCuda(cuda, x, y);
    // 计算参数
    double p = 2.0, theta = 0.0;
    double longlat = false;
    if (mSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance)
    {
        GwmMinkwoskiDistance* d = static_cast<GwmMinkwoskiDistance*>(mSpatialWeight.distance());
        p = d->poly();
        theta = d->theta();
    }
    else if (mSpatialWeight.distance()->type() == GwmDistance::CRSDistance)
    {
        GwmCRSDistance* d = static_cast<GwmCRSDistance*>(mSpatialWeight.distance());
        longlat = d->geographic();
    }
    GwmBandwidthWeight* bw = static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight());
    bool gwrStatus = cuda->Regression(true, p, theta, longlat, DBL_MAX, bw->kernel(), bw->adaptive(), mGroupSize, mGpuId);
    mat betas(nVar, nDp, fill::zeros);
    vec shat(2, fill::zeros);
    double value = DBL_MAX;
    emit tick(++mIndepVarSelectModelsCurrent, mIndepVarSelectModelsTotalNum);
    if (gwrStatus)
    {
        for (int r = 0; r < nDp; r++)
        {
            for (int c = 0; c < nVar; c++)
            {
                betas(c, r) = cuda->GetBetas(r, c);
            }
        }
        shat(0) = cuda->GetShat1();
        shat(1) = cuda->GetShat2();
        value = AICc(x, y, betas.t(), shat);
        QStringList names;
        for (const GwmVariable& v : indepVars)
            names << v.name;
        QString msg = QString("Model: %1 ~ %2 (AICc Value: %3)").arg(mDepVar.name).arg(names.join(" + ")).arg(value);
        emit message(msg);
    }
    GWCUDA_Del(cuda);
    return value;
}

mat GwmBasicGWRAlgorithm::regressionSerial(const mat &x, const vec &y)
{
    emit message("Regression ...");
    uword nRp = mRegressionPoints.n_rows, nVar = x.n_cols;
    const mat& points = hasRegressionLayer() ? mRegressionPoints : mDataPoints;
    mat betas(nVar, nRp, fill::zeros);
    for (uword i = 0; i < nRp; i++)
    {
        vec w = mSpatialWeight.spatialWeight(distanceParam1(i), mDataPoints);
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

mat GwmBasicGWRAlgorithm::regressionOmp(const mat &x, const vec &y)
{
    emit message("Regression ...");
    int nRp = mRegressionPoints.n_rows, nVar = x.n_cols;
    const mat& points = hasRegressionLayer() ? mRegressionPoints : mDataPoints;
    mat betas(nVar, nRp, fill::zeros);
    int current = 0;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < nRp; i++)
    {
        vec w = mSpatialWeight.spatialWeight(distanceParam1(i), mDataPoints);
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
    return betas.t();
}

mat GwmBasicGWRAlgorithm::regressionCuda(const mat &x, const vec &y)
{
    int nDp = mDataPoints.n_rows, nVar = x.n_cols, nRp = hasRegressionLayer() ? mRegressionPoints.n_rows : mDataPoints.n_rows;
    bool hasDmat = mSpatialWeight.distance()->type() == GwmDistance::DMatDistance;
    IGWmodelCUDA* cuda = GWCUDA_Create(nDp ,nVar, hasRegressionLayer(), nRp, hasDmat);
    initCuda(cuda, x, y);
    double p = 2.0, theta = 0.0;
    double longlat = false;
    if (mSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance)
    {
        GwmMinkwoskiDistance* d = static_cast<GwmMinkwoskiDistance*>(mSpatialWeight.distance());
        p = d->poly();
        theta = d->theta();
    }
    else if (mSpatialWeight.distance()->type() == GwmDistance::CRSDistance)
    {
        GwmCRSDistance* d = static_cast<GwmCRSDistance*>(mSpatialWeight.distance());
        longlat = d->geographic();
    }
    int groupSize = mGroupSize;
    int gpuID = mGpuId;
    GwmBandwidthWeight* bw = static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight());
    bool adaptive = bw->adaptive();
    bool gwrStatus = cuda->Regression(false, p, theta, longlat, bw->bandwidth(), bw->kernel(), adaptive, groupSize, gpuID);
    mat betas(nVar, nRp, fill::zeros);
    if (gwrStatus)
    {
        for (int r = 0; r < nRp; r++)
        {
            for (int c = 0; c < nVar; c++)
            {
                betas(c, r) = cuda->GetBetas(r, c);
            }
        }
    }
    GWCUDA_Del(cuda);
    return betas.t();
}

mat GwmBasicGWRAlgorithm::regressionHatmatrixSerial(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S)
{
    emit message("Regression ...");
    uword nDp = x.n_rows, nVar = x.n_cols;
    mat betas(nVar, nDp, fill::zeros);
    betasSE = mat(nVar, nDp, fill::zeros);
    shat = vec(2, fill::zeros);
    qDiag = vec(nDp, fill::zeros);
    S = mat(isStoreS() ? nDp : 1, nDp, fill::zeros);
    for (uword i = 0; i < nDp; i++)
    {
        vec w = mSpatialWeight.spatialWeight(distanceParam1(i), mDataPoints);
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

mat GwmBasicGWRAlgorithm::regressionHatmatrixOmp(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S)
{
    emit message("Regression ...");
    int nDp = x.n_rows, nVar = x.n_cols;
    mat betas(nVar, nDp, fill::zeros);
    betasSE = mat(nVar, nDp, fill::zeros);
    S = mat(isStoreS() ? nDp : 1, nDp, fill::zeros);
    mat shat_all(2, mOmpThreadNum, fill::zeros);
    mat qDiag_all(nDp, mOmpThreadNum, fill::zeros);
    int current = 0;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < nDp; i++)
    {
        int thread = omp_get_thread_num();
        vec w = mSpatialWeight.spatialWeight(distanceParam1(i), mDataPoints);
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
            S.row(isStoreS() ? i : 0) = si;
        }
        catch (std::exception e)
        {
            emit error(e.what());
        }
        emit tick(++current, nDp);
    }
    shat = sum(shat_all, 1);
    qDiag = sum(qDiag_all, 1);
    betasSE = betasSE.t();
    return betas.t();
}

mat GwmBasicGWRAlgorithm::regressionHatmatrixCuda(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S)
{
    int nDp = mDataPoints.n_rows, nVar = x.n_cols, nRp = hasRegressionLayer() ? mRegressionPoints.n_rows : mDataPoints.n_rows;
    bool hasDmat = mSpatialWeight.distance()->type() == GwmDistance::DMatDistance;
    IGWmodelCUDA* cuda = GWCUDA_Create(nDp ,nVar, hasRegressionLayer(), nRp, hasDmat);
    initCuda(cuda, x, y);
    double p = 2.0, theta = 0.0;
    bool longlat = false;
    if (mSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance)
    {
        GwmMinkwoskiDistance* d = static_cast<GwmMinkwoskiDistance*>(mSpatialWeight.distance());
        p = d->poly();
        theta = d->theta();
    }
    else if (mSpatialWeight.distance()->type() == GwmDistance::CRSDistance)
    {
        GwmCRSDistance* d = static_cast<GwmCRSDistance*>(mSpatialWeight.distance());
        longlat = d->geographic();
    }
    GwmBandwidthWeight* bw = static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight());
    bool adaptive = bw->adaptive();
    bool gwrStatus = cuda->Regression(true, p, theta, longlat, bw->bandwidth(), bw->kernel(), adaptive, mGroupSize, mGpuId);
    mat betas(nVar, nDp, fill::zeros);
    betasSE = mat(nVar, nDp, fill::zeros);
    shat = vec(2, fill::zeros);
    qDiag = vec(nDp, fill::zeros);
    if (gwrStatus)
    {
        for (int r = 0; r < nDp; r++)
        {
            for (int c = 0; c < nVar; c++)
            {
                betas(c, r) = cuda->GetBetas(r, c);
                betasSE(c, r) = cuda->GetBetasSE(r, c);
            }
            qDiag(r) = cuda->GetQdiag(r);
        }
        shat(0) = cuda->GetShat1();
        shat(1) = cuda->GetShat2();
        if (isStoreS())
        {
            S = mat(nDp, nDp, fill::zeros);
            for (int r = 0; r < nDp; r++)
            {
                for (int c = 0; c < nVar; c++)
                {
                    S(r, c) = cuda->GetS(r, c);
                }
            }
        }
    }
    betasSE = betasSE.t();
    GWCUDA_Del(cuda);
    return betas.t();
}

void GwmBasicGWRAlgorithm::createResultLayer(CreateResultLayerData data)
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

double GwmBasicGWRAlgorithm::bandwidthSizeCriterionAICSerial(GwmBandwidthWeight* bandwidthWeight)
{
    uword nDp = mDataPoints.n_rows, nVar = mIndepVars.size() + 1;
    mat betas(nVar, nDp, fill::zeros);
    vec shat(2, fill::zeros);
    for (uword i = 0; i < nDp; i++)
    {
        vec d = mSpatialWeight.distance()->distance(distanceParam1(i), mDataPoints);
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
    double value = GwmGeographicalWeightedRegressionAlgorithm::AICc(mX, mY, betas.t(), shat);
    QString msg = QString(tr("%1 bandwidth: %2 (AIC Score: %3)"))
            .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
            .arg(bandwidthWeight->bandwidth())
            .arg(value);
    emit message(msg);
    return value;
}

double GwmBasicGWRAlgorithm::bandwidthSizeCriterionAICOmp(GwmBandwidthWeight *bandwidthWeight)
{
    int nDp = mDataPoints.n_rows, nVar = mIndepVars.size() + 1;
    mat betas(nVar, nDp, fill::zeros);
    mat shat_all(2, mOmpThreadNum, fill::zeros);
    bool flag = true;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < nDp; i++)
    {
        if (flag)
        {
            int thread = omp_get_thread_num();
            vec d = mSpatialWeight.distance()->distance(distanceParam1(i), mDataPoints);
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
    if (flag)
    {
        vec shat = sum(shat_all, 1);
        double value = GwmGeographicalWeightedRegressionAlgorithm::AICc(mX, mY, betas.t(), shat);
        QString msg = QString(tr("%1 bandwidth: %2 (AIC Score: %3)"))
                .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
                .arg(bandwidthWeight->bandwidth())
                .arg(value);
        emit message(msg);
        return value;
    }
    else return DBL_MAX;
}

double GwmBasicGWRAlgorithm::bandwidthSizeCriterionAICCuda(GwmBandwidthWeight *bandwidthWeight)
{
    int nDp = mDataPoints.n_rows, nVar = mX.n_cols, nRp = hasRegressionLayer() ? mRegressionPoints.n_rows : mDataPoints.n_rows;
    bool hasDmat = mSpatialWeight.distance()->type() == GwmDistance::DMatDistance;
    IGWmodelCUDA* cuda = GWCUDA_Create(nDp ,nVar, hasRegressionLayer(), nRp, hasDmat);
    initCuda(cuda, mX, mY);
    double p = 2.0, theta = 0.0;
    double longlat = false;
    if (mSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance)
    {
        GwmMinkwoskiDistance* d = static_cast<GwmMinkwoskiDistance*>(mSpatialWeight.distance());
        p = d->poly();
        theta = d->theta();
    }
    else if (mSpatialWeight.distance()->type() == GwmDistance::CRSDistance)
    {
        GwmCRSDistance* d = static_cast<GwmCRSDistance*>(mSpatialWeight.distance());
        longlat = d->geographic();
    }
    GwmBandwidthWeight* bw = static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight());
    bool adaptive = bw->adaptive();
    bool gwrStatus = cuda->Regression(true, p, theta, longlat, bw->bandwidth(), bw->kernel(), adaptive, mGroupSize, mGpuId);
    mat betas(nVar, nDp, fill::zeros);
    vec shat(2, fill::zeros);
    double aic = DBL_MAX;
    if (gwrStatus)
    {
        for (int r = 0; r < nDp; r++)
        {
            for (int c = 0; c < nVar; c++)
            {
                betas(c, r) = cuda->GetBetas(r, c);
            }
        }
        shat(0) = cuda->GetShat1();
        shat(1) = cuda->GetShat2();
        aic = AICc(mX, mY, betas, shat);
        QString msg = QString(tr("%1 bandwidth: %2 (AIC Score: %3)"))
                .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
                .arg(bandwidthWeight->bandwidth())
                .arg(aic);
        emit message(msg);
    }
    GWCUDA_Del(cuda);
    return aic;
}

double GwmBasicGWRAlgorithm::bandwidthSizeCriterionCVSerial(GwmBandwidthWeight *bandwidthWeight)
{
    uword nDp = mDataPoints.n_rows;
    vec shat(2, fill::zeros);
    double cv = 0.0;
    for (uword i = 0; i < nDp; i++)
    {
        vec d = mSpatialWeight.distance()->distance(distanceParam1(i), mDataPoints);
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
        catch (std::exception e)
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

double GwmBasicGWRAlgorithm::bandwidthSizeCriterionCVOmp(GwmBandwidthWeight *bandwidthWeight)
{
    int nDp = mDataPoints.n_rows;
    vec shat(2, fill::zeros);
    vec cv_all(mOmpThreadNum, fill::zeros);
    bool flag = true;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < nDp; i++)
    {
        if (flag)
        {
            int thread = omp_get_thread_num();
            vec d = mSpatialWeight.distance()->distance(distanceParam1(i), mDataPoints);
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
                cv_all(thread) += res * res;
            }
            catch (std::exception e)
            {
                flag = false;
            }
        }
    }
    if (flag)
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

double GwmBasicGWRAlgorithm::bandwidthSizeCriterionCVCuda(GwmBandwidthWeight *bandwidthWeight)
{
    int nDp = mDataPoints.n_rows, nVar = mX.n_cols, nRp = hasRegressionLayer() ? mRegressionPoints.n_rows : mDataPoints.n_rows;
    bool hasDmat = mSpatialWeight.distance()->type() == GwmDistance::DMatDistance;
    IGWmodelCUDA* cuda = GWCUDA_Create(nDp ,nVar, hasRegressionLayer(), nRp, hasDmat);
    initCuda(cuda, mX, mY);
    double p = 2.0, theta = 0.0;
    double longlat = false;
    if (mSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance)
    {
        GwmMinkwoskiDistance* d = static_cast<GwmMinkwoskiDistance*>(mSpatialWeight.distance());
        p = d->poly();
        theta = d->theta();
    }
    else if (mSpatialWeight.distance()->type() == GwmDistance::CRSDistance)
    {
        GwmCRSDistance* d = static_cast<GwmCRSDistance*>(mSpatialWeight.distance());
        longlat = d->geographic();
    }
    bool adaptive = bandwidthWeight->adaptive();
    double cv = cuda->CV(p, theta, longlat, bandwidthWeight->bandwidth(), bandwidthWeight->kernel(), adaptive, mGroupSize, mGpuId);
    if (cv < DBL_MAX)
    {
        QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
                .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
                .arg(bandwidthWeight->bandwidth())
                .arg(cv);
        emit message(msg);
    }
    GWCUDA_Del(cuda);
    return cv;
}

void GwmBasicGWRAlgorithm::fTest(GwmBasicGWRAlgorithm::FTestParameters params)
{
    emit message("F Test");
    GwmFTestResult f1, f2, f4;
    double v1 = params.trS, v2 = params.trStS;
    int nDp = params.nDp, nVar = params.nVar;
    emit tick(0, nVar + 3);
//    double edf = 1.0 * nDp - 2 * v1 + v2;
    double RSSg = params.gwrRSS;
    vec betao = solve(mX, mY);
    vec residual = mY - mX * betao;
    double RSSo = sum(residual % residual);
    double DFo = nDp - nVar;
    double delta1 = 1.0 * nDp - 2 * v1 + v2;
    double sigma2delta1 = RSSg / delta1;
//    double sigma2 = RSSg / nDp;
    double trQ = params.trQ, trQtQ = params.trQtQ;
    double lDelta1 = trQ;
    double lDelta2 = trQtQ;

    // F1 Test
    f1.s = (RSSg/lDelta1)/(RSSo/DFo);
    f1.df1 = lDelta1 * lDelta1 / lDelta2;
    f1.df2 = DFo;
    f1.p = gsl_cdf_fdist_P(f1.s, f1.df1, f1.df2);
    emit tick(1, nVar + 3);

    // F2 Test
    f2.s = ((RSSo-RSSg)/(DFo-lDelta1))/(RSSo/DFo);
    f2.df1 = (DFo-lDelta1) * (DFo-lDelta1) / (DFo - 2 * lDelta1 + lDelta2);
    f2.df2 = DFo;
    f2.p = gsl_cdf_fdist_Q(f2.s, f2.df1, f2.df2);
    emit tick(2, nVar + 3);

    // F3 Test
    vec vk2(nVar, fill::zeros);
    for (int i = 0; i < nVar; i++)
    {
        vec betasi = mBetas.col(i);
        vec betasJndp = vec(nDp, fill::ones) * (sum(betasi) * 1.0 / nDp);
        vk2(i) = (1.0 / nDp) * det(trans(betasi - betasJndp) * betasi);
    }
    QList<GwmFTestResult> f3;
    for (int i = 0; i < nVar; i++)
    {
        vec diagB = calcDiagB(i);
        double g1 = diagB(0);
        double g2 = diagB(1);
        double numdf = g1 * g1 / g2;
        GwmFTestResult f3i;
        f3i.s = (vk2(i) / g1) / sigma2delta1;
        f3i.df1 = numdf;
        f3i.df2 = f1.df1;
        f3i.p = gsl_cdf_fdist_Q(f3i.s, numdf, f1.df1);
        f3.append(f3i);
        emit tick(3 + i, nVar + 3);
    }

    // F4 Test
    f4.s = RSSg / RSSo;
    f4.df1 = delta1;
    f4.df2 = DFo;
    f4.p = gsl_cdf_fdist_P(f4.s, f4.df1, f4.df2);
    emit tick(nVar + 3, nVar + 3);
    // 保存结果
    mF1TestResult = f1;
    mF2TestResult = f2;
    mF3TestResult = f3;
    mF4TestResult = f4;
}

double GwmBasicGWRAlgorithm::findMaxDistance()
{
    int nDp = mDataPoints.n_rows;
    double maxD = 0.0;
    for (int i = 0; i < nDp; i++)
    {
        double d = max(mSpatialWeight.distance()->distance(distanceParam1(i), mDataPoints));
        maxD = d > maxD ? d : maxD;
    }
    return maxD;
}

int GwmBasicGWRAlgorithm::groupSize() const
{
    return mGroupSize;
}

void GwmBasicGWRAlgorithm::setGroupSize(int groupSize)
{
    mGroupSize = groupSize;
}

double GwmBasicGWRAlgorithm::calcTrQtQSerial()
{
    double trQtQ = 0.0;
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    emit message(tr("Calculating the trace of matrix Q..."));
    emit tick(0, nDp);
    mat wspan(1, nVar, fill::ones);
    for (arma::uword i = 0; i < nDp; i++)
    {
        vec wi = mSpatialWeight.spatialWeight(distanceParam1(i), mDataPoints);
        mat xtwi = trans(mX % (wi * wspan));
        try {
            mat xtwxR = inv_sympd(xtwi * mX);
            mat ci = xtwxR * xtwi;
            mat si = mX.row(i) * inv(xtwi * mX) * xtwi;
            vec pi = -trans(si);
            pi(i) += 1.0;
            double qi = sum(pi % pi);
            trQtQ += qi * qi;
            for (arma::uword j = i + 1; j < nDp; j++)
            {
                vec wj = mSpatialWeight.spatialWeight(mDataPoints.row(j), mDataPoints);
                mat xtwj = trans(mX % (wj * wspan));
                try {
                    mat sj = mX.row(j) * inv_sympd(xtwj * mX) * xtwj;
                    vec pj = -trans(sj);
                    pj(j) += 1.0;
                    double qj = sum(pi % pj);
                    trQtQ += qj * qj * 2.0;
                } catch (...) {
                    emit error("Matrix seems to be singular.");
                    emit tick(nDp, nDp);
                    return DBL_MAX;
                }
            }
        } catch (...) {
            emit error("Matrix seems to be singular.");
            emit tick(nDp, nDp);
            return DBL_MAX;
        }
        emit tick(i + 1, nDp);
    }
    return trQtQ;
}

double GwmBasicGWRAlgorithm::calcTrQtQOmp()
{
    vec trQtQ(mOmpThreadNum, fill::zeros);
    int nDp = mX.n_rows, nVar = mX.n_cols;
    emit message(tr("Calculating the trace of matrix Q..."));
    emit tick(0, nDp);
    mat wspan(1, nVar, fill::ones);
    bool flag = true;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < nDp; i++)
    {
        if (flag)
        {
            int thread = omp_get_thread_num();
            vec wi = mSpatialWeight.spatialWeight(distanceParam1(i), mDataPoints);
            mat xtwi = trans(mX % (wi * wspan));
            try {
                mat xtwxR = inv_sympd(xtwi * mX);
                mat ci = xtwxR * xtwi;
                mat si = mX.row(i) * inv(xtwi * mX) * xtwi;
                vec pi = -trans(si);
                pi(i) += 1.0;
                double qi = sum(pi % pi);
                trQtQ(thread) += qi * qi;
                for (arma::uword j = i + 1; j < nDp && flag; j++)
                {
                    vec wj = mSpatialWeight.spatialWeight(mDataPoints.row(j), mDataPoints);
                    mat xtwj = trans(mX % (wj * wspan));
                    try {
                        mat sj = mX.row(j) * inv_sympd(xtwj * mX) * xtwj;
                        vec pj = -trans(sj);
                        pj(j) += 1.0;
                        double qj = sum(pi % pj);
                        trQtQ(thread) += qj * qj * 2.0;
                    } catch (...) {
                        emit error("Matrix seems to be singular.");
                        emit tick(nDp, nDp);
                        flag = false;
                    }
                }
            } catch (...) {
                emit error("Matrix seems to be singular.");
                emit tick(nDp, nDp);
                flag = false;
            }
        }
        emit tick(i + 1, nDp);
    }
    return flag ? sum(trQtQ) : DBL_MAX;
}

vec GwmBasicGWRAlgorithm::calcDiagBSerial(int i)
{
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    vec diagB(nDp, fill::zeros), c(nDp, fill::zeros);
    mat ek = eye(nVar, nVar);
    mat wspan(1, nVar, fill::ones);
    for (arma::uword j = 0; j < nDp; j++)
    {
        vec w = mSpatialWeight.spatialWeight(mDataPoints.row(j), mDataPoints);
        mat xtw = trans(mX % (w * wspan));
        try {
            mat C = trans(xtw) * inv_sympd(xtw * mX);
            c += C.col(i);
        } catch (...) {
            emit error("Matrix seems to be singular.");
            return { DBL_MAX, DBL_MAX };
        }
    }
    for (arma::uword k = 0; k < nDp; k++)
    {
        vec w = mSpatialWeight.spatialWeight(mDataPoints.row(k), mDataPoints);
        mat xtw = trans(mX % (w * wspan));
        try {
            mat C = trans(xtw) * inv_sympd(xtw * mX);
            vec b = C.col(i);
            diagB += (b % b - (1.0 / nDp) * (b % c));
        } catch (...) {
            emit error("Matrix seems to be singular.");
            return { DBL_MAX, DBL_MAX };
        }
    }
    diagB = 1.0 / nDp * diagB;
    return { sum(diagB), sum(diagB % diagB) };
}

vec GwmBasicGWRAlgorithm::calcDiagBOmp(int i)
{
    int nDp = mX.n_rows, nVar = mX.n_cols;
    mat diagB_all(nDp, mOmpThreadNum, fill::zeros), c_all(nDp, mOmpThreadNum, fill::zeros);
    mat ek = eye(nVar, nVar);
    mat wspan(1, nVar, fill::ones);
    bool flag = true;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int j = 0; j < nDp; j++)
    {
        if (flag)
        {
            int thread = omp_get_thread_num();
            vec w = mSpatialWeight.spatialWeight(mDataPoints.row(j), mDataPoints);
            mat xtw = trans(mX % (w * wspan));
            try {
                mat C = trans(xtw) * inv_sympd(xtw * mX);
                c_all.col(thread) += C.col(i);
            } catch (...) {
                emit error("Matrix seems to be singular.");
                flag = false;
            }
        }
    }
    vec c = sum(c_all, 1);
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int k = 0; k < nDp; k++)
    {
        if (flag)
        {
            int thread = omp_get_thread_num();
            vec w = mSpatialWeight.spatialWeight(mDataPoints.row(k), mDataPoints);
            mat xtw = trans(mX % (w * wspan));
            try {
                mat C = trans(xtw) * inv_sympd(xtw * mX);
                vec b = C.col(i);
                diagB_all.col(thread) += (b % b - (1.0 / nDp) * (b % c));
            } catch (...) {
                emit error("Matrix seems to be singular.");
                flag = false;
            }
        }
    }
    vec diagB = 1.0 / nDp * sum(diagB_all, 1);
    return { sum(diagB), sum(diagB % diagB) };
}

bool GwmBasicGWRAlgorithm::isValid()
{
    if (GwmGeographicalWeightedRegressionAlgorithm::isValid())
    {
        if (mRegressionLayer && mHasHatMatrix)
            return false;

        if (mRegressionLayer && mHasFTest)
            return false;

        return true;
    }
    else return false;
}

void GwmBasicGWRAlgorithm::setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType)
{
    mBandwidthSelectionCriterionType = bandwidthSelectionCriterionType;
    QMap<QPair<BandwidthSelectionCriterionType, IParallelalbe::ParallelType>, BandwidthSelectCriterionFunction> mapper = {
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, IParallelalbe::ParallelType::SerialOnly), &GwmBasicGWRAlgorithm::bandwidthSizeCriterionCVSerial),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, IParallelalbe::ParallelType::OpenMP), &GwmBasicGWRAlgorithm::bandwidthSizeCriterionCVOmp),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, IParallelalbe::ParallelType::CUDA), &GwmBasicGWRAlgorithm::bandwidthSizeCriterionCVCuda),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, IParallelalbe::ParallelType::SerialOnly), &GwmBasicGWRAlgorithm::bandwidthSizeCriterionAICSerial),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, IParallelalbe::ParallelType::OpenMP), &GwmBasicGWRAlgorithm::bandwidthSizeCriterionAICOmp),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, IParallelalbe::ParallelType::CUDA), &GwmBasicGWRAlgorithm::bandwidthSizeCriterionAICCuda)
    };
    mBandwidthSelectCriterionFunction = mapper[qMakePair(bandwidthSelectionCriterionType, mParallelType)];
}

void GwmBasicGWRAlgorithm::setParallelType(const IParallelalbe::ParallelType &type)
{
    if (type & parallelAbility())
    {
        mParallelType = type;
        switch (type) {
        case IParallelalbe::ParallelType::SerialOnly:
            mRegressionFunction = &GwmBasicGWRAlgorithm::regressionSerial;
            mRegressionHatmatrixFunction = &GwmBasicGWRAlgorithm::regressionHatmatrixSerial;
            mIndepVarsSelectCriterionFunction = &GwmBasicGWRAlgorithm::indepVarsSelectCriterionSerial;
            mCalcTrQtQFunction = &GwmBasicGWRAlgorithm::calcTrQtQSerial;
            mCalcDiagBFunction = &GwmBasicGWRAlgorithm::calcDiagBSerial;
            setBandwidthSelectionCriterionType(mBandwidthSelectionCriterionType);
            break;
        case IParallelalbe::ParallelType::OpenMP:
            mRegressionFunction = &GwmBasicGWRAlgorithm::regressionOmp;
            mRegressionHatmatrixFunction = &GwmBasicGWRAlgorithm::regressionHatmatrixOmp;
            mIndepVarsSelectCriterionFunction = &GwmBasicGWRAlgorithm::indepVarsSelectCriterionOmp;
            mCalcTrQtQFunction = &GwmBasicGWRAlgorithm::calcTrQtQOmp;
            mCalcDiagBFunction = &GwmBasicGWRAlgorithm::calcDiagBOmp;
            setBandwidthSelectionCriterionType(mBandwidthSelectionCriterionType);
            break;
        case IParallelalbe::ParallelType::CUDA:
            mRegressionFunction = &GwmBasicGWRAlgorithm::regressionCuda;
            mRegressionHatmatrixFunction = &GwmBasicGWRAlgorithm::regressionHatmatrixCuda;
            mIndepVarsSelectCriterionFunction = &GwmBasicGWRAlgorithm::indepVarsSelectCriterionCuda;
            mCalcTrQtQFunction = &GwmBasicGWRAlgorithm::calcTrQtQSerial;
            mCalcDiagBFunction = &GwmBasicGWRAlgorithm::calcDiagBSerial;
            setBandwidthSelectionCriterionType(mBandwidthSelectionCriterionType);
            break;
        default:
            mRegressionFunction = &GwmBasicGWRAlgorithm::regressionSerial;
            mRegressionHatmatrixFunction = &GwmBasicGWRAlgorithm::regressionHatmatrixSerial;
            mIndepVarsSelectCriterionFunction = &GwmBasicGWRAlgorithm::indepVarsSelectCriterionSerial;
            mCalcTrQtQFunction = &GwmBasicGWRAlgorithm::calcTrQtQSerial;
            mCalcDiagBFunction = &GwmBasicGWRAlgorithm::calcDiagBSerial;
            setBandwidthSelectionCriterionType(mBandwidthSelectionCriterionType);
            break;
        }
    }
}

#include "gwmbasicgwralgorithm.h"
#include <gsl/gsl_cdf.h>
#include <omp.h>


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
        switch (mParallelType) {
        case IParallelalbe::SerialOnly:
            mIndepVarsSelectCriterionFunction = &GwmBasicGWRAlgorithm::indepVarsSelectCriterionSerial;
        case IParallelalbe::OpenMP:
        default:
            mIndepVarsSelectCriterionFunction = &GwmBasicGWRAlgorithm::indepVarsSelectCriterionSerial;
            break;
        }
        mIndepVarSelector.setIndepVars(mIndepVars);
        mIndepVarSelector.setThreshold(mIndepVarSelectionThreshold);
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
        GwmBandwidthWeight* bandwidthWeight0 = static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight());
        mBandwidthSizeSelector.setBandwidth(bandwidthWeight0);
        double lower = bandwidthWeight0->adaptive() ? 20 : 0.0;
        double upper = bandwidthWeight0->adaptive() ? mDataPoints.n_rows : findMaxDistance();
        mBandwidthSizeSelector.setLower(lower);
        mBandwidthSizeSelector.setUpper(upper);
        switch (mBandwidthSelectionCriterionType)
        {
        case BandwidthSelectionCriterionType::CV:
            mBandwidthSelectCriterionFunction = &GwmBasicGWRAlgorithm::bandwidthSizeCriterionCV;
            break;
        case BandwidthSelectionCriterionType::AIC:
            mBandwidthSelectCriterionFunction = &GwmBasicGWRAlgorithm::bandwidthSizeCriterionAIC;
            break;
        default:
            mBandwidthSelectCriterionFunction = &GwmBasicGWRAlgorithm::bandwidthSizeCriterionCV;
            break;
        }
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
        for (uword i = 0; i < nDp; i++)
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

        if (mHasHatMatrix && mHasFTest)
        {
            double trQtQ = DBL_MAX;
            if (isStoreS())
            {
                mat EmS = eye(nDp, nDp) - S;
                mat Q = trans(EmS) * EmS;
                trQtQ = sum(diagvec(trans(Q) * Q));
            }
            else
            {
                trQtQ = (this->*calcTrQtQ())();
            }
            FTestParameters fTestParams;
            fTestParams.nDp = mDataLayer->featureCount();
            fTestParams.nVar = mIndepVars.size() + 1;
            fTestParams.trS = shat(0);
            fTestParams.trStS = shat(1);
            fTestParams.trQ = sum(qDiag);
            fTestParams.trQtQ = trQtQ;
            fTestParams.gwrRSS = sum(res % res);
            fTest(fTestParams);
        }
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
    uword nDp = x.n_rows, nVar = x.n_cols;
    mat betas(nVar, nDp, fill::zeros);
    mat shat(2, mOmpThreadNum, fill::zeros);
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (uword i = 0; i < nDp; i++)
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
            return DBL_MAX;
        }
    }
    double value = GwmGeographicalWeightedRegressionAlgorithm::AICc(x, y, betas.t(), sum(shat, 1));
    QStringList names;
    for (const GwmVariable& v : indepVars)
        names << v.name;
    QString msg = QString("Model: %1 ~ %2 (AICc Value: %3)").arg(mDepVar.name).arg(names.join(" + ")).arg(value);
    emit message(msg);
    return value;
}

mat GwmBasicGWRAlgorithm::regression(const mat &x, const vec &y)
{
    emit message("Regression ...");
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

mat GwmBasicGWRAlgorithm::regression(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S)
{
    emit message("Regression ...");
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
    mat betas(nVar, nDp, fill::zeros);
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

double GwmBasicGWRAlgorithm::bandwidthSizeCriterionCV(GwmBandwidthWeight *bandwidthWeight)
{
    uword nDp = mDataPoints.n_rows;
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
    f3.reserve(nVar);
    for (int i = 0; i < nVar; i++)
    {
        double g1 = 0.0, g2 = 0.0, numdf = 0.0;
        if (isStoreS())
        {
            mat B(nDp, nDp, fill::zeros);
            mat ek = eye(nVar, nVar);
            mat wspan(1, nVar, fill::ones);
            for (int j = 0; j < nDp; j++)
            {
                vec w = mSpatialWeight.spatialWeight(mDataPoints.row(j), mDataPoints);
                mat xtw = trans(mX % (w * wspan));
                B.row(j) = ek.row(i) * inv_sympd(xtw * mX) * xtw;
            }
            mat Bj = 1.0 / nDp * (B.t() * (eye(nDp, nDp) - (1.0 / nDp) * mat(nDp, nDp, fill::ones)) * B);
            vec b = diagvec(Bj);
            g1 = sum(b);
            g2 = sum(b % b);
            qDebug("Var %d tr(B)   = %lf", i, g1);
            qDebug("Var %d tr(B.B) = %lf", i, g2);
            numdf = g1 * g1 / g2;
        }
        else
        {
            vec diagB = (this->*calcDiagB())(i);
            g1 = diagB(0);
            g2 = diagB(1);
            numdf = g1 * g1 / g2;
        }
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
        double d = max(mSpatialWeight.distance()->distance(mDataPoints.row(i), mDataPoints));
        maxD = d > maxD ? d : maxD;
    }
    return maxD;
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
        vec wi = mSpatialWeight.spatialWeight(mDataPoints.row(i), mDataPoints);
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

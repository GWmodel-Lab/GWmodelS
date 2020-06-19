#include "gwmrobustgwrtaskthread.h"
#include "GWmodel/GWmodel.h"

#include <gsl/gsl_cdf.h>

GwmRobustGWRAlgorithm::GwmRobustGWRAlgorithm(): GwmBasicGWRAlgorithm()
{

}

GwmDiagnostic GwmRobustGWRAlgorithm::CalcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat)
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

void GwmRobustGWRAlgorithm::run()
{
    //点位初始化
    emit message(QString(tr("Setting data points")) + (hasRegressionLayer() ? tr(" and regression points") : "") + ".");
    initPoints();
    // 设置矩阵
    initXY(mX, mY, mDepVar, mIndepVars);

    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    mWeightMask = vec(nDp, fill::ones);

    if (filtered)
    {
        robustGWRCaliFirst();
    }
    else
    {
        robustGWRCaliSecond();
    }

    //诊断+结果图层
    if(mHasHatMatrix)
    {
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
            vec w = mSpatialWeight.spatialWeight(i);
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
                trQtQ = (this->*mCalcTrQtQFunction)();
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
        CreateResultLayerData resultLayerData = {
            qMakePair(QString("%1"), mBetas)
        };
        createResultLayer(resultLayerData);
    }

    emit success();
}

mat GwmRobustGWRAlgorithm::regression(const mat &x, const vec &y)
{
    if(!mHasHatMatrix)
    {
        emit message("Regression ...");
        uword nRp = mRegressionPoints.n_rows, nVar = x.n_cols;
        const mat& points = hasRegressionLayer() ? mRegressionPoints : mDataPoints;
        mat betas(nVar, nRp, fill::zeros);
        for (uword i = 0; i < nRp; i++)
        {
            vec w = mSpatialWeight.spatialWeight(i) % mWeightMask;
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
    }else{
        return regressionHatmatrixSerial(x, y, mBetasSE, mShat, mQDiag, mS);
    }
}

vec GwmRobustGWRAlgorithm::filtWeight(vec x)
{
    double iter = 0;
    double diffmse = 1;
    //计算residual
    mYHat = fitted(mX, mBetas);
    mResidual = mY - mYHat;
    //计算mse
    mMse = sum((mResidual % mResidual))/ mResidual.size();
    mWVect.ones(x.size(),1);
    //数组赋值
    for(int i=0;i<x.size();i++)
    {
        if(x[i]<=2){
            mWVect[i]=1;
        }else if(x[i]>2 && x[i]<3){
            mWVect[i]=pow((1-(pow(x[i]-2,2))),2);
        }else{
            mWVect[i]=0;
        }
    }
    return mWVect;
}

void GwmRobustGWRAlgorithm::setFiltered(bool value)
{
    if(value == true){
        this->filtered=true;
    }else{
        this->filtered=false;
    }
}

void GwmRobustGWRAlgorithm::setHasFTest(bool hasFTest)
{
    mHasFTest = hasFTest;
}

void GwmRobustGWRAlgorithm::setHasHatMatrix(bool hasHatMatrix)
{
    mHasHatMatrix = hasHatMatrix;
}

bool GwmRobustGWRAlgorithm::hasFTest() const
{
    return mHasFTest;
}

bool GwmRobustGWRAlgorithm::hasHatMatrix() const
{
    return mHasHatMatrix;
}

void GwmRobustGWRAlgorithm::createResultLayer(CreateResultLayerData data)
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

double GwmRobustGWRAlgorithm::calcTrQtQSerial()
{
    double trQtQ = 0.0;
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    emit message(tr("Calculating the trace of matrix Q..."));
    emit tick(0, nDp);
    mat wspan(1, nVar, fill::ones);
    for (arma::uword i = 0; i < nDp; i++)
    {
        vec wi = mSpatialWeight.spatialWeight(i);
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
                vec wj = mSpatialWeight.spatialWeight(j);
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

vec GwmRobustGWRAlgorithm::calcDiagBSerial(int i)
{
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    vec diagB(nDp, fill::zeros), c(nDp, fill::zeros);
    mat ek = eye(nVar, nVar);
    mat wspan(1, nVar, fill::ones);
    for (arma::uword j = 0; j < nDp; j++)
    {
        vec w = mSpatialWeight.spatialWeight(j);
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
        vec w = mSpatialWeight.spatialWeight(k);
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

void GwmRobustGWRAlgorithm::fTest(GwmRobustGWRAlgorithm::FTestParameters params)
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
        vec diagB = (this->*mCalcDiagBFunction)(i);
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

void GwmRobustGWRAlgorithm::robustGWRCaliFirst()
{
    mBetas = regression(mX, mY);
    //  ------------- 计算W.vect
    mYHat = fitted(mX, mBetas);
    mResidual = mY - mYHat;
    // 诊断信息
    mDiagnostic = CalcDiagnostic(mX, mY, mBetas, mShat);
    double trS = mShat(0), trStS = mShat(1);
    int nDp = mDataPoints.n_rows;
    double sigmaHat = mDiagnostic.RSS / (nDp * 1.0 - 2 * trS + trStS);
    mStudentizedResidual = mResidual / sqrt(sigmaHat * mQDiag);

    vec WVect(nDp,fill::zeros);

    //生成W.vect
    for(int i=0;i<mStudentizedResidual.size();i++){
        if(fabs(mStudentizedResidual[i])>3){
            WVect(i)=0;
        }else{
            WVect(i)=1;
        }
    }
    mWeightMask = WVect;
    mBetas = regression(mX, mY);
}

void GwmRobustGWRAlgorithm::robustGWRCaliSecond()
{
    int nDp = mX.n_rows;
    double iter = 0;
    double diffmse = 1;
    double delta = 1.0e-5;
    double maxiter = 20;
    mBetas = regression(mX,mY);
    //计算residual
    mYHat = fitted(mX, mBetas);
    mResidual = mY - mYHat;
    //计算mse
    mMse = sum((mResidual % mResidual))/ mResidual.size();
    //计算WVect
    mWeightMask = filtWeight(abs(mResidual/sqrt(mMse)));
    while(diffmse>delta && iter<maxiter){
        double oldmse = mMse;
        mBetas = regression(mX, mY);
        //计算residual
        mYHat = fitted(mX, mBetas);
        mResidual = mY - mYHat;
        mMse = sum((mResidual % mResidual))/ mResidual.size();
        mWeightMask = filtWeight(abs(mResidual/sqrt(mMse)));
        diffmse = abs(oldmse-mMse)/mMse;
        iter = iter +1;
    }
}

mat GwmRobustGWRAlgorithm::regressionHatmatrixSerial(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S)
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
        vec w = mSpatialWeight.spatialWeight(i) % mWeightMask;
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

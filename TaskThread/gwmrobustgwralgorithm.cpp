#include "gwmrobustgwralgorithm.h"

#include <gsl/gsl_cdf.h>
#include <omp.h>

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

    emit message("Regression ...");
    mBetas = regression(mX,mY);

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
            vec w = mSpatialWeight.weightVector(i);
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
            GwmBasicGWRAlgorithm::fTest(fTestParams);
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
    if(mHasHatMatrix)
    {
        if (mFiltered)
        {
            return robustGWRCaliFirst(x,y,mBetasSE,mShat,mQDiag,mS);
        }
        else
        {
            return robustGWRCaliSecond(x,y,mBetasSE,mShat,mQDiag,mS);
        }
    }else{
        return regressionSerial(x, y);
    }
}

vec GwmRobustGWRAlgorithm::filtWeight(vec residual, double mse)
{
    //计算residual
    vec r = abs(residual / sqrt(mse));
    vec wvect(r.size(), fill::ones);
    //数组赋值
    for(int i=0;i<r.size();i++)
    {
        if(r[i]<=2)
        {
            wvect[i]=1;
        }
        else if(r[i]>2 && r[i]<3)
        {
            double f = r[i]-2;
            wvect[i] = (1.0-(f * f)) * (1.0-(f * f));
        }
        else
        {
            wvect[i]=0;
        }
    }
    return wvect;
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

void GwmRobustGWRAlgorithm::setParallelType(const IParallelalbe::ParallelType &type)
{
    GwmBasicGWRAlgorithm::setParallelType(type);
    if (type & parallelAbility())
    {
        mParallelType = type;
        switch (type) {
        case IParallelalbe::ParallelType::SerialOnly:
            mRegressionHatmatrixFunction = &GwmRobustGWRAlgorithm::regressionHatmatrixSerial;
            break;
        case IParallelalbe::ParallelType::OpenMP:
            mRegressionHatmatrixFunction = &GwmRobustGWRAlgorithm::regressionHatmatrixOmp;
            break;
#ifdef ENABLE_CUDA
        case IParallelalbe::ParallelType::CUDA:
            mRegressionHatmatrixFunction = &GwmRobustGWRAlgorithm::regressionHatmatrixCuda;
            break;
#endif
        default:
            mRegressionHatmatrixFunction = &GwmRobustGWRAlgorithm::regressionHatmatrixSerial;
            break;
        }
    }
}

mat GwmRobustGWRAlgorithm::robustGWRCaliFirst(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S)
{
    mat betas = (this->*mRegressionHatmatrixFunction)(x,y,betasSE,shat,qDiag,S);
    //  ------------- 计算W.vect
    //vec yhat = fitted(x, betas);
    vec yhat = sum(betas % x,1);
    vec residual = y - yhat;
    // 诊断信息
    GwmDiagnostic diagnostic;
    diagnostic = CalcDiagnostic(x, y, betas, shat);
    double trS = shat(0), trStS = shat(1);
    double nDp = mDataPoints.n_rows;
    double sigmaHat = diagnostic.RSS / (nDp * 1.0 - 2 * trS + trStS);
    vec studentizedResidual = residual / sqrt(sigmaHat * qDiag);

    vec WVect(nDp,fill::zeros);

    //生成W.vect
    for(int i=0;i<studentizedResidual.size();i++){
        if(fabs(studentizedResidual[i])>3){
            WVect(i)=0;
        }else{
            WVect(i)=1;
        }
    }
    mWeightMask = WVect;
    betas = (this->*mRegressionHatmatrixFunction)(x,y,betasSE,shat,qDiag,S);
    //mShat赋值  必须
    mShat = shat;
    return betas;
}

mat GwmRobustGWRAlgorithm::robustGWRCaliSecond(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S)
{
    int nDp = x.n_rows;
    double iter = 0;
    double diffmse = 1;
    double delta = 1.0e-5;
    double maxiter = 20;
    mat betas = (this->*mRegressionHatmatrixFunction)(x,y,betasSE,shat,qDiag,S);
    //计算residual
    //vec yHat = fitted(x, betas);
    vec yHat = sum(betas % x,1);
    vec residual = y - yHat;
    //计算mse
    double mse = sum((residual % residual))/ residual.size();
    //计算WVect
    mWeightMask = filtWeight(residual, mse);
    while(diffmse>delta && iter<maxiter){
        double oldmse = mse;
        betas = (this->*mRegressionHatmatrixFunction)(x,y,betasSE,shat,qDiag,S);
        //计算residual
        //yHat = fitted(x, betas);
        yHat = sum(betas % x,1);
        residual = y - yHat;
        mse = sum((residual % residual))/ residual.size();
        mWeightMask = filtWeight(residual, mse);
        diffmse = abs(oldmse-mse)/mse;
        iter = iter +1;
    }
    //mShat赋值  必须
    mShat = shat;
    return betas;
}

mat GwmRobustGWRAlgorithm::regressionHatmatrixSerial(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S)
{
    uword nDp = x.n_rows, nVar = x.n_cols;
    mat betas(nVar, nDp, fill::zeros);
    betasSE = mat(nVar, nDp, fill::zeros);
    shat = vec(2, fill::zeros);
    qDiag = vec(nDp, fill::zeros);
    S = mat(isStoreS() ? nDp : 1, nDp, fill::zeros);
    for (uword i = 0; i < nDp; i++)
    {
        vec w = mSpatialWeight.weightVector(i) % mWeightMask;
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

mat GwmRobustGWRAlgorithm::regressionHatmatrixOmp(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S)
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
        vec w = mSpatialWeight.weightVector(i)  % mWeightMask;
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

#ifdef ENABLE_CUDA
mat GwmRobustGWRAlgorithm::regressionHatmatrixCuda(const mat &x, const vec &y, mat &betasSE, vec &shat, vec &qDiag, mat &S)
{
    int nDp = mDataPoints.n_rows, nVar = x.n_cols, nRp = hasRegressionLayer() ? mRegressionPoints.n_rows : mDataPoints.n_rows;
    bool hasDmat = mSpatialWeight.distance()->type() == GwmDistance::DMatDistance;
    IGWmodelCUDA* cuda = GWCUDA_Create(nDp ,nVar, hasRegressionLayer(), nRp, hasDmat);
    initCuda(cuda, x, y);
    double p = 2.0, theta = 0.0;
    bool longlat = false;
    if (mSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance)
    {
        GwmMinkwoskiDistance* d = mSpatialWeight.distance<GwmMinkwoskiDistance>();
        p = d->poly();
        theta = d->theta();
    }
    else if (mSpatialWeight.distance()->type() == GwmDistance::CRSDistance)
    {
        GwmCRSDistance* d = mSpatialWeight.distance<GwmCRSDistance>();
        longlat = d->geographic();
    }
    GwmBandwidthWeight* bw = mSpatialWeight.weight<GwmBandwidthWeight>();
    bool adaptive = bw->adaptive();
    for(int i=0;i<nDp;i++)
    {
        cuda->SetWeightMask(i,mWeightMask(i));
    }
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
#endif

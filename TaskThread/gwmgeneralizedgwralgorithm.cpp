#include "gwmgeneralizedgwralgorithm.h"

//#include "GWmodel/GWmodel.h"
//#include "gwmggwrbandwidthselectionthread.h"
#include "GWmodel/gwmgeneralizedlinearmodel.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <omp.h>
#include <exception>

using namespace std;

GwmEnumValueNameMapper<GwmGeneralizedGWRAlgorithm::Family> GwmGeneralizedGWRAlgorithm::FamilyValueNameMapper = {
    std::make_pair(GwmGeneralizedGWRAlgorithm::Family::Poisson, "Poisson"),
    std::make_pair(GwmGeneralizedGWRAlgorithm::Family::Binomial, "Binomial")
};

QMap<QString, double> GwmGeneralizedGWRAlgorithm::TolUnitDict = {
    make_pair(QString("e -3"), 0.001),
    make_pair(QString("e -5"), 0.00001),
    make_pair(QString("e -7"), 0.0000001),
    make_pair(QString("e -10"), 0.0000000001)
};


GwmGeneralizedGWRAlgorithm::GwmGeneralizedGWRAlgorithm() : GwmGeographicalWeightedRegressionAlgorithm()
{

}

void GwmGeneralizedGWRAlgorithm::setCanceled(bool canceled)
{
    if (mGlm) mGlm->setCanceled(canceled);
    mBandwidthSizeSelector.setCanceled(canceled);
    mSpatialWeight.distance()->setCanceled(canceled);
    return GwmTaskThread::setCanceled(canceled);
}

void GwmGeneralizedGWRAlgorithm::run()
{
    if(!checkCanceled())
    {
        // 点位初始化
        emit message(QString(tr("Setting data points")) + (hasRegressionLayer() ? tr(" and regression points") : "") + ".");
        initPoints();
    }
    if(!checkCanceled())
    {
        // 初始化
        emit message(QString(tr("Setting X and Y.")));
        initXY(mX, mY, mDepVar, mIndepVars);
    }
    // 优选带宽
    if (mIsAutoselectBandwidth && !checkCanceled())
    {
        emit message(QString(tr("Automatically selecting bandwidth ...")));
        //emit tick(0, 0);
        if ((mSpatialWeight.distance()->type() == GwmDistance::CRSDistance || mSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance) && !checkCanceled())
        {
            GwmCRSDistance* d = static_cast<GwmCRSDistance*>(mSpatialWeight.distance());
            d->setDataPoints(&mDataPoints);
            d->setFocusPoints(&mDataPoints);
        }
        GwmBandwidthWeight* bandwidthWeight0 = mSpatialWeight.weight<GwmBandwidthWeight>();
        mBandwidthSizeSelector.setBandwidth(bandwidthWeight0);
        double lower = bandwidthWeight0->adaptive() ? 20 : 0.0;
        double upper = bandwidthWeight0->adaptive() ? mDataPoints.n_rows : mSpatialWeight.distance()->maxDistance();
        mBandwidthSizeSelector.setLower(lower);
        mBandwidthSizeSelector.setUpper(upper);
        GwmBandwidthWeight* bandwidthWeight = !checkCanceled() ? mBandwidthSizeSelector.optimize(this) : nullptr;
        if (bandwidthWeight && !checkCanceled())
        {
            mSpatialWeight.setWeight(bandwidthWeight);
            // 绘图
            QVariant data = QVariant::fromValue(mBandwidthSizeSelector.bandwidthCriterion());
            emit plot(data, &GwmBandwidthSizeSelector::PlotBandwidthResult);
        }
        if ((mSpatialWeight.distance()->type() == GwmDistance::CRSDistance || mSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance) && !checkCanceled())
        {
            GwmCRSDistance* d = static_cast<GwmCRSDistance*>(mSpatialWeight.distance());
            d->setDataPoints(&mDataPoints);
            d->setFocusPoints(&mRegressionPoints);
        }
    }

    int nVar = mX.n_cols;
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
    if(!checkCanceled()) mBetas = mat(nVar, nRp, fill::zeros);
    if (mHasHatMatrix && !checkCanceled())
    {
        mBetasSE = mat( nVar,nDp, fill::zeros);
        mShat = vec(2,fill::zeros);
    }

    emit message(tr("Calculating Distance Matrix..."));
    mWtMat1 = mat(nDp,nDp,fill::zeros);
    if(!checkCanceled()){
        mWtMat2 = mat(nRp,nDp,fill::zeros);
    }
    if(mRegressionLayer && !checkCanceled()){
        for(int i = 0; i < nRp & !checkCanceled(); i++){
            vec weight = mSpatialWeight.weightVector(i);
            mWtMat2.col(i) = weight;
            emit tick(i, nRp);
        }
        if ((mSpatialWeight.distance()->type() == GwmDistance::CRSDistance || mSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance) && !checkCanceled())
        {
            GwmCRSDistance* d = static_cast<GwmCRSDistance*>(mSpatialWeight.distance());
            d->setDataPoints(&mDataPoints);
            d->setFocusPoints(&mDataPoints);
        }
        for(int i = 0; i < nDp & !checkCanceled(); i++){
            vec weight = mSpatialWeight.weightVector(i);
            mWtMat1.col(i) = weight;
            emit tick(i, nDp);
        }
    }
    else{
        for(int i = 0; i < nRp & !checkCanceled(); i++){
            vec weight = mSpatialWeight.weightVector(i);
            mWtMat2.col(i) = weight;
            emit tick(i, nRp);
        }
        mWtMat1 = mWtMat2;
    }

    bool isAllCorrect = true;
    if(!checkCanceled())
    {
        CalGLMModel(mX,mY);
        mBetas = (this->*mGGWRRegressionFunction)(mX,mY);
    }
    if(checkCanceled())
    {
        return;
    }

    if(mHasHatMatrix && !checkCanceled()){
        if(mFamily == Family::Poisson){
            mat betasTV = mBetas / mBetasSE;
            mBetas = trans(mBetas);
            mBetasSE = trans(mBetasSE);
            betasTV = trans(betasTV);
            double trS = mShat(0);
            double trStS = mShat(1);

            mat yhat = exp(Fitted(mX,mBetas));
            mat res = mY - yhat;

            //计算诊断信息
            double AIC = mGwDev + 2 * trS;
            double AICc = AIC + 2*trS*(trS+1)/(nDp-trS-1);
            double R2 = 1 - mGwDev/(mGLMDiagnostic.NullDev);  // pseudo.R2 <- 1 - gw.dev/null.dev
            vec vDiags(4);
            vDiags(0) = AIC;
            vDiags(1) = AICc;
            vDiags(2) = mGwDev;
            vDiags(3) = R2;
            mDiagnostic = GwmGGWRDiagnostic(vDiags);

            mResultList.push_back(qMakePair(QString("%1"), mBetas));
            mResultList.push_back(qMakePair(QString("y"), mY));
            mResultList.push_back(qMakePair(QString("yhat"), yhat));
            mResultList.push_back(qMakePair(QString("residual"), res));
            mResultList.push_back(qMakePair(QString("%1_SE"), mBetasSE));
            mResultList.push_back(qMakePair(QString("%1_TV"), betasTV));
        }
        else{
            mat n = vec(mY.n_rows,fill::ones);
            mBetas = trans(mBetas);

            double trS = mShat(0);
            double trStS = mShat(1);

            vec yhat = Fitted(mX,mBetas);
            yhat = exp(yhat)/(1+exp(yhat));

            vec res = mY - yhat;
            vec Dev = log(1/( (mY-n+yhat) % (mY-n+yhat) ) );
            double gwDev = sum(Dev);
            vec residual2 = res % res;
            double rss = sum(residual2);
            for(int i = 0; i < nDp & !checkCanceled(); i++){
                mBetasSE.col(i) = sqrt(mBetasSE.col(i));
    //            mBetasTV.col(i) = mBetas.col(i) / mBetasSE.col(i);
            }
            mBetasSE = trans(mBetasSE);
            mat betasTV = mBetas / mBetasSE;

            double AIC = gwDev + 2 * trS;
            double AICc = AIC + 2*trS*(trS+1)/(nDp-trS-1);
            double R2 = 1 - gwDev/(mGLMDiagnostic.NullDev);  // pseudo.R2 <- 1 - gw.dev/null.dev
            vec vDiags(4);
            vDiags(0) = AIC;
            vDiags(1) = AICc;
            vDiags(2) = gwDev;
            vDiags(3) = R2;
            mDiagnostic = GwmGGWRDiagnostic(vDiags);

            mResultList.push_back(qMakePair(QString("%1"), mBetas));
            mResultList.push_back(qMakePair(QString("y"), mY));
            mResultList.push_back(qMakePair(QString("yhat"), yhat));
            mResultList.push_back(qMakePair(QString("residual"), res));
            mResultList.push_back(qMakePair(QString("%1_SE"), mBetasSE));
            mResultList.push_back(qMakePair(QString("%1_TV"), betasTV));
        }
    }
    else{
        mBetas = trans(mBetas);
        mResultList.push_back(qMakePair(QString("%1"), mBetas));
    }
    // Create Result Layer
    if(!checkCanceled())
    {
        if (isAllCorrect)
        {
            createResultLayer(mResultList,QStringLiteral("_GGWR"));
        }
        emit tick(100,100);
        emit success();
    }
}

void GwmGeneralizedGWRAlgorithm::CalGLMModel(const mat &x, const vec &y)
{
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
    int nVar = x.n_cols;
    emit message(tr("Calibrating GLM model..."));
    mGlm = new GwmGeneralizedLinearModel();
    mGlm->setX(x);
    mGlm->setY(y);
    mGlm->setFamily(mFamily);
    mGlm->fit();
    double nulldev = mGlm->nullDev();
    double dev = mGlm->dev();
    double pseudor2 = 1- dev/nulldev;
    double aic = dev + 2 * nVar;
    double aicc = aic + 2 * nVar * (nVar + 1)/(nDp - nVar - 1);
    vec vGLMDiags(5);
    vGLMDiags(0) = aic;
    vGLMDiags(1) = aicc;
    vGLMDiags(2) = nulldev;
    vGLMDiags(3) = dev;
    vGLMDiags(4) = pseudor2;
    mGLMDiagnostic = GwmGLMDiagnostic(vGLMDiags);
}

mat GwmGeneralizedGWRAlgorithm::regressionPoissonSerial(const mat &x, const vec &y)
{
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
    int nVar = x.n_cols;
    mat betas = mat(nVar, nRp, fill::zeros);

    mat mu = (this->*mCalWtFunction)(x,y,mWtMat1);
    mGwDev = 0.0;
    for(int i = 0; i < nDp & !checkCanceled(); i++){
        if(y[i] != 0){
            mGwDev = mGwDev +  2*(y[i]*(log(y[i]/mu[i])-1)+mu[i]);
        }
        else{
            mGwDev = mGwDev + 2* mu[i];
        }
    }
    emit message(tr("Calibrating GGWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;
    bool isStoreS = (nDp <= 8192);
    mat ci, s_ri, S(isStoreS ? nDp : 1, nDp, fill::zeros);
    if(mHasHatMatrix && !checkCanceled()){
        for(int i = 0; i < nDp & !checkCanceled(); i++){
            try{
                vec wi = mWtMat2.col(i);
                vec gwsi = gwRegHatmatrix(x, myAdj, wi % mWt2, i, ci, s_ri);
                betas.col(i) = gwsi;
                mat invwt2 = 1.0 / mWt2;
                S.row(isStoreS ? i : 0) = s_ri;
                mat temp = mat(ci.n_rows,ci.n_cols);
                for(int j = 0; j < ci.n_rows & !checkCanceled(); j++){
                    temp.row(j) = ci.row(j) % trans(invwt2);
                }
                mBetasSE.col(i) = diag(temp * trans(ci));

                mShat(0) += s_ri(0, i);
                mShat(1) += det(s_ri * trans(s_ri));

                mBetasSE.col(i) = sqrt(mBetasSE.col(i));
    //            mBetasTV.col(i) = mBetas.col(i) / mBetasSE.col(i);

                emit tick(i, nDp);
            }
            catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }

        }
    }
    else{
        for(int i = 0; i < nRp & !checkCanceled(); i++){
            try{
                vec wi = mWtMat2.col(i);
                vec gwsi = gwReg(x, myAdj, wi * mWt2, i);
                betas.col(i) = gwsi;
                emit tick(i,nRp);
            }
            catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
    }
    return betas;
}

mat GwmGeneralizedGWRAlgorithm::regressionPoissonOmp(const mat &x, const vec &y)
{
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
    int nVar = x.n_cols;
    mat betas = mat(nVar, nRp, fill::zeros);

    mat mu = (this->*mCalWtFunction)(x,y,mWtMat1);

    mGwDev = 0.0;
    for(int i = 0; i < nDp & !checkCanceled(); i++){
        if(y[i] != 0){
            mGwDev = mGwDev +  2*(y[i]*(log(y[i]/mu[i])-1)+mu[i]);
        }
        else{
            mGwDev = mGwDev + 2* mu[i];
        }
    }
    emit message(tr("Calibrating GGWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;
    bool isStoreS = (nDp <= 8192);
    mat S(isStoreS ? nDp : 1, nDp, fill::zeros);
    int current = 0;
    if(mHasHatMatrix && !checkCanceled()){
        mat shat = mat(2,mOmpThreadNum,fill::zeros);       
#pragma omp parallel for num_threads(mOmpThreadNum)
        for(int i = 0; i < nDp; i++){
            mat ci,s_ri;
            if(!checkCanceled())
            {
                try{
                    int thread = omp_get_thread_num();
                    vec wi = mWtMat2.col(i);
                    vec gwsi = gwRegHatmatrix(x, myAdj, wi % mWt2, i, ci, s_ri);
                    betas.col(i) = gwsi;
                    mat invwt2 = 1.0 / mWt2;
                    S.row(isStoreS ? i : 0) = s_ri;
                    mat temp = mat(ci.n_rows,ci.n_cols);
                    for(int j = 0; j < ci.n_rows & !checkCanceled(); j++){
                        temp.row(j) = ci.row(j) % trans(invwt2);
                    }
                    mBetasSE.col(i) = diag(temp * trans(ci));

                    shat(0,thread) += s_ri(0, i);
                    shat(1,thread) += det(s_ri * trans(s_ri));

                    mBetasSE.col(i) = sqrt(mBetasSE.col(i));
        //            mBetasTV.col(i) = mBetas.col(i) / mBetasSE.col(i);

                    emit tick(current++, nDp);
                }
                catch (exception e) {
                    isAllCorrect = false;
                    emit error(e.what());
                }
            }
        }
        mShat(0) = sum(trans(shat.row(0)));
        mShat(1) = sum(trans(shat.row(1)));
    }
    else{
#pragma omp parallel for num_threads(mOmpThreadNum)
        for(int i = 0; i < nRp; i++){
            if(!checkCanceled())
            {
                try{
                    vec wi = mWtMat2.col(i);
                    vec gwsi = gwReg(x, myAdj, wi * mWt2, i);
                    betas.col(i) = gwsi;
                    emit tick(current++, nRp);
                }
                catch (exception e) {
                    isAllCorrect = false;
                    emit error(e.what());
                }
            }
        }
    }
    return betas;
}

mat GwmGeneralizedGWRAlgorithm::regressionBinomialOmp(const mat &x, const vec &y)
{
    int nVar = x.n_cols;
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
//    mat S = mat(nDp,nDp);
//    mat n = vec(mY.n_rows,fill::ones);
    mat betas = mat(nVar, nRp, fill::zeros);

    vec mu = (this->*mCalWtFunction)(x,y,mWtMat1);
    emit message(tr("Calibrating GGWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;

    bool isStoreS = (nDp <= 8192);
    mat S(isStoreS ? nDp : 1, nDp, fill::zeros);
//    mat S = mat(uword(0), uword(0));
    int current = 0;
    if(mHasHatMatrix){
        mat shat = mat(mOmpThreadNum,2,fill::zeros);
#pragma omp parallel for num_threads(mOmpThreadNum)
        for(int i = 0; i < nDp; i++){
            mat ci,s_ri;
            if(!checkCanceled())
            {
                try {
                    int thread = omp_get_thread_num();
                    vec wi = mWtMat1.col(i);
                    vec gwsi = gwRegHatmatrix(x, myAdj, wi % mWt2, i, ci, s_ri);
                    betas.col(i) = gwsi;
                    mat invwt2 = 1.0 / mWt2;
                    S.row(isStoreS ? i : 0) = s_ri;
                    mat temp = mat(ci.n_rows,ci.n_cols);
                    for(int j = 0; j < ci.n_rows & !checkCanceled(); j++){
                        temp.row(j) = ci.row(j) % trans(invwt2);
                    }
                    mBetasSE.col(i) = diag(temp * trans(ci));
                    shat(thread,0) += s_ri(0, i);
                    shat(thread,1) += det(s_ri * trans(s_ri));
                    emit tick(current++, nDp);
                }
                catch (exception e) {
                    isAllCorrect = false;
                    emit error(e.what());
                }
            }
        }
        mShat(0) = sum(shat.col(0));
        mShat(1) = sum(shat.col(1));
    }
    else{
#pragma omp parallel for num_threads(mOmpThreadNum)
        for(int i = 0; i < nRp; i++){
            if(!checkCanceled())
            {
                try {
                    vec wi = mWtMat2.col(i);
                    vec gwsi = gwReg(x, myAdj, wi * mWt2, i);
                    mBetas.col(i) = gwsi;
                    emit tick(current++, nRp);
                }
                catch (exception e) {
                    isAllCorrect = false;
                    emit error(e.what());
                }
            }
        }
    }

    return betas;
}

mat GwmGeneralizedGWRAlgorithm::regressionBinomialSerial(const mat &x, const vec &y)
{
    int nVar = x.n_cols;
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
//    mat S = mat(nDp,nDp);
//    mat n = vec(mY.n_rows,fill::ones);
    mat betas = mat(nVar, nRp, fill::zeros);

    vec mu = (this->*mCalWtFunction)(x,y,mWtMat1);
    emit message(tr("Calibrating GGWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;

    bool isStoreS = (nDp <= 8192);
    mat ci;
    mat s_ri, S(isStoreS ? nDp : 1, nDp, fill::zeros);
//    mat S = mat(uword(0), uword(0));
    if(mHasHatMatrix && !checkCanceled()){
        for(int i = 0; i < nDp & !checkCanceled(); i++){
            try {
                vec wi = mWtMat1.col(i);
                vec gwsi = gwRegHatmatrix(x, myAdj, wi % mWt2, i, ci, s_ri);
                betas.col(i) = gwsi;
                mat invwt2 = 1.0 / mWt2;
                S.row(isStoreS ? i : 0) = s_ri;
                mat temp = mat(ci.n_rows,ci.n_cols);
                for(int j = 0; j < ci.n_rows & !checkCanceled(); j++){
                    temp.row(j) = ci.row(j) % trans(invwt2);
                }
                mBetasSE.col(i) = diag(temp * trans(ci));
                mShat(0) += s_ri(0, i);
                mShat(1) += det(s_ri * trans(s_ri));
                emit tick(i, nDp);
            }
            catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }

    }
    else{
        for(int i = 0; i < nRp & !checkCanceled(); i++){
            try {
                vec wi = mWtMat2.col(i);
                vec gwsi = gwReg(x, myAdj, wi * mWt2, i);
                mBetas.col(i) = gwsi;
                emit tick(i, nRp);
            }
            catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
    }
    return betas;
}

double GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionCVSerial(GwmBandwidthWeight *bandwidthWeight)
{
    int n = mDataPoints.n_rows;
    vec cv = vec(n);
    mat wt = mat(n,n);
    for (int i = 0; i < n & !checkCanceled(); i++)
    {
        vec d = mSpatialWeight.distance()->distance(i);
        vec w = bandwidthWeight->weight(d);
        w.row(i) = 0;
        wt.col(i) = w;
        if(mBandwidthSizeSelector.counter<10)
            emit tick(mBandwidthSizeSelector.counter*10 + i * 5 / n, 100);
    }
    if (!checkCanceled()) (this->*mCalWtFunction)(mX,mY,wt);
    for (int i = 0; i < n & !checkCanceled(); i++){
        mat wi = wt.col(i) % mWt2;
        vec gwsi = gwReg(mX, myAdj, wi, i);
        mat yhatnoi = mX.row(i) * gwsi;
        if(mFamily == GwmGeneralizedGWRAlgorithm::Family::Poisson){
            cv.row(i) = mY.row(i) - exp(yhatnoi);
        }
        else{
            cv.row(i) = mY.row(i) - exp(yhatnoi)/(1+exp(yhatnoi));
        }
        if(mBandwidthSizeSelector.counter<10)
            emit tick(mBandwidthSizeSelector.counter*10 + i * 5 / n + 5, 100);
    }
    vec cvsquare = trans(cv) * cv ;
    double res = sum(cvsquare);
//    this->mBwScore.insert(bw,res);
    if(!checkCanceled())
    {
        QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
                .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
                .arg(bandwidthWeight->bandwidth())
                .arg(res);
        emit message(msg);
        return res;
    }   
    else return DBL_MAX;

}

double GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionCVOmp(GwmBandwidthWeight *bandwidthWeight)
{
    int n = mDataPoints.n_rows;
    vec cv = vec(n);
    mat wt = mat(n,n);
    int current1 = 0, current2 = 0;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < n; i++)
    {
        if(!checkCanceled())
        {
            vec d = mSpatialWeight.distance()->distance(i);
            vec w = bandwidthWeight->weight(d);
            w.row(i) = 0;
            wt.col(i) = w;
            if(mBandwidthSizeSelector.counter<10)
                emit tick(mBandwidthSizeSelector.counter*10 + current1 * 5 / n, 100);
            current1++;
        }
    }
    if (!checkCanceled()) (this->*mCalWtFunction)(mX,mY,wt);
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < n; i++){
        if(!checkCanceled())
        {
            mat wi = wt.col(i) % mWt2;
            vec gwsi = gwReg(mX, myAdj, wi, i);
            mat yhatnoi = mX.row(i) * gwsi;
            if(mFamily == GwmGeneralizedGWRAlgorithm::Family::Poisson){
                cv.row(i) = mY.row(i) - exp(yhatnoi);
            }
            else{
                cv.row(i) = mY.row(i) - exp(yhatnoi)/(1+exp(yhatnoi));
            }
            if(mBandwidthSizeSelector.counter<10)
                emit tick(mBandwidthSizeSelector.counter*10 + current2 * 5 / n + 5, 100);
            current2++;
        }
    }
    vec cvsquare = trans(cv) * cv ;
    double res = sum(cvsquare);
//    this->mBwScore.insert(bw,res);
    if(!checkCanceled())
    {
        QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
                .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
                .arg(bandwidthWeight->bandwidth())
                .arg(res);
        emit message(msg);
        return res;
    }
    else return DBL_MAX;
}

double GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionAICSerial(GwmBandwidthWeight *bandwidthWeight)
{
    int n = mDataPoints.n_rows;
    vec cv = vec(n);
    mat S = mat(n,n);
    mat wt = mat(n,n);    
    for (int i = 0; i < n & !checkCanceled(); i++)
    {
        vec d = mSpatialWeight.distance()->distance(i);
        vec w = bandwidthWeight->weight(d);
        wt.col(i) = w;
        if(mBandwidthSizeSelector.counter<10)
            emit tick(mBandwidthSizeSelector.counter*10 + i * 5 / n, 100);
    }
    if(!checkCanceled()) (this->*mCalWtFunction)(mX,mY,wt);
    vec trS = vec(1,fill::zeros);
    for (int i = 0; i < n & !checkCanceled(); i++){
        vec wi = wt.col(i) % mWt2;
        mat Ci = CiMat(mX,wi);
        S.row(i) = mX.row(i) * Ci;
        trS(0) += S(i,i);
        if(mBandwidthSizeSelector.counter<10)
            emit tick(mBandwidthSizeSelector.counter*10 + i * 5 / n + 5, 100);
    }
    double AICc;
    if(!checkCanceled())
    {
        if(S.is_finite()){
            double trs = double(trS(0));
            AICc = -2*mLLik + 2*trs + 2*trs*(trs+1)/(n-trs-1);
        }
        else{
            AICc = qInf();
        }
    }

    if(!checkCanceled())
    {
        QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
                .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
                .arg(bandwidthWeight->bandwidth())
                .arg(AICc);
        emit message(msg);
        return AICc;
    }
    else return DBL_MAX;
}

double GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionAICOmp(GwmBandwidthWeight *bandwidthWeight)
{
    int n = mDataPoints.n_rows;
    vec cv = vec(n);
    mat S = mat(n,n);
    mat wt = mat(n,n);
    int current1 = 0, current2 = 0;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < n; i++)
    {
        if(!checkCanceled())
        {
            vec d = mSpatialWeight.distance()->distance(i);
            vec w = bandwidthWeight->weight(d);
            wt.col(i) = w;
            if(mBandwidthSizeSelector.counter<10)
                emit tick(mBandwidthSizeSelector.counter*10 + current1 * 5 / n, 100);
            current1++;
        }
    }
    if (!checkCanceled())  (this->*mCalWtFunction)(mX,mY,wt);
    vec trS = vec(mOmpThreadNum,fill::zeros);
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < n; i++){
        if(!checkCanceled())
        {
            int thread = omp_get_thread_num();
            vec wi = wt.col(i) % mWt2;
            mat Ci = CiMat(mX,wi);
            S.row(i) = mX.row(i) * Ci;
            trS(thread) += S(i,i);
            if(mBandwidthSizeSelector.counter<10)
                emit tick(mBandwidthSizeSelector.counter*10 + current2 * 5 / n + 5, 100);
            current2++;
        }
    }
    double AICc;
    if(!checkCanceled())
    {
        if(S.is_finite()){
            double trs = double(sum(trS));
            AICc = -2*mLLik + 2*trs + 2*trs*(trs+1)/(n-trs-1);
        }
        else{
            AICc = qInf();
        }
    }

    if(!checkCanceled())
    {
        QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
                .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
                .arg(bandwidthWeight->bandwidth())
                .arg(AICc);
        emit message(msg);
        return AICc;
    }
    else return DBL_MAX;
}

mat GwmGeneralizedGWRAlgorithm::PoissonWtSerial(const mat &x, const vec &y, mat wt){
    int varn = x.n_cols;
    int dpn = x.n_rows;
    mat betas = mat(varn, dpn, fill::zeros);
    mat S = mat(dpn,dpn);
    int itCount = 0;
    double oldLLik = 0.0;
    vec mu = y + 0.1;
    vec nu = log(mu);
    vec cv = vec(dpn);
    mWt2 = ones(dpn);
    mLLik = 0;

    while(!checkCanceled()){
        myAdj = nu + (y - mu)/mu;
        for (int i = 0; i < dpn & !checkCanceled(); i++)
        {
            vec wi = wt.col(i);
            vec gwsi = gwReg(x, myAdj, wi % mWt2, i);
            betas.col(i) = gwsi;
        }
        mat betas1 = trans(betas);
        nu = Fitted(x,betas1);
        mu = exp(nu);
        oldLLik = mLLik;
        vec lliktemp = dpois(y,mu);
        mLLik = sum(lliktemp);
        if (abs((oldLLik - mLLik)/mLLik) < mTol)
            break;
        mWt2 = mu;
        itCount++;
        if (itCount == mMaxiter)
            break;
    }
//    return cv;
    return mu;
}

mat GwmGeneralizedGWRAlgorithm::PoissonWtOmp(const mat &x, const vec &y, mat wt){
    int varn = x.n_cols;
    int dpn = x.n_rows;
    mat betas = mat(varn, dpn, fill::zeros);
    mat S = mat(dpn,dpn);
    int itCount = 0;
    double oldLLik = 0.0;
    vec mu = y + 0.1;
    vec nu = log(mu);
    vec cv = vec(dpn);
    mWt2 = ones(dpn);
    mLLik = 0;
    while(!checkCanceled()){
        myAdj = nu + (y - mu)/mu;
        for (int i = 0; i < dpn & !checkCanceled(); i++)
        {
            vec wi = wt.col(i);
            vec gwsi = gwReg(x, myAdj, wi % mWt2, i);
            betas.col(i) = gwsi;
        }
        mat betas1 = trans(betas);
        nu = Fitted(x,betas1);
        mu = exp(nu);
        oldLLik = mLLik;
        vec lliktemp = dpois(y,mu);
        mLLik = sum(lliktemp);
        if (abs((oldLLik - mLLik)/mLLik) < mTol)
            break;
        mWt2 = mu;
        itCount++;
        if (itCount == mMaxiter)
            break;
    }
//    return cv;
    return mu;
}

mat GwmGeneralizedGWRAlgorithm::BinomialWtSerial(const mat &x, const vec &y, mat wt){
    int varn = x.n_cols;
    int dpn = x.n_rows;
    mat betas = mat(varn, dpn, fill::zeros);
    mat S = mat(dpn,dpn);
    mat n = vec(y.n_rows,fill::ones);
    int itCount = 0;
//    double lLik = 0.0;
    double oldLLik = 0.0;
    vec mu = vec(dpn,fill::ones) * 0.5;
    vec nu = vec(dpn,fill::zeros);
//    vec cv = vec(dpn);
    mWt2 = ones(dpn);
    mLLik = 0;
    while(!checkCanceled()){
        //计算公式有调整
        myAdj = nu + (y - mu)/(mu % (1 - mu));
        for (int i = 0; i < dpn & !checkCanceled(); i++)
        {
            vec wi = wt.col(i);
            vec gwsi = gwReg(x, myAdj, wi % mWt2, i);
            betas.col(i) = gwsi;
        }
        mat betas1 = trans(betas);
        nu = Fitted(x,betas1);
        mu = exp(nu)/(1 + exp(nu));
        oldLLik = mLLik;
        mLLik = sum(lchoose(n,y) + (n-y)%log(1 - mu/n) + y%log(mu/n));
        if (abs((oldLLik - mLLik)/mLLik) < mTol)
            break;
        mWt2 = n%mu%(1-mu);
        itCount++;
        if (itCount == mMaxiter)
            break;
    }
    return mu;
}

mat GwmGeneralizedGWRAlgorithm::BinomialWtOmp(const mat &x, const vec &y, mat wt){
    int varn = x.n_cols;
    int dpn = x.n_rows;
    mat betas = mat(varn, dpn, fill::zeros);
    mat S = mat(dpn,dpn);
    mat n = vec(y.n_rows,fill::ones);
    int itCount = 0;
//    double lLik = 0.0;
    double oldLLik = 0.0;
    vec mu = vec(dpn,fill::ones) * 0.5;
    vec nu = vec(dpn,fill::zeros);
//    vec cv = vec(dpn);
    mWt2 = ones(dpn);
    mLLik = 0;
    while(!checkCanceled()){
        //计算公式有调整
        myAdj = nu + (y - mu)/(mu % (1 - mu));
        for (int i = 0; i < dpn & !checkCanceled(); i++)
        {
            vec wi = wt.col(i);
            vec gwsi = gwReg(x, myAdj, wi % mWt2, i);
            betas.col(i) = gwsi;
        }
        mat betas1 = trans(betas);
        nu = Fitted(x,betas1);
        mu = exp(nu)/(1 + exp(nu));
        oldLLik = mLLik;
        mLLik = sum(lchoose(n,y) + (n-y)%log(1 - mu/n) + y%log(mu/n));
        if (abs((oldLLik - mLLik)/mLLik) < mTol)
            break;
        mWt2 = n%mu%(1-mu);
        itCount++;
        if (itCount == mMaxiter)
            break;
    }
    return mu;
}

void GwmGeneralizedGWRAlgorithm::createResultLayer(CreateResultLayerData data,QString name)
{
    QgsVectorLayer* srcLayer = mRegressionLayer ? mRegressionLayer : mDataLayer;
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    layerName += name;
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


void GwmGeneralizedGWRAlgorithm::setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType)
{
    mBandwidthSelectionCriterionType = bandwidthSelectionCriterionType;
    QMap<QPair<BandwidthSelectionCriterionType, IParallelalbe::ParallelType>, BandwidthSelectCriterionFunction> mapper = {
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, IParallelalbe::ParallelType::SerialOnly), &GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionCVSerial),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, IParallelalbe::ParallelType::OpenMP), &GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionCVOmp),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, IParallelalbe::ParallelType::CUDA), &GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionCVSerial),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, IParallelalbe::ParallelType::SerialOnly), &GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionAICSerial),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, IParallelalbe::ParallelType::OpenMP), &GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionAICOmp),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, IParallelalbe::ParallelType::CUDA), &GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionAICSerial)
    };
    mBandwidthSelectCriterionFunction = mapper[qMakePair(bandwidthSelectionCriterionType, mParallelType)];
}

void GwmGeneralizedGWRAlgorithm::setParallelType(const IParallelalbe::ParallelType &type)
{
    if (type & parallelAbility())
    {
        mParallelType = type;
        setBandwidthSelectionCriterionType(mBandwidthSelectionCriterionType);
        setFamily(mFamily);
    }
}

mat GwmGeneralizedGWRAlgorithm::diag(mat a){
    int n = a.n_rows;
    mat res = mat(uword(0), uword(0));
    if(a.n_cols > 1){
        res = vec(a.n_rows);
        for(int i = 0; i < n; i++){
            res[i] = a.row(i)[i];
        }
    }
    else{
        res = mat(a.n_rows,a.n_rows);
        mat base = eye(n,n);
        for(int i = 0; i < n; i++){
            res.row(i) = a[i] * base.row(i);
        }
    }
    return res;
}

//GWR clalibration
vec GwmGeneralizedGWRAlgorithm::gwReg(const mat& x, const vec &y, const vec &w, int focus)
{
    mat wspan(1, x.n_cols, fill::ones);
    mat xtw = trans(x % (w * wspan));
    mat xtwx = xtw * x;
    mat xtwy = xtw * y;
    mat xtwx_inv = inv(xtwx);
    vec beta = xtwx_inv * xtwy;
    return beta;
}

vec GwmGeneralizedGWRAlgorithm::gwRegHatmatrix(const mat &x, const vec &y, const vec &w, int focus, mat& ci, mat& s_ri)
{
    mat wspan(1, x.n_cols, fill::ones);
    mat xtw = trans(x % (w * wspan));
    mat xtwx = xtw * x;
    mat xtwy = xtw * y;
    mat xtwx_inv = inv(xtwx);
    vec beta = xtwx_inv * xtwy;
    ci = xtwx_inv * xtw;
    s_ri = x.row(focus) * ci;
    return beta;
}

mat GwmGeneralizedGWRAlgorithm::dpois(mat y,mat mu){
    int n = y.n_rows;
    mat res = vec(n);
    mat pdf = lgamma(y+1);
    res = -mu + y%log(mu) - pdf;
    return res;
}

mat GwmGeneralizedGWRAlgorithm::lchoose(mat n,mat k){
    int nrow = n.n_rows;
    mat res = vec(nrow);
//    for(int i = 0;i < nrow; i++){
//        res.row(i) = lgamma(n[i]+1) - lgamma(n[i]-k[i]+1) - lgamma(k[i]+1);
//    }
    res = lgamma(n+1) - lgamma(n-k+1) - lgamma(k+1);
    return res;
}

mat GwmGeneralizedGWRAlgorithm::dbinom(mat y,mat m,mat mu){
    int n = y.n_rows;
    mat res = vec(n);
    for(int i = 0;i < n; i++){
        double pdf = gsl_ran_binomial_pdf(int(y[i]), mu[i], int(m[i]));
        res[i] = log(pdf);
    }
    return res;
}

mat GwmGeneralizedGWRAlgorithm::lgammafn(mat x){
    int n = x.n_rows;
    mat res = vec(n,fill::zeros);
    for(int j = 0; j < n ; j++){
        res[j] = lgamma(x[j]);
    }
    return res;
}

mat GwmGeneralizedGWRAlgorithm::CiMat(const mat& x, const vec &w)
{
    return inv(trans(x) * diagmat(w) * x) * trans(x) * diagmat(w);
}

bool GwmGeneralizedGWRAlgorithm::setFamily(Family family){
    mFamily = family;
    QMap<QPair<Family, IParallelalbe::ParallelType>, GGWRRegressionFunction> mapper = {
        std::make_pair(qMakePair(Family::Poisson, IParallelalbe::ParallelType::SerialOnly), &GwmGeneralizedGWRAlgorithm::regressionPoissonSerial),
        std::make_pair(qMakePair(Family::Poisson, IParallelalbe::ParallelType::OpenMP), &GwmGeneralizedGWRAlgorithm::regressionPoissonOmp),
        std::make_pair(qMakePair(Family::Poisson, IParallelalbe::ParallelType::CUDA), &GwmGeneralizedGWRAlgorithm::regressionPoissonSerial),
        std::make_pair(qMakePair(Family::Binomial, IParallelalbe::ParallelType::SerialOnly), &GwmGeneralizedGWRAlgorithm::regressionBinomialSerial),
        std::make_pair(qMakePair(Family::Binomial, IParallelalbe::ParallelType::OpenMP), &GwmGeneralizedGWRAlgorithm::regressionBinomialOmp),
        std::make_pair(qMakePair(Family::Binomial, IParallelalbe::ParallelType::CUDA), &GwmGeneralizedGWRAlgorithm::regressionBinomialSerial)
    };
    mGGWRRegressionFunction = mapper[qMakePair(family, mParallelType)];
    QMap<QPair<Family, IParallelalbe::ParallelType>, CalWtFunction> mapper1 = {
        std::make_pair(qMakePair(Family::Poisson, IParallelalbe::ParallelType::SerialOnly), &GwmGeneralizedGWRAlgorithm::PoissonWtSerial),
        std::make_pair(qMakePair(Family::Poisson, IParallelalbe::ParallelType::OpenMP), &GwmGeneralizedGWRAlgorithm::PoissonWtOmp),
        std::make_pair(qMakePair(Family::Poisson, IParallelalbe::ParallelType::CUDA), &GwmGeneralizedGWRAlgorithm::PoissonWtSerial),
        std::make_pair(qMakePair(Family::Binomial, IParallelalbe::ParallelType::SerialOnly), &GwmGeneralizedGWRAlgorithm::BinomialWtSerial),
        std::make_pair(qMakePair(Family::Binomial, IParallelalbe::ParallelType::OpenMP), &GwmGeneralizedGWRAlgorithm::BinomialWtOmp),
        std::make_pair(qMakePair(Family::Binomial, IParallelalbe::ParallelType::CUDA), &GwmGeneralizedGWRAlgorithm::BinomialWtSerial)
    };
    mCalWtFunction = mapper1[qMakePair(family, mParallelType)];
    return true;
}

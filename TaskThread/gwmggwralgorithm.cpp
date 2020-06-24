#include "gwmggwralgorithm.h"

//#include "GWmodel/GWmodel.h"
//#include "gwmggwrbandwidthselectionthread.h"
#include "GWmodel/gwmgeneralizedlinearmodel.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <omp.h>
#include <exception>

using namespace std;

QMap<QString, double> GwmGGWRAlgorithm::TolUnitDict = {
    make_pair(QString("e -3"), 0.001),
    make_pair(QString("e -5"), 0.00001),
    make_pair(QString("e -7"), 0.0000001),
    make_pair(QString("e -10"), 0.0000000001)
};


GwmGGWRAlgorithm::GwmGGWRAlgorithm() : GwmBasicGWRAlgorithm()
{

}

void GwmGGWRAlgorithm::run()
{
    // 点位初始化
    emit message(QString(tr("Setting data points")) + (hasRegressionLayer() ? tr(" and regression points") : "") + ".");
    initPoints();

    // 初始化
    emit message(QString(tr("Setting X and Y.")));
    initXY(mX, mY, mDepVar, mIndepVars);

    // 优选带宽
    if (mIsAutoselectBandwidth)
    {
        emit message(QString(tr("Automatically selecting bandwidth ...")));
        emit tick(0, 0);
        if (mSpatialWeight.distance()->type() == GwmDistance::CRSDistance || mSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance)
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
        GwmBandwidthWeight* bandwidthWeight = mBandwidthSizeSelector.optimize(this);
        if (bandwidthWeight)
        {
            mSpatialWeight.setWeight(bandwidthWeight);
            // 绘图
            QVariant data = QVariant::fromValue(mBandwidthSizeSelector.bandwidthCriterion());
            emit plot(data, &GwmBandwidthSizeSelector::PlotBandwidthResult);
        }
        if (mSpatialWeight.distance()->type() == GwmDistance::CRSDistance || mSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance)
        {
            GwmCRSDistance* d = static_cast<GwmCRSDistance*>(mSpatialWeight.distance());
            d->setDataPoints(&mDataPoints);
            d->setFocusPoints(&mRegressionPoints);
        }
    }

    int nVar = mX.n_cols;
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
    mBetas = mat(nVar, nRp, fill::zeros);
    if (mHasHatMatrix)
    {
        mBetasSE = mat( nVar,nDp, fill::zeros);
        mShat = vec(2,fill::zeros);
    }

    emit message(tr("Calculating Distance Matrix..."));
    mWtMat1 = mat(nDp,nDp,fill::zeros);
    if(mRegressionLayer){
        mWtMat2 = mat(nRp,nDp,fill::zeros);
    }
    else{
        mWtMat2 = mat(nDp,nDp,fill::zeros);
    }
    if(mRegressionLayer){
        for(int i = 0; i < nRp; i++){
            vec weight = mSpatialWeight.spatialWeight(i);
            mWtMat2.col(i) = weight;
        }
        if (mSpatialWeight.distance()->type() == GwmDistance::CRSDistance || mSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance)
        {
            GwmCRSDistance* d = static_cast<GwmCRSDistance*>(mSpatialWeight.distance());
            d->setDataPoints(&mDataPoints);
            d->setFocusPoints(&mDataPoints);
        }
        for(int i = 0; i < nDp; i++){
            vec weight = mSpatialWeight.spatialWeight(i);
            mWtMat1.col(i) = weight;
        }
    }
    else{
        for(int i = 0; i < nRp; i++){
            vec weight = mSpatialWeight.spatialWeight(i);
            mWtMat2.col(i) = weight;
        }
        mWtMat1 = mWtMat2;
    }

    bool isAllCorrect = true;
//    if(mFamily == Family::Poisson){
//        isAllCorrect = gwrPoisson();
//    }
//    else{
//        isAllCorrect = gwrBinomial();
//    }
    isAllCorrect = (this->*mGGWRRegressionFunction)();

    if(mHasHatMatrix){
//        GwmGGWRBandWidthSelectionThread* taskThread = new GwmGGWRBandWidthSelectionThread(*this);
//        mat CV = taskThread->cvContrib(mX,mY,mDataPoints,mBandwidthSize,mBandwidthKernelFunction,mBandwidthType == BandwidthType::Adaptive);
//        delete taskThread;
//        taskThread = nullptr;
    }
    // Create Result Layer
    if (isAllCorrect)
    {
        createResultLayer(mResultList,QStringLiteral("_GGWR"));
    }
    emit success();
}

bool GwmGGWRAlgorithm::gwrPoissonSerial()
{
    int itCount = 0;
    double lLik = 0.0;
    double oldLLik = 0.0;
    mat mu = mY + 0.1;
    mat nu = log(mu);
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
//    mat wt2 = ones(nDp);
//    mat yAdj = vec(nDp);
    int nVar = mX.n_cols;
    mat yhat,res,betasTV;

    emit message(tr("Calibrating GLM model..."));
    GwmGeneralizedLinearModel* glm = new GwmGeneralizedLinearModel();
    glm->setX(mX);
    glm->setY(mY);
    glm->setFamily(mFamily);
    glm->fit();
    double nulldev = glm->nullDev();
    double dev = glm->dev();
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
    mu = (this->*mCalWtFunction)(mX,mY,mWtMat1);

    double gwDev = 0.0;
    for(int i = 0; i < nDp; i++){
        if(mY[i] != 0){
            gwDev = gwDev +  2*(mY[i]*(log(mY[i]/mu[i])-1)+mu[i]);
        }
        else{
            gwDev = gwDev + 2* mu[i];
        }
    }
    emit message(tr("Calibrating GGWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;
    bool isStoreS = (nDp <= 8192);
    mat ci, s_ri, S(isStoreS ? nDp : 1, nDp, fill::zeros);
    if(mHasHatMatrix){
        for(int i = 0; i < nDp; i++){
            try{
                vec wi = mWtMat2.col(i);
                vec gwsi = gwRegHatmatrix(mX, myAdj, wi % mWt2, i, ci, s_ri);
                mBetas.col(i) = gwsi;
                mat invwt2 = 1.0 / mWt2;
                S.row(isStoreS ? i : 0) = s_ri;
                mat temp = mat(ci.n_rows,ci.n_cols);
                for(int j = 0; j < ci.n_rows; j++){
                    temp.row(j) = ci.row(j) % trans(invwt2);
                }
                mBetasSE.col(i) = diag(temp * trans(ci));

                mShat(0) += s_ri(0, i);
                mShat(1) += det(s_ri * trans(s_ri));

                mBetasSE.col(i) = sqrt(mBetasSE.col(i));
    //            mBetasTV.col(i) = mBetas.col(i) / mBetasSE.col(i);

                emit tick(i + 1, nDp);
            }
            catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }

        }
        if(isAllCorrect){
            betasTV = mBetas / mBetasSE;
            mBetas = trans(mBetas);
            mBetasSE = trans(mBetasSE);
            betasTV = trans(betasTV);
            double trS = mShat(0);
            double trStS = mShat(1);

            yhat = exp(Fitted(mX,mBetas));
            res = mY - yhat;

            //计算诊断信息
            double AIC = gwDev + 2 * trS;
            double AICc = AIC + 2*trS*(trS+1)/(nDp-trS-1);
            double R2 = 1 - gwDev/(nulldev);  // pseudo.R2 <- 1 - gw.dev/null.dev
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
        for(int i = 0; i < nRp; i++){
            try{
                vec wi = mWtMat2.col(i);
                vec gwsi = gwReg(mX, myAdj, wi * mWt2, i);
                mBetas.col(i) = gwsi;
                emit tick(i + 1,nRp);
            }
            catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        mBetas = trans(mBetas);

        mResultList.push_back(qMakePair(QString("%1"), mBetas));
    }
    return isAllCorrect;
}

bool GwmGGWRAlgorithm::gwrPoissonOmp()
{
    int itCount = 0;
    double lLik = 0.0;
    double oldLLik = 0.0;
    mat mu = mY + 0.1;
    mat nu = log(mu);
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
//    mat wt2 = ones(nDp);
//    mat yAdj = vec(nDp);
    int nVar = mX.n_cols;
    mat yhat,res,betasTV;

    emit message(tr("Calibrating GLM model..."));
    GwmGeneralizedLinearModel* glm = new GwmGeneralizedLinearModel();
    glm->setX(mX);
    glm->setY(mY);
    glm->setFamily(mFamily);
    glm->fit();
    double nulldev = glm->nullDev();
    double dev = glm->dev();
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
    mu = (this->*mCalWtFunction)(mX,mY,mWtMat1);

    double gwDev = 0.0;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for(int i = 0; i < nDp; i++){
        if(mY[i] != 0){
            gwDev = gwDev +  2*(mY[i]*(log(mY[i]/mu[i])-1)+mu[i]);
        }
        else{
            gwDev = gwDev + 2* mu[i];
        }
    }
    emit message(tr("Calibrating GGWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;
    bool isStoreS = (nDp <= 8192);
    mat ci, s_ri, S(isStoreS ? nDp : 1, nDp, fill::zeros);
    if(mHasHatMatrix){
#pragma omp parallel for num_threads(mOmpThreadNum)
        for(int i = 0; i < nDp; i++){
            try{
                vec wi = mWtMat2.col(i);
                vec gwsi = gwRegHatmatrix(mX, myAdj, wi % mWt2, i, ci, s_ri);
                mBetas.col(i) = gwsi;
                mat invwt2 = 1.0 / mWt2;
                S.row(isStoreS ? i : 0) = s_ri;
                mat temp = mat(ci.n_rows,ci.n_cols);
                for(int j = 0; j < ci.n_rows; j++){
                    temp.row(j) = ci.row(j) % trans(invwt2);
                }
                mBetasSE.col(i) = diag(temp * trans(ci));

                mShat(0) += s_ri(0, i);
                mShat(1) += det(s_ri * trans(s_ri));

                mBetasSE.col(i) = sqrt(mBetasSE.col(i));
    //            mBetasTV.col(i) = mBetas.col(i) / mBetasSE.col(i);

                emit tick(i + 1, nDp);
            }
            catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }

        }
        if(isAllCorrect){
            betasTV = mBetas / mBetasSE;
            mBetas = trans(mBetas);
            mBetasSE = trans(mBetasSE);
            betasTV = trans(betasTV);
            double trS = mShat(0);
            double trStS = mShat(1);

            yhat = exp(Fitted(mX,mBetas));
            res = mY - yhat;

            //计算诊断信息
            double AIC = gwDev + 2 * trS;
            double AICc = AIC + 2*trS*(trS+1)/(nDp-trS-1);
            double R2 = 1 - gwDev/(nulldev);  // pseudo.R2 <- 1 - gw.dev/null.dev
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
#pragma omp parallel for num_threads(mOmpThreadNum)
        for(int i = 0; i < nRp; i++){
            try{
                vec wi = mWtMat2.col(i);
                vec gwsi = gwReg(mX, myAdj, wi * mWt2, i);
                mBetas.col(i) = gwsi;
                emit tick(i + 1,nRp);
            }
            catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        mBetas = trans(mBetas);

        mResultList.push_back(qMakePair(QString("%1"), mBetas));
    }
    return isAllCorrect;
}

bool GwmGGWRAlgorithm::gwrBinomialOmp()
{
    int nVar = mX.n_cols;
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
//    mat S = mat(nDp,nDp);
    mat n = vec(mY.n_rows,fill::ones);
    int itCount = 0;
    double lLik = 0.0;
    double oldLLik = 0.0;
    vec mu = vec(nDp,fill::ones) * 0.5;
    vec nu = vec(nDp,fill::zeros);
//    vec wt2 = ones(nDp);
//    vec cv = vec(nDp);
//    mat yAdj = vec(nDp);
    mat yhat,res,betasTV;

    emit message(tr("Calibrating GLM model..."));
    GwmGeneralizedLinearModel* glm = new GwmGeneralizedLinearModel();
    glm->setX(mX);
    glm->setY(mY);
    glm->setFamily(mFamily);
    glm->fit();
    double nulldev = glm->nullDev();
    double dev = glm->dev();
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

    mu = (this->*mCalWtFunction)(mX,mY,mWtMat1);
    emit message(tr("Calibrating GGWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;

    bool isStoreS = (nDp <= 8192);
    mat ci;
    mat s_ri, S(isStoreS ? nDp : 1, nDp, fill::zeros);;
//    mat S = mat(uword(0), uword(0));
    if(mHasHatMatrix){
#pragma omp parallel for num_threads(mOmpThreadNum)
        for(int i = 0; i < nDp; i++){
            try {
                vec wi = mWtMat1.col(i);
                vec gwsi = gwRegHatmatrix(mX, myAdj, wi % mWt2, i, ci, s_ri);
                mat invwt2 = 1.0 / mWt2;
                S.row(isStoreS ? i : 0) = s_ri;
                mat temp = mat(ci.n_rows,ci.n_cols);
                for(int j = 0; j < ci.n_rows; j++){
                    temp.row(j) = ci.row(j) % trans(invwt2);
                }
                mBetasSE.col(i) = diag(temp * trans(ci));
                emit tick(i + 1, nDp);
            }
            catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        if(isAllCorrect){
            mBetas = trans(mBetas);

            double trS = mShat(0);
            double trStS = mShat(1);

            yhat = Fitted(mX,mBetas);
            yhat = exp(yhat)/(1+exp(yhat));

            res = mY - yhat;
            vec Dev = log(1/( (mY-n+yhat) % (mY-n+yhat) ) );
            double gwDev = sum(Dev);
            vec residual2 = res % res;
            double rss = sum(residual2);
            for(int i = 0; i < nDp; i++){
                mBetasSE.col(i) = sqrt(mBetasSE.col(i));
    //            mBetasTV.col(i) = mBetas.col(i) / mBetasSE.col(i);
            }
            mBetasSE = trans(mBetasSE);
            betasTV = mBetas / mBetasSE;

            double AIC = gwDev + 2 * trS;
            double AICc = AIC + 2*trS*(trS+1)/(nDp-trS-1);
            double R2 = 1 - gwDev/(nulldev);  // pseudo.R2 <- 1 - gw.dev/null.dev
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
#pragma omp parallel for num_threads(mOmpThreadNum)
        for(int i = 0; i < nRp; i++){
            try {
                vec wi = mWtMat2.col(i);
                vec gwsi = gwReg(mX, myAdj, wi * mWt2, i);
                mBetas.col(i) = gwsi;
                emit tick(i + 1, nRp);
            }
            catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        mBetas = trans(mBetas);
    }

    return isAllCorrect;
}

bool GwmGGWRAlgorithm::gwrBinomialSerial()
{
    int nVar = mX.n_cols;
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
//    mat S = mat(nDp,nDp);
    mat n = vec(mY.n_rows,fill::ones);
    int itCount = 0;
    double lLik = 0.0;
    double oldLLik = 0.0;
    vec mu = vec(nDp,fill::ones) * 0.5;
    vec nu = vec(nDp,fill::zeros);
//    vec wt2 = ones(nDp);
//    vec cv = vec(nDp);
//    mat yAdj = vec(nDp);
    mat yhat,res,betasTV;

    emit message(tr("Calibrating GLM model..."));
    GwmGeneralizedLinearModel* glm = new GwmGeneralizedLinearModel();
    glm->setX(mX);
    glm->setY(mY);
    glm->setFamily(mFamily);
    glm->fit();
    double nulldev = glm->nullDev();
    double dev = glm->dev();
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

    mu = (this->*mCalWtFunction)(mX,mY,mWtMat1);
    emit message(tr("Calibrating GGWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;

    bool isStoreS = (nDp <= 8192);
    mat ci;
    mat s_ri, S(isStoreS ? nDp : 1, nDp, fill::zeros);;
//    mat S = mat(uword(0), uword(0));
    if(mHasHatMatrix){
        for(int i = 0; i < nDp; i++){
            try {
                vec wi = mWtMat1.col(i);
                vec gwsi = gwRegHatmatrix(mX, myAdj, wi % mWt2, i, ci, s_ri);
                mat invwt2 = 1.0 / mWt2;
                S.row(isStoreS ? i : 0) = s_ri;
                mat temp = mat(ci.n_rows,ci.n_cols);
                for(int j = 0; j < ci.n_rows; j++){
                    temp.row(j) = ci.row(j) % trans(invwt2);
                }
                mBetasSE.col(i) = diag(temp * trans(ci));
                emit tick(i + 1, nDp);
            }
            catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        if(isAllCorrect){
            mBetas = trans(mBetas);

            double trS = mShat(0);
            double trStS = mShat(1);

            yhat = Fitted(mX,mBetas);
            yhat = exp(yhat)/(1+exp(yhat));

            res = mY - yhat;
            vec Dev = log(1/( (mY-n+yhat) % (mY-n+yhat) ) );
            double gwDev = sum(Dev);
            vec residual2 = res % res;
            double rss = sum(residual2);
            for(int i = 0; i < nDp; i++){
                mBetasSE.col(i) = sqrt(mBetasSE.col(i));
    //            mBetasTV.col(i) = mBetas.col(i) / mBetasSE.col(i);
            }
            mBetasSE = trans(mBetasSE);
            betasTV = mBetas / mBetasSE;

            double AIC = gwDev + 2 * trS;
            double AICc = AIC + 2*trS*(trS+1)/(nDp-trS-1);
            double R2 = 1 - gwDev/(nulldev);  // pseudo.R2 <- 1 - gw.dev/null.dev
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
        for(int i = 0; i < nRp; i++){
            try {
                vec wi = mWtMat2.col(i);
                vec gwsi = gwReg(mX, myAdj, wi * mWt2, i);
                mBetas.col(i) = gwsi;
                emit tick(i + 1, nRp);
            }
            catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        mBetas = trans(mBetas);
    }

    return isAllCorrect;
}

double GwmGGWRAlgorithm::bandwidthSizeGGWRCriterionCVSerial(GwmBandwidthWeight *bandwidthWeight)
{
    int n = mDataPoints.n_rows;
    vec cv = vec(n);
    mat wt = mat(n,n);
    for (int i = 0; i < n; i++)
    {
        vec d = mSpatialWeight.distance()->distance(i);
        vec w = bandwidthWeight->weight(d);
        w.row(i) = 0;
        wt.col(i) = w;
    }
//    if(mFamily == GwmGGWRAlgorithm::Family::Poisson){
//        PoissonWt(mX,mY,wt);
//    }
//    else{
//        BinomialWt(mX,mY,wt);
//    }
    (this->*mCalWtFunction)(mX,mY,wt);
    for (int i = 0; i < n; i++){
        mat wi = wt.col(i) % mWt2;
        vec gwsi = gwReg(mX, myAdj, wi, i);
        mat yhatnoi = mX.row(i) * gwsi;
        if(mFamily == GwmGGWRAlgorithm::Family::Poisson){
            cv.row(i) = mY.row(i) - exp(yhatnoi);
        }
        else{
            cv.row(i) = mY.row(i) - exp(yhatnoi)/(1+exp(yhatnoi));
        }
    }
    vec cvsquare = trans(cv) * cv ;
    double res = sum(cvsquare);
//    this->mBwScore.insert(bw,res);
    QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
            .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
            .arg(bandwidthWeight->bandwidth())
            .arg(res);
    emit message(msg);
    return res;

}

double GwmGGWRAlgorithm::bandwidthSizeGGWRCriterionCVOmp(GwmBandwidthWeight *bandwidthWeight)
{
    int n = mDataPoints.n_rows;
    vec cv = vec(n);
    mat wt = mat(n,n);
    for (int i = 0; i < n; i++)
    {
        vec d = mSpatialWeight.distance()->distance(i);
        vec w = bandwidthWeight->weight(d);
        w.row(i) = 0;
        wt.col(i) = w;
    }
//    if(mFamily == GwmGGWRAlgorithm::Family::Poisson){
//        PoissonWt(mX,mY,wt);
//    }
//    else{
//        BinomialWt(mX,mY,wt);
//    }
    (this->*mCalWtFunction)(mX,mY,wt);
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < n; i++){
        mat wi = wt.col(i) % mWt2;
        vec gwsi = gwReg(mX, myAdj, wi, i);
        mat yhatnoi = mX.row(i) * gwsi;
        if(mFamily == GwmGGWRAlgorithm::Family::Poisson){
            cv.row(i) = mY.row(i) - exp(yhatnoi);
        }
        else{
            cv.row(i) = mY.row(i) - exp(yhatnoi)/(1+exp(yhatnoi));
        }
    }
    vec cvsquare = trans(cv) * cv ;
    double res = sum(cvsquare);
//    this->mBwScore.insert(bw,res);
    QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
            .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
            .arg(bandwidthWeight->bandwidth())
            .arg(res);
    emit message(msg);
    return res;

}

double GwmGGWRAlgorithm::bandwidthSizeGGWRCriterionAICSerial(GwmBandwidthWeight *bandwidthWeight)
{
    int n = mDataPoints.n_rows;
    vec cv = vec(n);
    mat S = mat(n,n);
    mat wt = mat(n,n);
    for (int i = 0; i < n; i++)
    {
        vec d = mSpatialWeight.distance()->distance(i);
        vec w = bandwidthWeight->weight(d);
        wt.col(i) = w;
    }
//    if(mFamily == GwmGGWRAlgorithm::Family::Poisson){
//        PoissonWt(mX,mY,wt);
//    }
//    else{
//        BinomialWt(mX,mY,wt);
//    }
    (this->*mCalWtFunction)(mX,mY,wt);
    vec trS = vec(1,fill::zeros);
    for (int i = 0; i < n; i++){
        vec wi = wt.col(i) % mWt2;
        mat Ci = CiMat(mX,wi);
        S.row(i) = mX.row(i) * Ci;
        trS(0) += S(i,i);
    }
    double AICc;
    if(S.is_finite()){
        double trs = double(trS(0));
        AICc = -2*mLLik + 2*trs + 2*trs*(trs+1)/(n-trs-1);
    }
    else{
        AICc = qInf();
    }

    QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
            .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
            .arg(bandwidthWeight->bandwidth())
            .arg(AICc);
    emit message(msg);
    return AICc;
}

double GwmGGWRAlgorithm::bandwidthSizeGGWRCriterionAICOmp(GwmBandwidthWeight *bandwidthWeight)
{
    int n = mDataPoints.n_rows;
    vec cv = vec(n);
    mat S = mat(n,n);
    mat wt = mat(n,n);
    for (int i = 0; i < n; i++)
    {
        vec d = mSpatialWeight.distance()->distance(i);
        vec w = bandwidthWeight->weight(d);
        wt.col(i) = w;
    }
//    if(mFamily == GwmGGWRAlgorithm::Family::Poisson){
//        PoissonWt(mX,mY,wt);
//    }
//    else{
//        BinomialWt(mX,mY,wt);
//    }
    (this->*mCalWtFunction)(mX,mY,wt);
    vec trS = vec(1,fill::zeros);
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < n; i++){
        vec wi = wt.col(i) % mWt2;
        mat Ci = CiMat(mX,wi);
        S.row(i) = mX.row(i) * Ci;
        trS(0) += S(i,i);
    }
    double AICc;
    if(S.is_finite()){
        double trs = double(trS(0));
        AICc = -2*mLLik + 2*trs + 2*trs*(trs+1)/(n-trs-1);
    }
    else{
        AICc = qInf();
    }

    QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
            .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
            .arg(bandwidthWeight->bandwidth())
            .arg(AICc);
    emit message(msg);
    return AICc;
}

mat GwmGGWRAlgorithm::PoissonWtSerial(const mat &x, const vec &y, mat wt){
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

    while(1){
        myAdj = nu + (y - mu)/mu;
        for (int i = 0; i < dpn; i++)
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

mat GwmGGWRAlgorithm::PoissonWtOmp(const mat &x, const vec &y, mat wt){
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
#pragma omp parallel for num_threads(mOmpThreadNum)
    for(int k = 0; k < mMaxiter; k++){
        myAdj = nu + (y - mu)/mu;
        for (int i = 0; i < dpn; i++)
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

mat GwmGGWRAlgorithm::BinomialWtSerial(const mat &x, const vec &y, mat wt){
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
    while(1){
        //计算公式有调整
        myAdj = nu + (y - mu)/(mu % (1 - mu));
        for (int i = 0; i < dpn; i++)
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

mat GwmGGWRAlgorithm::BinomialWtOmp(const mat &x, const vec &y, mat wt){
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
#pragma omp parallel for num_threads(mOmpThreadNum)
    for(int k = 0; k < mMaxiter; k++){
        //计算公式有调整
        myAdj = nu + (y - mu)/(mu % (1 - mu));
        for (int i = 0; i < dpn; i++)
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


void GwmGGWRAlgorithm::setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType)
{
    mBandwidthSelectionCriterionType = bandwidthSelectionCriterionType;
    QMap<QPair<BandwidthSelectionCriterionType, IParallelalbe::ParallelType>, BandwidthSelectCriterionFunction> mapper = {
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, IParallelalbe::ParallelType::SerialOnly), &GwmGGWRAlgorithm::bandwidthSizeGGWRCriterionCVSerial),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, IParallelalbe::ParallelType::OpenMP), &GwmGGWRAlgorithm::bandwidthSizeGGWRCriterionCVOmp),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, IParallelalbe::ParallelType::CUDA), &GwmGGWRAlgorithm::bandwidthSizeGGWRCriterionCVSerial),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, IParallelalbe::ParallelType::SerialOnly), &GwmGGWRAlgorithm::bandwidthSizeGGWRCriterionAICSerial),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, IParallelalbe::ParallelType::OpenMP), &GwmGGWRAlgorithm::bandwidthSizeGGWRCriterionAICOmp),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, IParallelalbe::ParallelType::CUDA), &GwmGGWRAlgorithm::bandwidthSizeGGWRCriterionAICSerial)
    };
    mBandwidthSelectCriterionFunction = mapper[qMakePair(bandwidthSelectionCriterionType, mParallelType)];
}

void GwmGGWRAlgorithm::setParallelType(const IParallelalbe::ParallelType &type)
{
    if (type & parallelAbility())
    {
        mParallelType = type;
        setBandwidthSelectionCriterionType(mBandwidthSelectionCriterionType);
        setFamily(mFamily);
    }
}

mat GwmGGWRAlgorithm::diag(mat a){
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
vec GwmGGWRAlgorithm::gwReg(const mat& x, const vec &y, const vec &w, int focus)
{
    mat wspan(1, x.n_cols, fill::ones);
    mat xtw = trans(x % (w * wspan));
    mat xtwx = xtw * x;
    mat xtwy = xtw * y;
    mat xtwx_inv = inv(xtwx);
    vec beta = xtwx_inv * xtwy;
    return beta;
}

vec GwmGGWRAlgorithm::gwRegHatmatrix(const mat &x, const vec &y, const vec &w, int focus, mat& ci, mat& s_ri)
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

mat GwmGGWRAlgorithm::dpois(mat y,mat mu){
    int n = y.n_rows;
    mat res = vec(n);
    mat pdf = lgamma(y+1);
    res = -mu + y%log(mu) - pdf;
    return res;
}

mat GwmGGWRAlgorithm::lchoose(mat n,mat k){
    int nrow = n.n_rows;
    mat res = vec(nrow);
//    for(int i = 0;i < nrow; i++){
//        res.row(i) = lgamma(n[i]+1) - lgamma(n[i]-k[i]+1) - lgamma(k[i]+1);
//    }
    res = lgamma(n+1) - lgamma(n-k+1) - lgamma(k+1);
    return res;
}

mat GwmGGWRAlgorithm::dbinom(mat y,mat m,mat mu){
    int n = y.n_rows;
    mat res = vec(n);
    for(int i = 0;i < n; i++){
        double pdf = gsl_ran_binomial_pdf(int(y[i]), mu[i], int(m[i]));
        res[i] = log(pdf);
    }
    return res;
}

mat GwmGGWRAlgorithm::lgammafn(mat x){
    int n = x.n_rows;
    mat res = vec(n,fill::zeros);
    for(int j = 0; j < n ; j++){
        res[j] = lgamma(x[j]);
    }
    return res;
}

mat GwmGGWRAlgorithm::CiMat(const mat& x, const vec &w)
{
    return inv(trans(x) * diagmat(w) * x) * trans(x) * diagmat(w);
}

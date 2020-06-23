#include "gwmggwralgorithm.h"

#include "GWmodel/GWmodel.h"
//#include "gwmggwrbandwidthselectionthread.h"
#include "GWmodel/gwmgeneralizedlinearmodel.h"

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
    if(mFamily == Family::Poisson){
        isAllCorrect = gwrPoisson();
    }
    else{
        isAllCorrect = gwrBinomial();
    }

    if(mHasHatMatrix){
//        GwmGGWRBandWidthSelectionThread* taskThread = new GwmGGWRBandWidthSelectionThread(*this);
//        mat CV = taskThread->cvContrib(mX,mY,mDataPoints,mBandwidthSize,mBandwidthKernelFunction,mBandwidthType == BandwidthType::Adaptive);
//        delete taskThread;
//        taskThread = nullptr;
    }
    // Create Result Layer
    if (isAllCorrect)
    {
        createResultLayer(mResultList);
    }
    emit success();
}

bool GwmGGWRAlgorithm::gwrPoisson()
{
    int itCount = 0;
    double lLik = 0.0;
    double oldLLik = 0.0;
    mat mu = mY + 0.1;
    mat nu = log(mu);
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
    mat wt2 = ones(nDp);
    mat yAdj = vec(nDp);
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
    while(1){
        yAdj = nu + (mY - mu)/mu;
        for(int i = 0; i < nDp; i++){
            vec wi = mWtMat1.col(i);
            vec gwsi = gwReg(mX, yAdj, wi % wt2, i);
            mBetas.col(i) = gwsi;
        }
        mat mBetastemp = trans(mBetas);
        nu = gwFitted(mX,mBetastemp);
        mu = exp(nu);
        oldLLik = lLik;
        vec lliktemp = dpois(mY,mu);
        lLik = sum(lliktemp);
        if (abs((oldLLik - lLik)/lLik) < mTol)
            break;
        wt2 = mu;
        itCount ++;
        if(itCount == mMaxiter)
            break;
    }
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
                vec gwsi = gwRegHatmatrix(mX, yAdj, wi % wt2, i, ci, s_ri);
                mBetas.col(i) = gwsi;
                mat invwt2 = 1.0 / wt2;
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

            yhat = exp(gwFitted(mX,mBetas));
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
                vec gwsi = gwReg(mX, yAdj, wi * wt2, i);
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

bool GwmGGWRAlgorithm::gwrBinomial()
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
    vec wt2 = ones(nDp);
//    vec cv = vec(nDp);
    mat yAdj = vec(nDp);
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

    while(1){
        //计算公式有调整
        yAdj = nu + (mY - mu)/(mu%(1 - mu));
        for (int i = 0; i < nDp; i++)
        {
            vec wi = mWtMat1.col(i);
            vec gwsi = gwReg(mX, yAdj, wi % wt2, i);
            mBetas.col(i) = gwsi;
        }
        mat mBetastemp = trans(mBetas);
        nu = gwFitted(mX,mBetastemp);
        mu = exp(nu)/(1 + exp(nu));
        oldLLik = lLik;
        lLik = sum(lchoose(n,mY) + (n-mY)%log(1 - mu/n) + mY%log(mu/n));
        if (abs((oldLLik - lLik)/lLik) < mTol)
            break;
        wt2 = n%mu%(1-mu);
        itCount++;
        if (itCount == mMaxiter)
            break;
    }

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
                vec gwsi = gwRegHatmatrix(mX, yAdj, wi % wt2, i, ci, s_ri);
                mat invwt2 = 1.0 / wt2;
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

            yhat = gwFitted(mX,mBetas);
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
                vec gwsi = gwReg(mX, yAdj, wi * wt2, i);
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

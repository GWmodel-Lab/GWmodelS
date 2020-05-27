#include "gwmggwrtaskthread.h"
#include "GWmodel/GWmodel.h"
#include "gwmggwrbandwidthselectionthread.h"
#include "GWmodel/gwmgeneralizedlinearmodel.h"

#include <exception>

using namespace std;


GwmGGWRTaskThread::GwmGGWRTaskThread()
    :GwmGWRTaskThread()
{
    mWtMat1 = mat(uword(0), uword(0));
    mWtMat2 = mat(uword(0), uword(0));
    isCV = true;
    mMaxiter = 20;
    mTol = 1.0e-5;
    mFamily = Family::Poisson;
}

GwmGGWRTaskThread::GwmGGWRTaskThread(const GwmGGWRTaskThread &taskThread)
    :GwmGWRTaskThread(taskThread)
{
    mWtMat1 = taskThread.getWtMat1();
    mWtMat2 = taskThread.getWtMat2();
    isCV = taskThread.getIsCV();
    mMaxiter = taskThread.getMaxiter();
    mTol = taskThread.getTol();
    mFamily = taskThread.getFamily();
}

void GwmGGWRTaskThread::run(){
    //设置矩阵
    if (!setXY())
    {
        return;
    }
    if (isBandwidthSizeAutoSel)
    {
        emit message(tr("Selecting optimized bandwidth..."));
        GwmGGWRBandWidthSelectionThread bwSelThread(*this);
        connect(&bwSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
        connect(&bwSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
        connect(&bwSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
        bwSelThread.start();
        bwSelThread.wait();
        disconnect(&bwSelThread, &GwmTaskThread::message, this, &GwmTaskThread::message);
        disconnect(&bwSelThread, &GwmTaskThread::tick, this, &GwmTaskThread::tick);
        disconnect(&bwSelThread, &GwmTaskThread::error, this, &GwmTaskThread::error);
        mBandwidthSize = bwSelThread.getBandwidthSize();
        mBandwidthSelScore = bwSelThread.getBwScore();
        // 绘图
        QList<QVariant> plotData;
        for (auto i = mBandwidthSelScore.constBegin(); i != mBandwidthSelScore.constEnd(); i++)
        {
            plotData.append(QVariant(QPointF(i.key(), i.value())));
        }
        emit plot(QVariant(plotData), &GwmBandwidthSelectTaskThread::plotBandwidthResult);
    }
    if(mRegressionLayer){
        hasHatMatrix = false;
    }
    else{
        hasHatMatrix = true;
    }
    int nVar = mX.n_cols;
//    int nRp = mRegPoints.n_rows;
    int nDp = mLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
//    mat betas1 = mBetas;

    emit message(tr("Calculating Distance Matrix..."));
    mWtMat1 = mat(nDp,nDp,fill::zeros);
    if(mRegressionLayer){
        mWtMat2 = mat(nRp,nDp,fill::zeros);
    }
    else{
        mWtMat2 = mat(nDp,nDp,fill::zeros);
    }
    for(int i = 0; i < mFeatureList.size(); i++){
        vec dist = distance(i,mDataPoints);
        vec weight = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
        mWtMat1.col(i) = weight;
    }
    if(mRegressionLayer){
        for(int i = 0; i < nRp; i++){
            vec dist = distance(i,mRegPoints);
            vec weight = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
            mWtMat2.col(i) = weight;
        }
    }
    else{
        mWtMat2 = mWtMat1;
    }
     //model calibration
    bool isAllCorrect = true;
    if(mFamily == Family::Poisson){
        isAllCorrect = gwrPoisson();
    }
    else{
        isAllCorrect = gwrBinomial();
    }

    if(hasHatMatrix && isCV){
        GwmGGWRBandWidthSelectionThread* taskThread = new GwmGGWRBandWidthSelectionThread(*this);
        mat CV = taskThread->cvContrib(mX,mY,mDataPoints,mBandwidthSize,mBandwidthKernelFunction,mBandwidthType == BandwidthType::Adaptive);
        delete taskThread;
        taskThread = nullptr;
    }
    // Create Result Layer
    if (isAllCorrect)
    {
        createResultLayer();
    }
    emit success();
}


vec GwmGGWRTaskThread::distance(int focus, mat regPoints)
{
    switch (mDistSrcType)
    {
    case DistanceSourceType::Minkowski:
        return distanceMinkowski(focus,regPoints);
    case DistanceSourceType::DMatFile:
        return distanceDmat(focus,regPoints);
    default:
        return distanceCRS(focus,regPoints);
    }
}

vec GwmGGWRTaskThread::distanceCRS(int focus, mat regPoints)
{
    bool longlat = mLayer->crs().isGeographic();
    return gwDist(mDataPoints, regPoints, focus, 2.0, 0.0, longlat, mRegressionLayer != nullptr);
}

vec GwmGGWRTaskThread::distanceMinkowski(int focus, mat regPoints)
{
    QMap<QString, QVariant> parameters = mDistSrcParameters.toMap();
    double p = parameters["p"].toDouble();
    double theta = parameters["theta"].toDouble();
    return gwDist(mDataPoints, regPoints, focus, p, theta, false, mRegressionLayer != nullptr);
}

vec GwmGGWRTaskThread::distanceDmat(int focus, mat regPoints)
{
    QString filename = mDistSrcParameters.toString();
    qint64 featureCount = mFeatureList.size();
    QFile dmat(filename);
    if (dmat.open(QFile::QIODevice::ReadOnly))
    {
        qint64 basePos = 2 * sizeof (int);
        dmat.seek(basePos + focus * featureCount * sizeof (double));
        QByteArray values = dmat.read(featureCount * sizeof (double));
        return vec((double*)values.data(), featureCount);
    }
    else
    {
        return vec(featureCount, fill::zeros);
    }
}

bool GwmGGWRTaskThread::gwrPoisson(){
    int itCount = 0;
    double lLik = 0.0;
    double oldLLik = 0.0;
    mat mu = mY + 0.1;
    mat nu = log(mu);
    int nDp = mLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
    mat wt2 = ones(nDp);
    mat yAdj = vec(nDp);
    int nVar = mY.n_rows;

    emit message(tr("Calibrating GLM model..."));
    GwmGeneralizedLinearModel* glm = new GwmGeneralizedLinearModel();
    glm->setX(mX);
    glm->setY(mY);
    glm->setFamily(mFamily);
    glm->fit();
    double nulldev = glm->nullDev();
    double dev = glm->dev();
    double pseudor2 = 1- dev/nulldev;

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
//        lLik = sum(dpois(y, mu, log = TRUE));
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
//    mat ci;
//    mat s_ri;
//    mat S = mat(uword(0), uword(0));
    emit message(tr("Calibrating GGWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;
    bool isStoreS = hasFTest && (nDp <= 8192);
    mat ci, s_ri, S(isStoreS ? nDp : 1, nDp, fill::zeros);
    if(hasHatMatrix){
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

                mSHat(0) += s_ri(0, i);
                mSHat(1) += det(s_ri * trans(s_ri));

                mBetasSE.col(i) = sqrt(mBetasSE.col(i));
    //            mBetasTV.col(i) = mBetas.col(i) / mBetasSE.col(i);

                vec p = -trans(s_ri);
                p(i) += 1.0;
                mQDiag += p % p;

                emit tick(i + 1, mFeatureList.size());
            }
            catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }

        }
        if(isAllCorrect){
            mBetasTV = mBetas / mBetasSE;
            mBetas = trans(mBetas);
            mBetasSE = trans(mBetasSE);
            mBetasTV = trans(mBetasTV);
            double trS;
            double trStS;
            if(isStoreS){
                vec diagS = diag(S);
                trS = sum(diagS);
                vec diagStS = diag(S * diag(wt2) * trans(S) * diag(1/wt2));
                trStS =  sum(diagStS);
            }
            else{
                vec diagS = trans(S);
                trS = sum(diagS);
                vec temp = S * diag(wt2) * trans(S);
                vec diagStS = diag(double(temp[0]) * diag(1/wt2));
                trStS =  sum(diagStS);
            }

            mYHat = gwFitted(mX,mBetas);
            mat residual = mY - mYHat;
    //        for(int i = 0; i < nDp; i++){

    //        }
            double AIC = gwDev + 2 * trS;
            double AICc = AIC + 2*trS*(trS+1)/(nDp-trS-1);
            double R2 = 1 - gwDev/(nulldev);  // pseudo.R2 <- 1 - gw.dev/null.dev


            diagnostic();

            // F Test
            if (hasHatMatrix && hasFTest)
            {
                double trQtQ = DBL_MAX;
                if (isStoreS)
                {
                    mat EmS = eye(nDp, nDp) - S;
                    mat Q = trans(EmS) * EmS;
                    trQtQ = sum(diagvec(trans(Q) * Q));
                }
                else
                {
                    emit message(tr("Calculating the trace of matrix Q..."));
                    emit tick(0, nDp);
                    trQtQ = 0.0;
                    mat wspan(1, nVar, fill::ones);
                    for (arma::uword i = 0; i < nDp; i++)
                    {
                        vec di = distance(i,mRegPoints);
                        vec wi = gwWeight(di, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
                        mat xtwi = trans(mX % (wi * wspan));
                        mat si = mX.row(i) * inv(xtwi * mX) * xtwi;
                        vec pi = -trans(si);
                        pi(i) += 1.0;
                        double qi = sum(pi % pi);
                        trQtQ += qi * qi;
                        for (arma::uword j = i + 1; j < nDp; j++)
                        {
                            vec dj = distance(j,mRegPoints);
                            vec wj = gwWeight(dj, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
                            mat xtwj = trans(mX % (wj * wspan));
                            mat sj = mX.row(j) * inv(xtwj * mX) * xtwj;
                            vec pj = -trans(sj);
                            pj(j) += 1.0;
                            double qj = sum(pi % pj);
                            trQtQ += qj * qj * 2.0;
                        }
                        emit tick(i + 1, nDp);
                    }
                }
                GwmFTestParameters fTestParams;
                fTestParams.nDp = mX.n_rows;
                fTestParams.nVar = mX.n_cols;
                fTestParams.trS = mSHat(0);
                fTestParams.trStS = mSHat(1);
                fTestParams.trQ = sum(mQDiag);
                fTestParams.trQtQ = trQtQ;
                fTestParams.bw = mBandwidthSize;
                fTestParams.adaptive = mBandwidthType == BandwidthType::Adaptive;
                fTestParams.kernel = mBandwidthKernelFunction;
                fTestParams.gwrRSS = sum(mResidual % mResidual);
                f1234Test(fTestParams);
            }
        }

    }
    else{
        for(int i = 0; i < nRp; i++){
            try{
                vec wi = mWtMat2.col(i);
                vec gwsi = gwReg(mX, yAdj, wi * wt2, i);
                mBetas.col(i) = gwsi;
                emit tick(i + 1, mFeatureList.size());
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

bool GwmGGWRTaskThread::gwrBinomial(){
//    double tol = 1.0e-5;
//    int maxiter = 20;
    int nVar = mX.n_cols;
    int nDp = mLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
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

    emit message(tr("Calibrating GLM model..."));
    GwmGeneralizedLinearModel* glm = new GwmGeneralizedLinearModel();
    glm->setX(mX);
    glm->setY(mY);
    glm->setFamily(mFamily);
    glm->fit();
    double nulldev = glm->nullDev();
    double dev = glm->dev();
    double pseudor2 = 1- dev/nulldev;

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
        wt2 = mu;
        itCount++;
        if (itCount == mMaxiter)
            break;
    }

    emit message(tr("Calibrating GGWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;

    bool isStoreS = hasFTest && (nDp <= 8192);
    mat ci;
    mat s_ri, S(isStoreS ? nDp : 1, nDp, fill::zeros);;
//    mat S = mat(uword(0), uword(0));
    if(hasHatMatrix){
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
                emit tick(i + 1, mFeatureList.size());
            }
            catch (exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
        mBetas = trans(mBetas);
        double trS;
        if(isStoreS){
            vec diagS = diag(S);
            trS = sum(diagS);
        }
        else{
            vec diagS = trans(S);
            trS = sum(diagS);
        }
        mYHat = gwFitted(mX,mBetas);
        mat residual = mY - exp(mYHat)/(1+exp(mYHat));
        vec Dev = log(1/((mY-n+exp(mYHat)/(1+exp(mYHat)))) % ((mY-n+exp(mYHat)/(1+exp(mYHat)))));
        double gwDev = sum(Dev);
        vec residual2 = residual % residual;
        double rss = sum(residual2);
        for(int i = 0; i < nDp; i++){
            mBetasSE.col(i) = sqrt(mBetasSE.col(i));
//            mBetasTV.col(i) = mBetas.col(i) / mBetasSE.col(i);
        }
        mBetasTV = mBetas / mBetasSE;
        mBetasSE = trans(mBetasSE);
        mBetasTV = trans(mBetasTV);

        double AIC = gwDev + 2 * trS;
        double AICc = AIC + 2*trS*(trS+1)/(nDp-trS-1);
        double R2 = 1 - gwDev/(nulldev);  // pseudo.R2 <- 1 - gw.dev/null.dev
    }
    else{
        for(int i = 0; i < nRp; i++){
            try {
                vec wi = mWtMat2.col(i);
                vec gwsi = gwReg(mX, yAdj, wi * wt2, i);
                mBetas.col(i) = gwsi;
                emit tick(i + 1, mFeatureList.size());
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

mat GwmGGWRTaskThread::diag(mat a){
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

GwmGGWRTaskThread::Family GwmGGWRTaskThread::getFamily() const
{
    return mFamily;
}

double GwmGGWRTaskThread::getTol() const
{
    return mTol;
}

int GwmGGWRTaskThread::getMaxiter() const
{
    return mMaxiter;
}

bool GwmGGWRTaskThread::getIsCV() const
{
    return isCV;
}

mat GwmGGWRTaskThread::getWtMat1() const
{
    return mWtMat1;
}

mat GwmGGWRTaskThread::getWtMat2() const
{
    return mWtMat2;
}

bool GwmGGWRTaskThread::setFamily(Family family){
    mFamily = family;
    return true;
}

bool GwmGGWRTaskThread::setTol(double tol){
    mTol = tol;
    return true;
}

bool GwmGGWRTaskThread::setIsCV(bool iscv){
    isCV = iscv;
    return true;
}

bool GwmGGWRTaskThread::setMaxiter(int maxiter){
    mMaxiter = maxiter;
    return true;
}

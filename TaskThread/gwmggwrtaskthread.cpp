#include "gwmggwrtaskthread.h"
#include "GWmodel/GWmodel.h"
#include "gwmggwrbandwidthselectionthread.h"
#include "GWmodel/gwmgeneralizedlinearmodel.h"

#include <exception>

using namespace std;

QMap<QString, double> GwmGGWRTaskThread::TolUnitDict = {
    make_pair(QString("e -3"), 0.001),
    make_pair(QString("e -5"), 0.00001),
    make_pair(QString("e -7"), 0.0000001),
    make_pair(QString("e -10"), 0.0000000001)
};


GwmGGWRTaskThread::GwmGGWRTaskThread()
    :GwmGWRTaskThread()
{
    mWtMat1 = mat(uword(0), uword(0));
    mWtMat2 = mat(uword(0), uword(0));
    isCV = true;
//    mMaxiter = 20;
//    mTol = 1.0e-5;
//    mFamily = Family::Poisson;
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
    mBetas = mat(nVar, nRp, fill::zeros);
    if (hasHatMatrix)
    {
        mBetasSE = mat( nVar,nDp, fill::zeros);
    }

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
    int nVar = mX.n_cols;

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
            double trS = mSHat(0);
            double trStS = mSHat(1);

            mYHat = exp(gwFitted(mX,mBetas));
            mResidual = mY - mYHat;

            //计算诊断信息
            double AIC = gwDev + 2 * trS;
            double AICc = AIC + 2*trS*(trS+1)/(nDp-trS-1);
            double R2 = 1 - gwDev/(nulldev);  // pseudo.R2 <- 1 - gw.dev/null.dev
            vec vDiags(4);
//            int n = mX.n_rows;
//            double edf = n - 2 * mSHat(0) + mSHat(1);  //edf
//            double enp = 2 * mSHat(0) - mSHat(1);  // enp
//            double r2_adj = 1 - (1 - R2) * (n - 1) / (edf - 1);
            vDiags(0) = AIC;
            vDiags(1) = AICc;
            vDiags(2) = gwDev;
            vDiags(3) = R2;
//            vDiags(4) = gwDev;
//            vDiags(5) = R2;
//            vDiags(6) = r2_adj;
            mDiagnostic = GwmGGWRDiagnostic(vDiags);
//            diagnosticGGWR();

            // F Test

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
        if(isAllCorrect){
            mBetas = trans(mBetas);

            double trS = mSHat(0);
            double trStS = mSHat(1);

            mat yhat = gwFitted(mX,mBetas);
            mYHat = exp(yhat)/(1+exp(yhat));

            mResidual = mY - mYHat;
            vec Dev = log(1/( (mY-n+exp(yhat)/(1+exp(yhat))) % (mY-n+exp(yhat)/(1+exp(yhat))) ) );
            double gwDev = sum(Dev);
            vec residual2 = mResidual % mResidual;
            double rss = sum(residual2);
            for(int i = 0; i < nDp; i++){
                mBetasSE.col(i) = sqrt(mBetasSE.col(i));
    //            mBetasTV.col(i) = mBetas.col(i) / mBetasSE.col(i);
            }
            mBetasSE = trans(mBetasSE);
            mBetasTV = mBetas / mBetasSE;

            double AIC = gwDev + 2 * trS;
            double AICc = AIC + 2*trS*(trS+1)/(nDp-trS-1);
            double R2 = 1 - gwDev/(nulldev);  // pseudo.R2 <- 1 - gw.dev/null.dev
            vec vDiags(4);
//            int n = mX.n_rows;
//            double edf = n - 2 * mSHat(0) + mSHat(1);  //edf
//            double enp = 2 * mSHat(0) - mSHat(1);  // enp
//            double r2_adj = 1 - (1 - R2) * (n - 1) / (edf - 1);
            vDiags(0) = AIC;
            vDiags(1) = AICc;
            vDiags(2) = gwDev;
            vDiags(3) = R2;
//            vDiags(4) = gwDev;
//            vDiags(5) = R2;
//            vDiags(6) = r2_adj;
            mDiagnostic = GwmGGWRDiagnostic(vDiags);
//            diagnosticGGWR();


        }


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

void GwmGGWRTaskThread::diagnosticGGWR()
{
    emit message(tr("Calculating diagnostic informations..."));

    // 诊断信息
    double trS = mSHat(0), trStS = mSHat(1);
    double nDp = mFeatureList.size();
    double sigmaHat = mDiagnostic.RSS / (nDp - 2 * trS + trStS);
    mStudentizedResidual = mResidual / sqrt(sigmaHat * mQDiag);
    mBetasSE = sqrt(sigmaHat * mBetasSE);
    mBetasTV = mBetas / mBetasSE;
    vec dybar2 = (mY - sum(mY) / nDp) % (mY - sum(mY) / nDp);
    vec dyhat2 = (mY - mYHat) % (mY - mYHat);

    // Local RSquare
    mLocalRSquare = vec(mFeatureList.size(), fill::zeros);
    for (int i = 0; i < mFeatureList.size(); i++)
    {
        vec dist = distance(i,mRegPoints);
        mat w = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
        double tss = sum(dybar2 % w);
        double rss = sum(dyhat2 % w);
        mLocalRSquare(i) = (tss - rss) / tss;
    }
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

GwmGGWRDiagnostic GwmGGWRTaskThread::getDiagnostic() const
{
    return mDiagnostic;
}

GwmGLMDiagnostic GwmGGWRTaskThread::getGLMDiagnostic() const
{
    return mGLMDiagnostic;
}

bool GwmGGWRTaskThread::setFamily(Family family){
    mFamily = family;
    return true;
}

bool GwmGGWRTaskThread::setTol(double tol, QString unit){
    mTolUnit = unit;
    mTol = double(tol) * TolUnitDict[unit];
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


void GwmGGWRTaskThread::createResultLayer()
{
    emit message("Creating result layer...");
    QgsVectorLayer* srcLayer = mRegressionLayer ? mRegressionLayer : mLayer;
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    if (mBandwidthType == BandwidthType::Fixed)
    {
        layerName += QString("_B%1%2").arg(mBandwidthSizeOrigin, 0, 'f', 3).arg(mBandwidthSize);
    }
    else
    {
        layerName += QString("_B%1").arg(int(mBandwidthSize));
    }
    mResultLayer = new QgsVectorLayer(layerFileName, layerName, QStringLiteral("memory"));
    mResultLayer->setCrs(srcLayer->crs());

    QgsFields fields;
    fields.append(QgsField(QStringLiteral("Intercept"), QVariant::Double, QStringLiteral("double")));
    for (int index : mIndepVarsIndex)
    {
        QString srcName = mLayer->fields().field(index).name();
        QString name = srcName;
        fields.append(QgsField(name, QVariant::Double, QStringLiteral("double")));
    }
    if (hasHatMatrix)
    {
        fields.append(QgsField(QStringLiteral("y"), QVariant::Double, QStringLiteral("double")));
//        fields.append(QgsField(QStringLiteral("yhat"), QVariant::Double, QStringLiteral("double")));
        fields.append(QgsField(QStringLiteral("residual"), QVariant::Double, QStringLiteral("double")));
//        fields.append(QgsField(QStringLiteral("Stud_residual"), QVariant::Double, QStringLiteral("double")));
        fields.append(QgsField(QStringLiteral("Intercept_SE"), QVariant::Double, QStringLiteral("double")));
        for (int index : mIndepVarsIndex)
        {
            QString srcName = mLayer->fields().field(index).name();
            QString name = srcName + QStringLiteral("_SE");
            fields.append(QgsField(name, QVariant::Double, QStringLiteral("double")));
        }
        fields.append(QgsField(QStringLiteral("Intercept_TV"), QVariant::Double, QStringLiteral("double")));
        for (int index : mIndepVarsIndex)
        {
            QString srcName = mLayer->fields().field(index).name();
            QString name = srcName + QStringLiteral("_TV");
            fields.append(QgsField(name, QVariant::Double, QStringLiteral("double")));
        }
//        fields.append(QgsField(QStringLiteral("Local_R2"), QVariant::Double, QStringLiteral("double")));
    }
    mResultLayer->dataProvider()->addAttributes(fields.toList());
    mResultLayer->updateFields();

    mResultLayer->startEditing();
    if (hasHatMatrix)
    {
        int indepSize = mIndepVarsIndex.size() + 1;
        for (int f = 0; f < mFeatureList.size(); f++)
        {
            int curCol = 0;
            QgsFeature srcFeature = mFeatureList[f];
            QgsFeature feature(fields);
            feature.setGeometry(srcFeature.geometry());
            for (int a = 0; a < indepSize; a++)
            {
                int fieldIndex = a + curCol;
                QString attributeName = fields[fieldIndex].name();
                double attributeValue = mBetas(f, a);
                feature.setAttribute(attributeName, attributeValue);
            }
            curCol += indepSize;
            feature.setAttribute(fields[curCol++].name(), mY(f));
//            feature.setAttribute(fields[curCol++].name(), mYHat(f));
            feature.setAttribute(fields[curCol++].name(), mResidual(f));
//            feature.setAttribute(fields[curCol++].name(), mStudentizedResidual(f));
            for (int a = 0; a < indepSize; a++)
            {
                int fieldIndex = a + curCol;
                QString attributeName = fields[fieldIndex].name();
                double attributeValue = mBetasSE(f, a);
                feature.setAttribute(attributeName, attributeValue);
            }
            curCol += indepSize;
            for (int a = 0; a < indepSize; a++)
            {
                int fieldIndex = a + curCol;
                QString attributeName = fields[fieldIndex].name();
                double attributeValue = mBetasTV(f, a);
                feature.setAttribute(attributeName, attributeValue);
            }
            curCol += indepSize;
//            feature.setAttribute(fields[curCol++].name(), mLocalRSquare(f));
            mResultLayer->addFeature(feature);
        }
    }
    else
    {
        for (int f = 0; f < mFeatureList.size(); f++)
        {
            QgsFeature srcFeature = mFeatureList[f];
            QgsFeature feature(fields);
            feature.setGeometry(srcFeature.geometry());
            for (int a = 0; a < fields.size(); a++)
            {
                QString attributeName = fields[a].name();
                double attributeValue = mBetas(f, a);
                feature.setAttribute(attributeName, attributeValue);
            }
            mResultLayer->addFeature(feature);
        }
    }
    mResultLayer->commitChanges();
}

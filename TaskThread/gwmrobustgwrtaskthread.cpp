#include "gwmrobustgwrtaskthread.h"
#include "GWmodel/GWmodel.h"

GwmRobustGWRTaskThread::GwmRobustGWRTaskThread():GwmGWRTaskThread()
{

}

void GwmRobustGWRTaskThread::run()
{
    bool isAllCorrect;
    // 设置矩阵
    if (!setXY())
    {
        return;
    }
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    bool isStoreS = hasFTest && (nDp <= 8192);
    mS=mat(isStoreS ? nDp : 1, nDp, fill::zeros);
    if (filtered)
    {
        isAllCorrect = robustGWRCaliFirst();
    }
    else
    {
        isAllCorrect = robustGWRCaliSecond();
    }
    //产生结果图层
    if(isAllCorrect)
    {
        //诊断信息
        diagnostic();
        // F Test
        if (hasHatMatrix && hasFTest)
        {
            // Q
            double trQtQ = DBL_MAX;
            if (isStoreS)
            {
                mat EmS = eye(nDp, nDp) - mS;
                mat Q = trans(EmS) * EmS;
                trQtQ = sum(diagvec(trans(Q) * Q));
            }
            else
            {
                const CalcTrQtQ calc = calcTrQtQ[mParallelMethodType];
                trQtQ = (this->*calc)(mS);
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
        createResultLayer();
    }
    emit success();
}

bool GwmRobustGWRTaskThread::gwrModelCalibration()
{
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    emit message(tr("Calibrating GWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;
    if (hasHatMatrix)
    {
        bool isStoreS = hasFTest && (nDp <= 8192);

        // 解算
        mSHat.fill(fill::zeros);
        mQDiag.fill(fill::zeros);
        const RegressionAll regression = regressionAll[mParallelMethodType];
        isAllCorrect = (this->*regression)(hasHatMatrix, mS);
    }
    else
    {
        mat _(0, 0);
        isAllCorrect = regressionAllSerial(hasHatMatrix, _);
    }
    return isAllCorrect;
}

bool GwmRobustGWRTaskThread::robustGWRCaliFirst()
{
    gwrModelCalibration();
    //  ------------- 计算W.vect
    //计算Stud_residual
    mYHat = fitted(mX, mBetas);
    mResidual = mY - mYHat;
    // 诊断信息
    vec vDiags = gwrDiag(mY, mX, mBetas, mSHat);
    mDiagnostic = GwmGWRDiagnostic(vDiags);
    double trS = mSHat(0), trStS = mSHat(1);
    int nDp = mFeatureList.size();
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
    WVect.print();
    mWeightMask = WVect;
    bool res1 = gwrModelCalibration();
    return res1;
}

vec GwmRobustGWRTaskThread::filtWeight(vec x)
{
    double iter = 0;
    double diffmse = 1;
    //计算residual
    mYHat = fitted(mX, mBetas);
    mResidual = mY - mYHat;
    //计算mse
    mse = sum((mResidual % mResidual))/ mResidual.size();
    result.ones(x.size(),1);
    //数组赋值
    for(int i=0;i<x.size();i++)
    {
        if(x[i]<=2){
            result[i]=1;
        }else if(x[i]>2 && x[i]<3){
            result[i]=pow((1-(pow(x[i]-2,2))),2);
        }else{
            result[i]=0;
        }
    }
    return result;
}

bool GwmRobustGWRTaskThread::robustGWRCaliSecond()
{
    int nDp = mX.n_rows;
    double iter = 0;
    double diffmse = 1;
    double delta = 1.0e-5;
    double maxiter = 20;
    bool res2;
    gwrModelCalibration();
    //计算residual
    mX.print("X");
    mBetas.print("Betas");
    mYHat = fitted(mX, mBetas);
    mResidual = mY - mYHat;
    //计算mse
    mse = sum((mResidual % mResidual))/ mResidual.size();
    //计算WVect
    mWeightMask = filtWeight(abs(mResidual/sqrt(mse)));
    while(diffmse>delta && iter<maxiter){
        double oldmse = mse;
        res2 = gwrModelCalibration();
        //计算residual
        mYHat = fitted(mX, mBetas);
        mResidual = mY - mYHat;
        mse = sum((mResidual % mResidual))/ mResidual.size();
        mWeightMask = filtWeight(abs(mResidual/sqrt(mse)));
        diffmse = abs(oldmse-mse)/mse;
        iter = iter +1;
    }
    return res2;
}

void GwmRobustGWRTaskThread::setFiltered(bool value)
{
    if(value == true){
        this->filtered=true;
    }else{
        this->filtered=false;
    }
}

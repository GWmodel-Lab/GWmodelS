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
    if(filtered==true){
        isAllCorrect = robustGWRCaliFirst();
    }else if(filtered==false){
        isAllCorrect = robustGWRCaliSecond();
    }
    //产生结果图层
    if(isAllCorrect)
    {
        createResultLayer();
        //诊断信息
        diagnostic();
    }
}

bool GwmRobustGWRTaskThread::gwrModelCalibration(const vec &weightMask)
{
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    emit message(tr("Calibrating GWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;
    if (hasHatMatrix)
    {
        bool isStoreS = hasFTest && (nDp <= 8192);

        // 解算
        mat S(isStoreS ? nDp : 1, nDp, fill::zeros);
        const RegressionAll regression = regressionAll[mParallelMethodType];
        bool isAllCorrect = (this->*regression)(hasHatMatrix, weightMask, S);
        mBetas = trans(mBetas);
        mBetasSE = trans(mBetasSE);
    }
    else
    {
        mat _(0, 0);
        isAllCorrect = regressionAllSerial(hasHatMatrix, weightMask, _);
        mBetas = trans(mBetas);
    }
    return isAllCorrect;
}

bool GwmRobustGWRTaskThread::robustGWRCaliFirst()
{
    //  ------------- 计算W.vect
    vec WVect;
    //计算Stud_residual
    mYHat = fitted(mX, mBetas);
    mResidual = mY - mYHat;
    // 诊断信息
    vec vDiags = gwrDiag(mY, mX, mBetas, mSHat);
    mDiagnostic = GwmGWRDiagnostic(vDiags);
    double trS = mSHat(0), trStS = mSHat(1);
    double nDp = mFeatureList.size();
    double sigmaHat = mDiagnostic.RSS / (nDp - 2 * trS + trStS);
    mStudentizedResidual = mResidual / sqrt(sigmaHat * mQDiag);
    //生成W.vect
    for(int i=0;i<mStudentizedResidual.size();i++){
        if(fabs(mStudentizedResidual[i])>3){
            WVect[i]=0;
        }else{
            WVect[i]=1;
        }
    }
    bool res1 = gwrModelCalibration(WVect);
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
    double iter = 0;
    double diffmse = 1;
    double delta = 1.0e-5;
    double maxiter = 20;
    bool res2;
    //计算residual
    mYHat = fitted(mX, mBetas);
    mResidual = mY - mYHat;
    //计算mse
    mse = sum((mResidual % mResidual))/ mResidual.size();
    //计算WVect
    vec WVect;
    WVect = filtWeight(abs(mResidual/sqrt(mse)));
    while(diffmse>delta && iter<maxiter){
        double oldmse = mse;
        res2 = gwrModelCalibration(WVect);
        //计算residual
        mYHat = fitted(mX, mBetas);
        mResidual = mY - mYHat;
        mse = sum((mResidual % mResidual))/ mResidual.size();
        WVect = filtWeight(abs(mResidual/sqrt(mse)));
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

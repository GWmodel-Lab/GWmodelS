#include "gwmggwrbandwidthselectionthread.h"
#include "GWmodel/GWmodel.h"

GwmGGWRBandWidthSelectionThread::GwmGGWRBandWidthSelectionThread()
{
    mWt2 = vec(uword(0));
    mLLik = 0.0;
    myAdj = vec(uword(0));
    mMaxiter = 20;
    mTol = 0.00001;
    mFamily = GwmGGWRTaskThread::Family::Poisson;
}

GwmGGWRBandWidthSelectionThread::GwmGGWRBandWidthSelectionThread(const GwmGGWRTaskThread& gwrTaskThread)
    : GwmBandwidthSelectTaskThread(gwrTaskThread)
{
    GwmGWRTaskThread::hasHatMatrix = false;
    mWt2 = vec(uword(0));
    mLLik = 0.0;
    myAdj = vec(uword(0));
    createdFromGWRTaskThread = true;
    mMaxiter = gwrTaskThread.getMaxiter();
    mTol = gwrTaskThread.getTol();
    mFamily = gwrTaskThread.getFamily();
}


void GwmGGWRBandWidthSelectionThread::run(){
    //初始化BW score容器
    this->mBwScore.clear();
    emit tick(0, 0);
    //获得数据点
    if (!createdFromGWRTaskThread && !setXY())
    {
        return;
    }
    //计算lower\upper
    bool adaptive = mBandwidthType == BandwidthType::Adaptive;
    double upper = adaptive ? mX.n_rows : getFixedBwUpper();
    double lower = adaptive ? 20 : 0.0;
    //如果是cv,gold(gwr.cv....)
    //如果是Aic,gold(gwr.aic....)
    //默认是cv
    if (GwmGWRTaskThread::mBandwidthSelectionApproach == CV)
    {
        mBandwidthSize = gold(&GwmGGWRBandWidthSelectionThread::cvAll,lower,upper,adaptive,mX,mY,mDataPoints,mBandwidthKernelFunction,adaptive);
    }
    else
    {
        mBandwidthSize = gold(&GwmGGWRBandWidthSelectionThread::aicAll,lower,upper,adaptive,mX,mY,mDataPoints,mBandwidthKernelFunction,adaptive);
    }
}

double GwmGGWRBandWidthSelectionThread::cvAll(const mat& x, const vec& y,const mat& dp,double bw, int kernel, bool adaptive){
    int n = dp.n_rows;
    vec cv = vec(n);
    mat wt = mat(n,n);
    for (int i = 0; i < n; i++)
    {
        vec d = distance(i);
        vec w = gwWeight(d, bw, kernel, adaptive);
        w.row(i) = 0;
        wt.col(i) = w;
    }
    if(mFamily == GwmGGWRTaskThread::Family::Poisson){
        PoissonWt(x,y,bw,wt);
    }
    else{
        BinomialWt(x,y,bw,wt);
    }
    for (int i = 0; i < n; i++){
        mat wi = wt.col(i) % mWt2;
        vec gwsi = gwReg(x, myAdj, wi, i);
        mat yhatnoi = x.row(i) * gwsi;
        if(mFamily == GwmGGWRTaskThread::Family::Poisson){
            cv.row(i) = y.row(i) - exp(yhatnoi);
        }
        else{
            cv.row(i) = y.row(i) - exp(yhatnoi)/(1+exp(yhatnoi));
        }
    }
    vec cvsquare = trans(cv) * cv ;
    double res = sum(cvsquare);
    this->mBwScore.insert(bw,res);
    emit message(createOutputMessage(bw, res));
    return res;
}

mat GwmGGWRBandWidthSelectionThread::cvContrib(const mat& x, const vec& y,const mat& dp,double bw, int kernel, bool adaptive){
    int n = dp.n_rows;
    vec cv = vec(n);
    mat wt = mat(n,n);
    for (int i = 0; i < n; i++)
    {
        vec d = distance(i);
        vec w = gwWeight(d, bw, kernel, adaptive);
        w.row(i) = 0;
        wt.col(i) = w;
    }
    if(mFamily == GwmGGWRTaskThread::Family::Poisson){
        PoissonWt(x,y,bw,wt);
    }
    else{
        BinomialWt(x,y,bw,wt);
    }
    for (int i = 0; i < n; i++){
        mat wi = wt.col(i) % mWt2;
        vec gwsi = gwReg(x, myAdj, wi, i);
        mat yhatnoi = x.row(i) * gwsi;
        if(mFamily == GwmGGWRTaskThread::Family::Poisson){
            cv.row(i) = y.row(i) - exp(yhatnoi);
        }
        else{
            cv.row(i) = y.row(i) - exp(yhatnoi)/(1+exp(yhatnoi));
        }
    }
    return cv;
}

double GwmGGWRBandWidthSelectionThread::aicAll(const mat &x, const vec &y, const mat &dp, double bw, int kernel, bool adaptive){
    int n = dp.n_rows;
    vec cv = vec(n);
    mat wt = mat(n,n);
    mat S = mat(n,n);
    for (int i = 0; i < n; i++)
    {
        vec d = distance(i);
        vec w = gwWeight(d, bw, kernel, adaptive);
        wt.col(i) = w;
    }
    if(mFamily == GwmGGWRTaskThread::Family::Poisson){
        PoissonWt(x,y,bw,wt);
    }
    else{
        BinomialWt(x,y,bw,wt);
    }
    vec trS = vec(1,fill::zeros);
    for (int i = 0; i < n; i++){
        vec wi = wt.col(i) % mWt2;
        mat Ci = CiMat(x,wi);
        S.row(i) = x.row(i) * Ci;
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

    this->mBwScore.insert(bw,AICc);
    emit message(createOutputMessage(bw, AICc));
    return AICc;
}

void GwmGGWRBandWidthSelectionThread::PoissonWt(const mat &x, const vec &y, double bw, mat wt,bool verbose){
//    double tol = 1.0e-5;
//    int maxiter = 20;
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

    while(1){
        myAdj = nu + (y - mu)/mu;
        for (int i = 0; i < dpn; i++)
        {
            vec wi = wt.col(i);
            vec gwsi = gwReg(x, myAdj, wi % mWt2, i);
            betas.col(i) = gwsi;
        }
        mat betas1 = trans(betas);
        nu = gwFitted(x,betas1);
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
}


void GwmGGWRBandWidthSelectionThread::BinomialWt(const mat &x, const vec &y, double bw, mat wt,bool verbose){
//    double tol = 1.0e-5;
//    int maxiter = 20;
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
        nu = gwFitted(x,betas1);
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

//    return cv;
}

double GwmGGWRBandWidthSelectionThread::gold(pfApproach1 p,double xL, double xU, bool adaptBw,const mat& x, const vec& y, const mat& dp, int kernel, bool adaptive)
{
    const double eps = 1e-4;
    const double R = (sqrt(5)-1)/2;
    int iter = 0;
    double d = R * (xU - xL);
    double x1 = adaptBw ? floor(xL + d) : (xL + d);
    double x2 = adaptBw ? round(xU - d) : (xU - d);
    double f1 = (this->*p)(x, y, dp, x1, kernel, adaptive);
    double f2 = (this->*p)(x, y, dp, x2, kernel, adaptive);
    double d1 = (isinf(f1) && isinf(f2))? qInf() : f2 - f1;
    double xopt = f1 < f2 ? x1 : x2;
    double ea = 100;
    while ((fabs(d) > eps) && (fabs(d1) > eps) && iter < ea)
    {
        d = R * d;
        if (f1 < f2)
        {
            xL = x2;
            x2 = x1;
            x1 = adaptBw ? round(xL + d) : (xL + d);
            f2 = f1;
            f1 = (this->*p)(x, y, dp, x1, kernel, adaptive);
        }
        else
        {
            xU = x1;
            x1 = x2;
            x2 = adaptBw ? floor(xU - d) : (xU - d);
            f1 = f2;
            f2 = (this->*p)(x, y, dp, x2, kernel, adaptive);
        }
        iter = iter + 1;
        xopt = (f1 < f2) ? x1 : x2;
        d1 = (isinf(f1) && isinf(f2))? qInf() : f2 - f1;
    }
    return xopt;
}

#include "gwmbandwidthselecttaskthread.h"

#include "GWmodel/GWmodel.h"
#include <cmath>

//gwr.cv
//typedef double (GwmBandwidthSelect::*pf)(const mat& , const vec& , const mat& , double , int , bool );
//gwr.aic
//typedef double (*pf2)(mat , vec , mat , double , int , bool );

GwmBandwidthSelectTaskThread::GwmBandwidthSelectTaskThread()
    : GwmGWRTaskThread()
{
    GwmGWRTaskThread::hasHatMatrix = false;
}

GwmBandwidthSelectTaskThread::GwmBandwidthSelectTaskThread(const GwmGWRTaskThread& gwrTaskThread)
    : GwmGWRTaskThread(gwrTaskThread)
    , createdFromGWRTaskThread(true)
{
    GwmGWRTaskThread::hasHatMatrix = false;
}

void GwmBandwidthSelectTaskThread::run()
{
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
        mBandwidthSize = gold(&GwmBandwidthSelectTaskThread::cvAll,lower,upper,adaptive,mX,mY,mDataPoints,mBandwidthKernelFunction,adaptive);
    }
    else
    {
        mBandwidthSize = gold(&GwmBandwidthSelectTaskThread::aicAll,lower,upper,adaptive,mX,mY,mDataPoints,mBandwidthKernelFunction,adaptive);
    }
    qDebug() << "[GwmBandwidthSelectTaskThread::run]" << "bandwidth size" << mBandwidthSize;
}

double GwmBandwidthSelectTaskThread::cvAll(const mat& x, const vec& y, const mat& dp,double bw, int kernel, bool adaptive)
{
    int n = dp.n_rows;
    double cv = 0.0;
    for (int i = 0; i < n; i++)
    {
        vec d = distance(i);
        vec w = gwWeight(d, bw, kernel, adaptive);
        double res = gwCV(x, y, w, i);
        cv += res * res;
    }
    emit message(createOutputMessage(bw, cv));
    return cv;
}

double GwmBandwidthSelectTaskThread::aicAll(const mat& x, const vec& y, const mat& dp, double bw, int kernel, bool adaptive)
{
    int n = dp.n_rows, k = x.n_cols;
    mat betas(k, n, fill::zeros);
    vec s_hat(2, fill::zeros);
    mat ci, si;

    for (int i = 0; i < n;i++)
    {
        vec d = distance(i);
        vec w = gwWeight(d,bw,kernel,adaptive);
        betas.col(i) = gwRegHatmatrix(x, y, w, i, ci, si);
        s_hat(0) += si(0, i);
        s_hat(1) += det(si * trans(si));
    }
    betas = trans(betas);
    double score = AICc(y,x,betas,s_hat);
    emit message(createOutputMessage(bw, score));
    return score;
}

double GwmBandwidthSelectTaskThread::gold(pfApproach p,double xL, double xU, bool adaptBw,const mat& x, const vec& y, const mat& dp, int kernel, bool adaptive)
{
    const double eps = 1e-4;
    const double R = (sqrt(5)-1)/2;
    int iter = 0;
    double d = R * (xU - xL);
    double x1 = adaptBw ? floor(xL + d) : (xL + d);
    double x2 = adaptBw ? round(xU - d) : (xU - d);
    double f1 = (this->*p)(x, y, dp, x1, kernel, adaptive);
    double f2 = (this->*p)(x, y, dp, x2, kernel, adaptive);
    double d1 = f2 - f1;
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
        d1 = f2 - f1;
    }
    return xopt;
}

double GwmBandwidthSelectTaskThread::getFixedBwUpper()
{
    QgsRectangle extent = this->mLayer->extent();
    bool longlat = mLayer->crs().isGeographic();
    mat extentDp(2, 2, fill::zeros);
    extentDp(0, 0) = extent.xMinimum();
    extentDp(0, 1) = extent.yMinimum();
    extentDp(1, 0) = extent.xMaximum();
    extentDp(1, 1) = extent.yMaximum();
    vec dist = gwDist(extentDp, extentDp, 0, 2.0, 0.0, longlat, false);
    return dist(1);
}

QString GwmBandwidthSelectTaskThread::createOutputMessage(double bw, double score)
{
    QString m("%1: %2 (%3: %4)");
    if (mBandwidthType == BandwidthType::Adaptive)
    {
        return m.arg(tr("Adaptive bandwidth"))
                .arg((int)bw)
                .arg(GwmGWRTaskThread::mBandwidthSelectionApproach == BandwidthSelectionApproach::CV ? tr("CV score") : tr("AIC score"))
                .arg(score, 0, 'f', 6);
    }
    else
    {
        return m.arg(tr("Fixed bandwidth"))
                .arg(bw, 0, 'f', 3)
                .arg(GwmGWRTaskThread::mBandwidthSelectionApproach == BandwidthSelectionApproach::CV ? tr("CV score") : tr("AIC score"))
                .arg(score, 0, 'f', 6);
    }
}

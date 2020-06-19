#include "gwmgwpcataskthread.h"
#include <SpatialWeight/gwmcrsdistance.h>

GwmGWPCATaskThread::GwmGWPCATaskThread() : GwmSpatialMonoscaleAlgorithm()
{

}

void GwmGWPCATaskThread::run()
{
    // 设置矩阵
    initPoints();
    GwmCRSDistance d(mDataLayer->featureCount(), mDataLayer->crs().isGeographic());
    d.setDataPoints(&mDataPoints);
    d.setFocusPoints(&mDataPoints);
    //
    initXY(mX,mVariables);
    //选带宽
    //这里判断是否选带宽
    if(false)
    {
        GwmBandwidthWeight* bandwidthWeight0 = static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight());
        GwmBandwidthSizeSelector selector;
        selector.setBandwidth(bandwidthWeight0);
        double lower = bandwidthWeight0->adaptive() ? 20 : 0.0;
        double upper = bandwidthWeight0->adaptive() ? mDataPoints.n_rows : mSpatialWeight.distance()->maxDistance();
        selector.setLower(lower);
        selector.setUpper(upper);
        GwmBandwidthWeight* bandwidthWeight = selector.optimize(this);
        if(bandwidthWeight)
        {
            mSpatialWeight.setWeight(bandwidthWeight);
        }
    }
    //存储d的计算值
    mSpatialWeight.setDistance(d);
    mSpatialWeight.setWeight(GwmBandwidthWeight(100, true, GwmBandwidthWeight::Gaussian));;
    mat dResult(mDataPoints.n_rows, mVariables.size(),fill::zeros);
    //存储最新的wt
    vec latestWt(mDataPoints.n_rows,1,fill::zeros);
    //GWPCA算法
    for(int i=0;i<mDataPoints.n_rows;i++)
    {
        //vec distvi = mSpatialWeight.distance()->distance(i);
        vec wt = mSpatialWeight.spatialWeight(i);
        //取wt大于0的部分
        //临时变量?很麻烦
        int j=0;
        int length=0;
        for(int k=0;k<wt.n_rows;k++)
        {
            //判断有几项大于0
            if(wt(k)>0){
                length++;
            }
        }
        vec newWt(length,fill::zeros);
        mat newX(length,mX.n_cols,fill::zeros);
        for(int k=0;k<wt.n_rows;k++)
        {
            if(wt(k)>0){
                newWt(j) = wt(i);
                newX.row(j) = mX.row(i);
                j++;
            }
        }
        if(newWt.n_rows<=5)
        {
            break;
        }
        //调用PCA函数
        //事先准备好的D和V
        mat V;
        vec D;
        wpca(newX,newWt,0,mk,V,D);
        latestWt = newWt;
        dResult.row(i) = D;
    }
    //R代码中的d1计算
    mat dResult1(mDataPoints.n_rows, mVariables.size(),fill::zeros);
    dResult1 = (dResult / pow(sum(latestWt),0.5)) % (dResult / pow(sum(latestWt),0.5));
    //取dResult1的前K列
    mat localPV;
    localPV = dResult1.cols(0,mk) % (1 / sum(dResult1,1)) *100;
    localPV.print();
    //R代码 的 Local variance
    //t(apply(d1, 2, summary))
    const vec p = { 0.0, 0.25, 0.5, 0.75, 1.0 };
    for(int i=0;i<dResult1.n_cols;i++)
    {
        vec q = quantile(dResult1.col(i), p);
        qDebug() << "Comp";
        q.print();
    }
    //R代码 的 Local Proportion of Variance
    //t(apply(local.PV, 2, summary))
    for(int i=0;i<localPV.n_cols;i++)
    {
        vec q = quantile(localPV.col(i), p);
        qDebug() << "Comp";
        q.print();
    }
    //Cumulative summary(rowSums(local.PV))
    vec localPVSum = sum(localPV,2);
    vec Cumulative = quantile(localPVSum,p);
    Cumulative.print();
}

void GwmGWPCATaskThread::initPoints()
{
    int nDp = mDataLayer->featureCount();
    mDataPoints = mat(nDp, 2, fill::zeros);
    QgsFeatureIterator iterator = mDataLayer->getFeatures();
    QgsFeature f;
    for (int i = 0; iterator.nextFeature(f); i++)
    {
        QgsPointXY centroPoint = f.geometry().centroid().asPoint();
        mDataPoints(i, 0) = centroPoint.x();
        mDataPoints(i, 1) = centroPoint.y();
    }
}

void GwmGWPCATaskThread::initXY(mat &x, const QList<GwmVariable> &indepVars)
{
    int nDp = mDataLayer->featureCount(), nVar = indepVars.size();
    // Data layer and X,Y
    x = mat(nDp, nVar, fill::zeros);
    //y = vec(nDp, fill::zeros);
    QgsFeatureIterator iterator = mDataLayer->getFeatures();
    QgsFeature f;
    bool ok = false;
    for (int i = 0; iterator.nextFeature(f); i++)
    {
        //double vY = f.attribute(depVar.name).toDouble(&ok);
        for (int k = 0; k < indepVars.size(); k++)
        {
            double vX = f.attribute(indepVars[k].name).toDouble(&ok);
            if (ok) x(i, k) = vX;
            else emit error(tr("Independent variable value cannot convert to a number. Set to 0."));
        }
    }
}

void GwmGWPCATaskThread::wpca(const mat &x, const vec &wt, double nu, double nv, mat &V, vec &S)
{
    //首先完成中心化
    mat tmpLocalCenter(x.n_rows,x.n_cols,fill::zeros);
    for(int i=0;i<x.n_rows;i++)
    {
        tmpLocalCenter.row(i) = x.row(i) * wt(i);
    }
    //SVD
    mat U;
    svd(U,S,V,(x.each_row() - sum(tmpLocalCenter)/sum(wt)).each_col() % sqrt(wt));
    //S即为R中的d
    //V即为R中的v
}

mat GwmGWPCATaskThread::rwpca(const mat &x, const vec &wt, double nu=0, double nv=2)
{
    //计算mids
    mat mids = x;
    mids = mids.each_row() - x.row((abs(wt - 0.5)).index_min());
    //计算robustSvd的值
    mat coeff;
    mat score;
    vec latent;
    vec tsquared;
    princomp(coeff, score, latent, tsquared, mids.each_col() % wt);
    //?coeff是对的，差一个参数
    return coeff;
}

double GwmGWPCATaskThread::criterion(GwmBandwidthWeight *weight)
{
    int n = mX.n_rows;
    int m = mX.n_cols;
    double score = 0;
    //主循环开始
    //主循环
    for (int i = 0; i < n; i++)
    {
        //vec distvi = distance(i);
        vec wt = mSpatialWeight.spatialWeight(i);
        wt(i) = 0;
        //取wt大于0的部分
        //临时变量?很麻烦
        int j=0;
        int length=0;
        for(int k=0;k<wt.n_rows;k++)
        {
            //判断有几项大于0
            if(wt(k)>0){
                length++;
            }
        }
        vec newWt(length,fill::zeros);
        mat newX(length,mX.n_cols,fill::zeros);
        for(int k=0;k<wt.n_rows;k++)
        {
            if(wt(k)>0){
                newWt(j) = wt(i);
                newX.row(j) = mX.row(i);
                j++;
            }
        }
        //判断length(newWt)
        if(newWt.n_rows <=1)
        {
            break;
        }
        //调用PCA函数
        //事先准备好的S和V
        mat V;
        vec S;
        if(mRobust == false)
        {
            wpca(newX,newWt,0,mk,V,S);
        }else if(mRobust == true){
            V = rwpca(newX,newWt,0,mk);
        }
        V = V * trans(V);
        score = score + pow(sum(mX.row(i) - mX.row(i) * V),2);
    }
    return score;
}

double GwmGWPCATaskThread::gold(pfGwmGWPCABandwidthSelectionApproach p, double xL, double xU, bool adaptBw, const mat &x, double k, bool robust, int kernel, bool adaptive)
{
    const double eps = 1e-4;
    const double R = (sqrt(5)-1)/2;
    int iter = 0;
    double d = R * (xU - xL);
    double x1 = adaptBw ? floor(xL + d) : (xL + d);
    double x2 = adaptBw ? round(xU - d) : (xU - d);
    double f1 = (this->*p)(x1, x, k, robust, kernel, adaptive);
    double f2 = (this->*p)(x2, x, k, robust, kernel, adaptive);
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
            f1 = (this->*p)(x1, x, k, robust, kernel, adaptive);
        }
        else
        {
            xU = x1;
            x1 = x2;
            x2 = adaptBw ? floor(xU - d) : (xU - d);
            f1 = f2;
            f2 = (this->*p)(x2, x, k, robust, kernel, adaptive);
        }
        iter = iter + 1;
        xopt = (f1 < f2) ? x1 : x2;
        d1 = f2 - f1;
    }
    return xopt;
}

double GwmGWPCATaskThread::bwGWPCA(double k, bool robust, int kernel, bool adaptive)
{
    double upper, lower;
    //upper = adaptive ? mX.n_rows : findMaxDistance();
    lower = adaptive ? 20 : 0.0;
    double bw;
    //计算x？
    mat x;
    bw = gold(&GwmGWPCATaskThread::gwpcaCV,lower,upper,adaptive,x,k,robust,kernel,adaptive);
    //显示带宽和CV值
    return bw;
}

void GwmGWPCATaskThread::setVariables(const QList<GwmVariable> &variables)
{
    mVariables = variables;
}

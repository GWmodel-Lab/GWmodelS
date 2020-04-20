#include "gwmbandwidthselecttaskthread.h"

#include "GWmodel/GWmodel.h"
#include <cmath>

//gwr.cv
//typedef double (GwmBandwidthSelect::*pf)(const mat& , const vec& , const mat& , double , int , bool );
//gwr.aic
//typedef double (*pf2)(mat , vec , mat , double , int , bool );

GwmBandwidthSelectTaskThread::GwmBandwidthSelectTaskThread(QgsVectorLayer* layer, GwmLayerAttributeItem* depVar, QList<GwmLayerAttributeItem*> indepVars)
    : GwmGWRTaskThread()
    , mLayer(layer)
    , mDepVar(depVar)
    , mIndepVars(indepVars)
{
    QgsFeatureIterator it = mLayer->getFeatures();
    QgsFeature feature;
    while (it.nextFeature(feature))
    {
        mFeatureIds.append(feature.id());
    }

    // 计算所需的矩阵
    mX = mat(mFeatureIds.size(), indepVars.size() + 1);
    mY = vec(mFeatureIds.size());
    mBetas = mat(indepVars.size() + 1, mFeatureIds.size());
    mDataPoints = mat(mFeatureIds.size(), 2);

    // 获取自变量和因变量所属的属性列索引
    mDepVarIndex = mDepVar->attributeIndex();
    for (GwmLayerAttributeItem* item : mIndepVars)
    {
        int iIndepVar = item->attributeIndex();
        mIndepVarsIndex.append(iIndepVar);
    }
}

void GwmBandwidthSelectTaskThread::run()
{
    //获得数据点
    if (setXY())
    {
        return;
    }
    //计算lower\upper
    double lower,upper;
    bool adaptive = mBandwidthType == BandwidthType::Adaptive;
    if(adaptive)
    {
        upper = this->mX.n_rows;
        lower = this->mX.n_cols;
    }else{
        upper = sqrt(pow(this->mLayer->extent().width(),2)+pow(this->mLayer->extent().height(),2));
        lower = 0;
    }
    //如果是cv,gold(gwr.cv....)
    //如果是Aic,gold(gwr.aic....)
    //默认是cv
    if(mApproach == CV){
        mBandwidthSize = gold(&GwmBandwidthSelectTaskThread::gwCVAll,lower,upper,adaptive,mX,mY,mDataPoints,mBandwidthKernelFunction,adaptive);
    }else{
        mBandwidthSize = gold(&GwmBandwidthSelectTaskThread::AICRes,lower,upper,adaptive,mX,mY,mDataPoints,mBandwidthKernelFunction,adaptive);
    }
}

bool GwmBandwidthSelectTaskThread::isNumeric(QVariant::Type type)
{
    switch (type)
    {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::UInt:
    case QVariant::Double:
        return true;
    default:
        return false;
    }
}

bool GwmBandwidthSelectTaskThread::setXY()
{
    QgsField depField = mLayer->fields()[mDepVarIndex];
    if (!isNumeric(depField.type()))
    {
        emit error(tr("Dependent variable is not numeric."));
        return false;
    }
    for (int iIndepVar : mIndepVarsIndex)
    {
        QgsField indepField = mLayer->fields()[iIndepVar];
        if (!isNumeric(indepField.type()))
        {
            emit error(tr("Independent variable \"") + indepField.name() + tr("\" is not numeric."));
            return false;
        }
    }

    int row = 0;
    bool* ok = nullptr;
    for (QgsFeatureId featureId : mFeatureIds)
    {
        QgsFeature feature = mLayer->getFeature(featureId);
        double vY = feature.attribute(mDepVarIndex).toDouble(ok);
        if (ok)
        {
            mY.at(row, 0) = vY;
            delete ok;
            ok = nullptr;

            // 设置 X 矩阵
            for (int index : mIndepVarsIndex)
            {
                double vX = feature.attribute(index).toDouble(ok);
                if (ok)
                {
                    mX.at(row, index + 1) = vX;
                    delete ok;
                    ok = nullptr;
                }
                else
                {
                    emit error(tr("Independent variable value cannot convert to a number. Set to 0."));
                }
            }
            // 设置坐标
            QgsPointXY centroPoint = feature.geometry().centroid().asPoint();
            mDataPoints.at(row, 0) = centroPoint.x();
            mDataPoints.at(row, 1) = centroPoint.y();
        }
        else
        {
            emit error(tr("Dependent variable value cannot convert to a number. Set to 0."));
        }
    }
    // 坐标旋转
    if (mDistSrcType == DistanceSourceType::Minkowski)
    {
        QMap<QString, QVariant> parameters = mDistSrcParameters.toMap();
        double theta = parameters["theta"].toDouble();
        mDataPoints = coordinateRotate(mDataPoints, theta);
    }
    return true;
}

vec GwmBandwidthSelectTaskThread::distance(const QgsFeatureId &id)
{
    switch (mDistSrcType)
    {
    case DistanceSourceType::Minkowski:
        return distanceMinkowski(id);
    default:
        return distanceCRS(id);
    }
}

vec GwmBandwidthSelectTaskThread::distanceCRS(const QgsFeatureId &id)
{
    QgsFeature fSrc = mLayer->getFeature(id);
    QgsGeometry gSrc = fSrc.geometry();
    vec dist(mFeatureIds.size(), fill::zeros);
    for (int i = 0; i < mFeatureIds.size(); i++)
    {
        QgsFeature fDes = mLayer->getFeature(mFeatureIds[i]);
        dist.at(i) = fDes.geometry().distance(gSrc);
    }
    return dist;
}

vec GwmBandwidthSelectTaskThread::distanceMinkowski(const QgsFeatureId &id)
{
    QMap<QString, QVariant> parameters = mDistSrcParameters.toMap();
    double p = parameters["p"].toDouble();
    QgsFeature fSrc = mLayer->getFeature(id);
    QgsPointXY gSrc = fSrc.geometry().centroid().asPoint();
    vec srcLoc(gSrc.x(), gSrc.y());
    return mkDistVec(mDataPoints, srcLoc, p);
}

double GwmBandwidthSelectTaskThread::gwCVAll(const mat& x, const vec& y, const mat& dp,double bw, int kernel, bool adaptive)
{
    int n = dp.n_rows;
    double cv = 0.0;
    //int lgroup = floor(((double)n) / ngroup);
    int iStart = 0, iEnd = n;
    for (int i = iStart; i < iEnd; i++) {

      QgsFeatureId id = mFeatureIds[i];
      mat d = distance(id);

      mat w = gwWeight(d, bw, kernel, adaptive);
      w(i, 0) = 0.0;
      mat ws(1, x.n_cols, fill::ones);
      mat xtw = trans(x %(w * ws));
      mat xtwx = xtw * x;
      mat xtwy = trans(x) * (w % y);
      mat xtwx_inv = inv(xtwx);
      mat betas = xtwx_inv * xtwy;
      double res = y(i) - det(x.row(i) * betas);
      cv += res * res;
    }
    return cv;
}

double GwmBandwidthSelectTaskThread::AICc1(vec y, mat x, mat beta, vec s_hat)
{
  double ss = rss(y, x, beta);
  // vec s_hat = trhat2(S);
  int n = x.n_rows;
  double AICc = n * log(ss / n) + n * log(2 * datum::pi) + n * ((n + s_hat(0)) / (n - 2 - s_hat(0))); //AICc
  return AICc;
  //return 2*enp + 2*n*log(ss/n) + 2*enp*(enp+1)/(n - enp - 1);
}

double GwmBandwidthSelectTaskThread::AICRes(const mat& x, const vec& y, const mat& dp, double bw, int kernel, bool adaptive)
{
    int n = dp.n_rows, k = x.n_cols;
    mat Betas(n,k,fill::zeros);
    int iStart = 0;
    int iEnd = n;

    mat betasSE(n, k, fill::zeros);
    mat s_hat(1, 2, fill::zeros);
    mat qdiag(1, n, fill::zeros);
    mat rowsumSE(n, 1, fill::ones);

    for(int i=iStart;i<iEnd;i++)
    {
        QgsFeatureId id = mFeatureIds[i];
        mat d = distance(id);
        mat w = gwWeight(d,bw,kernel,adaptive);
        mat ws(1,k,fill::ones);
        mat xtw = trans(x %(w * ws));
        mat xtwx = xtw * x;
        mat xtwy = trans(x) * (w % y);
        mat xtwx_inv = inv(xtwx);
        Betas.row(i) = trans(xtwx_inv * xtwy);
        // hatmatrix
        mat ci = xtwx_inv * xtw;
        mat si = x.row(i) * ci;
        betasSE.row(i) = trans((ci % ci) * rowsumSE);
        s_hat(0) += si(0, i);
        s_hat(1) += det(si * trans(si));
        mat onei(1, n, fill::zeros);
        onei(i) = 1;
        mat p = onei - si;
        qdiag += p % p;
    }
    double AICcRes = AICc1(y,x,Betas,s_hat);
    return AICcRes;
}

double GwmBandwidthSelectTaskThread::gold(pfApproach p,double xL, double xU, bool adaptBw,const mat& x, const vec& y, const mat& dp, int kernel, bool adaptive)
{
    double eps = 1e-4;
    double R = (sqrt(5)-1)/2;
    double iter = 1;
    double d = R*(xU-xL);
    double x1,x2,f1,f2,d1,xopt;
    if(adaptBw)
    {
        x1 = floor(xL+d);
        x2 = round(xU-d);
    }
    else
    {
        x1 = xL+d;
        x2 = xU-d;
    }
    f1=(this->*p)(x,y,dp,x1,kernel,adaptive);
    f2=(this->*p)(x,y,dp,x2,kernel,adaptive);
    d1=f2-f1;
    if(f1<f2){
        xopt=x1;
    }else{
        xopt=x2;
    }
    double ea=100;
    while((fabs(d)>eps) && (fabs(d1)>eps)){
        d = R*d;
        if(f1<f2){
            xL = x2;
            x2 = x1;
            if(adaptBw){
                x1 = round(xL+d);
            }else{
                x1 = xL+d;
            }
            f2=f1;
            f1=(this->*p)(x,y,dp,x1,kernel,adaptive);
        }
        else{
            xU=x1;
            x1=x2;
            if(adaptBw){
                x2=floor(xU-d);
            }else{
                x2=xU-d;
            }
            f1=f2;
            f2=(this->*p)(x,y,dp,x2,kernel,adaptive);
        }
        iter = iter+1;
        if(f1<f2){
            xopt = x1;
        }else{
            xopt = x2;
        }
        d1 = f2-f1;
    }
    return xopt;
}

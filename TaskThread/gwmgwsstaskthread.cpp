#include "gwmgwsstaskthread.h"

GwmGWSSTaskThread::GwmGWSSTaskThread() : GwmSpatialMonoscaleAlgorithm()
{

}

bool GwmGWSSTaskThread::quantile() const
{
    return mQuantile;
}

void GwmGWSSTaskThread::setQuantile(bool quantile)
{
    mQuantile = quantile;
}

bool GwmGWSSTaskThread::isValid()
{
    if (mDataLayer == nullptr)
        return false;

    if (mVariables.size() < 1)
        return false;

    return true;
}

void GwmGWSSTaskThread::run()
{
    // 点位初始化
    initPoints();
    // 初始化
    initXY(mX, mVariables);

    bool adaptive = mBandwidth->adaptive();
    double upper = adaptive ? mDataPoints.n_rows : DBL_MAX;
    double lower = adaptive ? 20 : 0.0;
    int nVar = mX.n_cols;
    emit tick(0,nVar);
    for(int i = 0; i < nVar; i++){
        mBWS(0,i) = gold(GwmCVType::mean,lower,upper,mX.col(i));
        qDebug() << "Local Mean bw:" << mVariables[i].name << mBWS(0,i);
        emit message(QString("Local Mean: %1 (bw: %2)").arg(mVariables[i].name).arg(mBWS(0,i)));
        mBWS(1,i) = gold(GwmCVType::median,lower,upper,mX.col(i));
        qDebug() << "Local Median bw:" << mVariables[i].name << mBWS(1,i);
        emit message(QString("Local Median: %1 (bw: %2)").arg(mVariables[i].name).arg(mBWS(1,i)));
        emit tick(i,nVar);
    }
    createResultLayer();
    emit success();
}

double GwmGWSSTaskThread::findmedian(const mat &x, const mat &w)
{
    int lw = w.n_rows;
    vec xo = sort(x);
    vec wo = w.rows(sort_index(x));
    vec Cum = cumsum(wo);
    int cond = lw - 1;
    for(int i = 0; i < lw; i++){
        if(Cum(i) > 0.5){
            cond = i - 1;
            break;
        }
    }
    if(cond < 0)
    {
        cond = 0;
    }
    return xo[cond];
}

void GwmGWSSTaskThread::initPoints()
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

void GwmGWSSTaskThread::initXY(mat &x, const QList<GwmVariable> &indepVars)
{
    int nDp = mDataLayer->featureCount(), nVar = indepVars.size();
    // Data layer and X,Y
    x = mat(nDp, nVar, fill::zeros);
    QgsFeatureIterator iterator = mDataLayer->getFeatures();
    QgsFeature f;
    bool ok = false;
    for (int i = 0; iterator.nextFeature(f); i++)
    {
        for (int k = 0; k < indepVars.size(); k++)
        {
            double vX = f.attribute(indepVars[k].name).toDouble(&ok);
            if (ok) x(i, k ) = vX;
            else emit error(tr("variable value cannot convert to a number. Set to 0."));
        }
    }

    mBWS = mat(2,nVar,fill::zeros);
}

double GwmGWSSTaskThread::meanCV(const mat &x, GwmBandwidthWeight* bandwidthWeight)
{
    uword nDp = mDataPoints.n_rows, nVar = mVariables.size();
    vec shat(2, fill::zeros);
    double cv = 0.0;
    vec CVScore = vec(nDp);
    for (uword i = 0; i < nDp; i++)
    {
        vec d = mSpatialWeight.distance()->distance(mDataPoints.row(i), mDataPoints);
        vec w = bandwidthWeight->weight(d);
        double sumw = sum(w);
        vec Wi = w / sumw;
        double lmean = sum(Wi % x);
        w.row(i) = 0;
        w = w / sum(w);
        double lmeanresi = sum(w % x);
        double res = lmean - lmeanresi;
        cv += res * res;
    }
    qDebug() << "Local Mean bw:" << bandwidthWeight->bandwidth()  << "CV value" << cv;
    emit message(QString("Local Mean bw: %1 (CV value: %2)").arg(bandwidthWeight->bandwidth()).arg(cv));
    return cv;
}

double GwmGWSSTaskThread::medianCV(const mat &x, GwmBandwidthWeight *bandwidthWeight)
{
    uword nDp = mDataPoints.n_rows, nVar = mVariables.size();
    vec shat(2, fill::zeros);
    double cv = 0.0;
    for (uword i = 0; i < nDp; i++)
    {
        vec d = mSpatialWeight.distance()->distance(mDataPoints.row(i), mDataPoints);
        vec w = bandwidthWeight->weight(d);
        double sumw = sum(w);
        vec Wi = w / sumw;
        double lmedian = findmedian(x,Wi);
        Wi = del(Wi,i);
        double test = sum(Wi);
        Wi = Wi / sum(Wi);
        double lmedianresi = findmedian(del(x,i),Wi);
        double res = lmedian - lmedianresi;
        if(1){
            double k = 0;
        }
        cv += res * res;
    }
    qDebug() << "Local Median bw:" << bandwidthWeight->bandwidth() << "CV value" << cv;
    emit message(QString("Local Median bw: %1 (CV value: %2)").arg(bandwidthWeight->bandwidth()).arg(cv));
    return cv;
}

double GwmGWSSTaskThread::gold(GwmCVType cvType, double xL, double xU, const mat& x)
{
    switch (cvType)
    {
    case GwmCVType::mean:
        mCVFunction = &GwmGWSSTaskThread::meanCV;
        break;
    case GwmCVType::median:
        mCVFunction = &GwmGWSSTaskThread::medianCV;
        break;
    default:
        mCVFunction = &GwmGWSSTaskThread::meanCV;
        break;
    }
    GwmBandwidthWeight* w1 = new GwmBandwidthWeight(*mBandwidth);
    GwmBandwidthWeight* w2 = new GwmBandwidthWeight(*mBandwidth);
    const double eps = 1e-4;
    const double R = (sqrt(5)-1)/2;
    int iter = 0;
    double d = R * (xU - xL);
    bool adaptBw = mBandwidth->adaptive();
    double x1 = adaptBw ? floor(xL + d) : (xL + d);
    double x2 = adaptBw ? round(xU - d) : (xU - d);
    w1->setBandwidth(x1);
    w2->setBandwidth(x2);
    double f1 = (this->*mCVFunction)(x, w1);
    double f2 = (this->*mCVFunction)(x, w2);
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
            w1->setBandwidth(x1);
            f1 = (this->*mCVFunction)(x, w1);
        }
        else
        {
            xU = x1;
            x1 = x2;
            x2 = adaptBw ? floor(xU - d) : (xU - d);
            f1 = f2;
            w2->setBandwidth(x2);
            f2 = (this->*mCVFunction)(x, w2);
        }
        iter = iter + 1;
        xopt = (f1 < f2) ? x1 : x2;
        d1 = f2 - f1;
    }
    return xopt;
}

void GwmGWSSTaskThread::createResultLayer()
{
    QgsVectorLayer* srcLayer = mDataLayer;

    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    layerName += QStringLiteral("_GWSS");

    mResultLayer = new QgsVectorLayer(layerFileName, layerName, QStringLiteral("memory"));
    mResultLayer->setCrs(srcLayer->crs());

    QgsFields fields;

    // 设置要素几何
    mResultLayer->startEditing();
    QgsFeatureIterator iterator = srcLayer->getFeatures();
    QgsFeature f;
    for (int i = 0; iterator.nextFeature(f); i++)
    {
        QgsFeature feature(fields);
        feature.setGeometry(f.geometry());


        mResultLayer->addFeature(feature);
    }
    mResultLayer->commitChanges();

}

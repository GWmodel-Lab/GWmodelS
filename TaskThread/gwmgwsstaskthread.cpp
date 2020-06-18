#include "gwmgwsstaskthread.h"
#include "SpatialWeight/gwmcrsdistance.h"

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
    int nDp = mX.n_rows,nRp = hasRegressionLayer() ? mRegressionPoints.n_rows : mDataPoints.n_rows;
    emit tick(0,nRp);
    for(int i = 0; i < nRp; i++){
        vec d = mSpatialWeight.distance()->distance(i);
        vec w = mBandwidth->weight(d);
        double sumw = sum(w);
        mat Wi = w / sumw;
        mLocalMean.row(i) = trans(Wi) * mX;
        if(mQuantile){
//            mat cor = cor(mX);
            mat quant = mat(3,nVar);
            for(int j = 0; j < nVar; j++){
                quant.col(j) = findq(mX.col(j), Wi);
            }
            mLocalMedian.row(i) = quant.row(1);
            mIQR.row(i) = quant.row(2) - quant.row(0);
            mQI.row(i) = (2 * quant.row(1) - quant.row(2) - quant.row(0))/mIQR.row(i);
        }
        for(int j = 0; j < nVar; j++){
            vec lvar = trans(Wi) * (square(mX.col(j) - mLocalMean(i,j)));
            mLVar(i,j) = lvar(0);
            mStandardDev(i,j) = sqrt(mLVar(i,j));
            vec localSkewness = (trans(Wi) * pow(mX.col(j) - mLocalMean(i,j),3)) / pow(mStandardDev(i,j),3);
            mLocalSkewness(i,j) = localSkewness(0);
            mLCV(i,j) = mStandardDev(i,j)/mLocalMean(i,j);
        }
        if(nVar >= 2){
            int tag = 0;
            for(int j = 0; j < nVar-1; j++){
                for(int k = 0; k < nVar; k++){
                    tag++;
                    mCovmat(i,tag) = covwt(mX.col(j),mX.col(k),Wi);
                    mCorrmat(i,tag) = corwt(mX.col(j),mX.col(k),Wi);
                    mSCorrmat(i,tag) = corwt(rank(mX.col(j)),rank(mX.col(k)),Wi);
                }
            }
        }
        emit tick(i,nRp);
    }
    CreateResultLayerData resultLayerData;
    resultLayerData.push_back(qMakePair(QString("Local means"), mLocalMean));
    resultLayerData.push_back(qMakePair(QString("standard deviation"), mStandardDev));
    resultLayerData.push_back(qMakePair(QString("local variance"), mLVar));
    resultLayerData.push_back(qMakePair(QString("Local skewness"), mLocalSkewness));
    resultLayerData.push_back(qMakePair(QString("localized coefficient of variation"), mLCV));
    if(mQuantile){
        resultLayerData.push_back(qMakePair(QString("Local median"), mLocalMedian));
        resultLayerData.push_back(qMakePair(QString("Interquartile range"), mIQR));
        resultLayerData.push_back(qMakePair(QString("Quantile imbalance"), mQI));
    }
    if(nVar >= 2){
        resultLayerData.push_back(qMakePair(QString("Cov"), trans(mCovmat)));
        resultLayerData.push_back(qMakePair(QString("Corr"), trans(mCorrmat)));
    }
    createResultLayer(resultLayerData);
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

mat GwmGWSSTaskThread::findq(const mat &x, const mat &w)
{
    int lw = w.n_rows;
    int lp = 3;
    vec q = vec(lp,fill::zeros);
    vec xo = sort(x);
    vec wo = w(sort_index(x));
    vec Cum = cumsum(wo);
    int cond = lw - 1;
    for(int j = 0; j < lp ; j++){
        double k = 0.25 * (j + 1);
        for(int i = 0; i < lw; i++){
            if(Cum(i) > k){
                cond = i - 1;
                break;
            }
        }
        if(cond < 0)
        {
            cond = 0;
        }
        q.row(j) = xo[cond];
        cond = lw - 1;
    }
    return q;
}

double GwmGWSSTaskThread::covwt(mat x1, mat x2, mat w){
    vec wi = w/sum(w);
    double center1 = sum(x1 % wi);
    double center2 = sum(x2 % wi);
    vec n1 = sqrt(wi) % (x1 - center1);
    vec n2 = sqrt(wi) % (x2 - center2);
    double res = sum(n1 % n2) / (1 - sum(square(wi)));
    return res;
}

double GwmGWSSTaskThread::corwt(mat x1, mat x2, mat w)
{
    return covwt(x1,x2,w)/sqrt(covwt(x1,x1,w)*covwt(x2,x2,w));
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
    // Regression Layer
    if (hasRegressionLayer())
    {
        int nRp = mRegressionLayer->featureCount();
        mRegressionPoints = mat(nRp, 2, fill::zeros);
        QgsFeatureIterator iterator = mRegressionLayer->getFeatures();
        QgsFeature f;
        for (int i = 0; i < nRp && iterator.nextFeature(f); i++)
        {
            QgsPointXY centroPoint = f.geometry().centroid().asPoint();
            mRegressionPoints(i, 0) = centroPoint.x();
            mRegressionPoints(i, 1) = centroPoint.y();
        }
    }
    else mRegressionPoints = mDataPoints;

    if (mSpatialWeight.distance()->type() == GwmDistance::CRSDistance || mSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance)
    {
        GwmCRSDistance* d = static_cast<GwmCRSDistance*>(mSpatialWeight.distance());
        d->setDataPoints(&mDataPoints);
        d->setFocusPoints(&mDataPoints);
    }
}

void GwmGWSSTaskThread::initXY(mat &x, const QList<GwmVariable> &indepVars)
{
    int nDp = mDataLayer->featureCount(), nVar = indepVars.size();
    int nRp = hasRegressionLayer() ? mRegressionPoints.n_rows : mDataPoints.n_rows;
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
    mLocalMean = mat(nRp,nVar,fill::zeros);
    mStandardDev = mat(nRp,nVar,fill::zeros);
    mLocalSkewness = mat(nRp,nVar,fill::zeros);
    mLCV = mat(nRp,nVar,fill::zeros);
    mLVar = mat(nRp,nVar,fill::zeros);
    if(mQuantile){
        mLocalMedian = mat(nRp,nVar,fill::zeros);
        mIQR = mat(nRp,nVar,fill::zeros);
        mQI = mat(nRp,nVar,fill::zeros);
    }

    if(nVar > 1){
        mCovmat = mat((nVar-1)*nVar/2,nRp,fill::zeros);
        mCorrmat = mat((nVar-1)*nVar/2,nRp,fill::zeros);
        mSCorrnms = mat((nVar-1)*nVar/2,nRp,fill::zeros);
        mSCorrmat = mat((nVar-1)*nVar/2,nRp,fill::zeros);
    }
}

double GwmGWSSTaskThread::meanCV(const mat &x, GwmBandwidthWeight* bandwidthWeight)
{
    uword nDp = mDataPoints.n_rows, nVar = mVariables.size();
    vec shat(2, fill::zeros);
    double cv = 0.0;
    vec CVScore = vec(nDp);
    for (uword i = 0; i < nDp; i++)
    {
        vec d = mSpatialWeight.distance()->distance(i);
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
    emit message(QString("Local MeanA bw: %1 (CV value: %2)").arg(bandwidthWeight->bandwidth()).arg(cv));
    return cv;
}

double GwmGWSSTaskThread::medianCV(const mat &x, GwmBandwidthWeight *bandwidthWeight)
{
    uword nDp = mDataPoints.n_rows, nVar = mVariables.size();
    vec shat(2, fill::zeros);
    double cv = 0.0;
    for (uword i = 0; i < nDp; i++)
    {
        vec d = mSpatialWeight.distance()->distance(i);
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

void GwmGWSSTaskThread::createResultLayer(CreateResultLayerData data)
{
    QgsVectorLayer* srcLayer =  mRegressionLayer ? mRegressionLayer : mDataLayer;
    int nVar = mVariables.size();
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    layerName += QStringLiteral("_GWSS");

    mResultLayer = new QgsVectorLayer(layerFileName, layerName, QStringLiteral("memory"));
    mResultLayer->setCrs(srcLayer->crs());

    // 设置字段
    QgsFields fields;
    for (QPair<QString, const mat&> item : data)
    {
        QString title = item.first;
        const mat& value = item.second;
        if (value.n_cols > nVar)
        {
            for(uword j = 0; j < nVar-1; j++){
                for (uword k = 0; k < nVar; k++)
                {
                    QString variableName = mVariables[k].name + "*" + mVariables[k].name + "_" + title;
//                    QString fieldName = title.arg(variableName);
                    fields.append(QgsField(variableName, QVariant::Double, QStringLiteral("double")));
                }
            }
        }
        else
        {
            for (uword k = 0; k < value.n_cols; k++)
            {
                QString variableName = mVariables[k].name + "_" + title;
//                QString fieldName = title.arg(variableName);
                fields.append(QgsField(variableName, QVariant::Double, QStringLiteral("double")));
            }
        }
    }
    mResultLayer->dataProvider()->addAttributes(fields.toList());
    mResultLayer->updateFields();


    // 设置要素几何
    mResultLayer->startEditing();
    QgsFeatureIterator iterator = srcLayer->getFeatures();
    QgsFeature f;
    for (int i = 0; iterator.nextFeature(f); i++)
    {
        QgsFeature feature(fields);
        feature.setGeometry(f.geometry());
        // 设置属性
        int k = 0;
        for (QPair<QString, const mat&> item : data)
        {
            for (uword d = 0; d < item.second.n_cols; d++)
            {
                feature.setAttribute(k, item.second(i, d));
                k++;
            }
        }

        mResultLayer->addFeature(feature);
    }
    mResultLayer->commitChanges();

}

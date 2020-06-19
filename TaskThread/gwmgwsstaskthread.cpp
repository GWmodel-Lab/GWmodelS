#include "gwmgwsstaskthread.h"
#include "gwmgwsstaskthread.h"
#include "SpatialWeight/gwmcrsdistance.h"

vec GwmGWSSTaskThread::del(vec x, int rowcount){
    vec res;
    if(rowcount == 0)
        res = x.rows(rowcount+1,x.n_rows-1);
    else if(rowcount == x.n_rows-1)
        res = x.rows(0,x.n_rows-2);
    else
        res = join_cols(x.rows(0,rowcount - 1),x.rows(rowcount+1,x.n_rows-1));
    return res;
}

GwmGWSSTaskThread::GwmGWSSTaskThread() : GwmSpatialMonoscaleAlgorithm()
{

}

bool GwmGWSSTaskThread::isValid()
{
    if (mDataLayer == nullptr)
        return false;

    if (mVariables.size() < 1)
        return false;

    if (static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight())->bandwidth() == 0)
        return false;

    return true;
}

void GwmGWSSTaskThread::run()
{
    // 点位初始化
    initPoints();
    // 初始化
    initXY(mX, mVariables);

    bool adaptive = static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight())->adaptive();
    double upper = adaptive ? mDataPoints.n_rows : DBL_MAX;
    double lower = adaptive ? 20 : 0.0;
    int nVar = mX.n_cols;
    int nDp = mX.n_rows,nRp = mDataPoints.n_rows;
    emit tick(0,nRp);
    for(int i = 0; i < nRp; i++){
//        vec d = mSpatialWeight.distance()->distance(i);
        vec w = mSpatialWeight.spatialWeight(i);
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
        resultLayerData.push_back(qMakePair(QString("Corr"), trans(mSCorrmat)));

    }
    mResultList = resultLayerData;
    createResultLayer(resultLayerData);
    emit success();
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
    int nRp = mDataPoints.n_rows;
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



void GwmGWSSTaskThread::createResultLayer(CreateResultLayerData data)
{
    QgsVectorLayer* srcLayer =  mDataLayer;
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
                    QString variableName = mVariables[j].name + "*" + mVariables[k].name + "_" + title;
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

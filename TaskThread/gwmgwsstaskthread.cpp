#include "gwmgwsstaskthread.h"
#include "gwmgwsstaskthread.h"
#include "SpatialWeight/gwmcrsdistance.h"
#ifdef ENABLE_OpenMP
#include <omp.h>
#endif

int GwmGWSSTaskThread::treeChildCount = 0;

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
    if(!checkCanceled())
    {
        // 点位初始化
        initPoints();
        // 初始化
        initXY(mX, mVariables);
    }
    int nVar = mX.n_cols, nRp = mDataPoints.n_rows;
    (this->*mCalFunciton)();
    CreateResultLayerData resultLayerData;
    if(!checkCanceled())
    {
        resultLayerData.push_back(qMakePair(QString("LM"), mLocalMean));
        resultLayerData.push_back(qMakePair(QString("LSD"), mStandardDev));
        resultLayerData.push_back(qMakePair(QString("LVar"), mLVar));
        resultLayerData.push_back(qMakePair(QString("LSke"), mLocalSkewness));
        resultLayerData.push_back(qMakePair(QString("LCV"), mLCV));
    }
    if(mQuantile && !checkCanceled()){
        resultLayerData.push_back(qMakePair(QString("Median"), mLocalMedian));
        resultLayerData.push_back(qMakePair(QString("IQR"), mIQR));
        resultLayerData.push_back(qMakePair(QString("QI"), mQI));
    }
    if(nVar >= 2 && !checkCanceled()){
        resultLayerData.push_back(qMakePair(QString("Cov"), mCovmat));
        resultLayerData.push_back(qMakePair(QString("Corr"), mCorrmat));
        resultLayerData.push_back(qMakePair(QString("Spearman_rho"), mSCorrmat));
    }
    if(!checkCanceled())
    {
        mResultList = resultLayerData;
        createResultLayer(resultLayerData);
        emit success();
        emit tick(100, 100);
    }
    if(checkCanceled()) return;
}

bool GwmGWSSTaskThread::CalculateSerial(){
    mat rankX = mX;
    rankX.each_col([&](vec x) { x = rank(x); });
    int nVar = mX.n_cols, nRp = mDataPoints.n_rows;
    emit tick(0,nRp);
    for(int i = 0; i < nRp && !checkCanceled(); i++){
//        vec d = mSpatialWeight.distance()->distance(i);
        vec w = mSpatialWeight.weightVector(i);
        double sumw = sum(w);
        vec Wi = w / sumw;
        mLocalMean.row(i) = trans(Wi) * mX;
        if(mQuantile && !checkCanceled()){
//            mat cor = cor(mX);
            mat quant = mat(3,nVar);
            for(int j = 0; j < nVar && !checkCanceled(); j++){
                quant.col(j) = findq(mX.col(j), Wi);
            }
            mLocalMedian.row(i) = quant.row(1);
            mIQR.row(i) = quant.row(2) - quant.row(0);
            mQI.row(i) = (2 * quant.row(1) - quant.row(2) - quant.row(0))/mIQR.row(i);
        }
        mat centerized = mX.each_row() - mLocalMean.row(i);
        mLVar.row(i) = Wi.t() * (centerized % centerized);
        mStandardDev.row(i) = sqrt(mLVar.row(i));
        mLocalSkewness.row(i) = (Wi.t() * (centerized % centerized % centerized)) / (mLVar.row(i) % mStandardDev.row(i));
        if(nVar >= 2 && !checkCanceled()){
            int tag = 0;
            for(int j = 0; j < nVar-1 && !checkCanceled(); j++){
                for(int k = j+1; k < nVar && !checkCanceled(); k++){
                    double covjk = covwt(mX.col(j), mX.col(k), Wi);
                    double sumW2 = sum(Wi % Wi);
                    double covjj = mLVar(i, j) / (1.0 - sumW2);
                    double covkk = mLVar(i, k) / (1.0 - sumW2);
                    mCovmat(i,tag) = covjk;
                    mCorrmat(i,tag) = covjk / sqrt(covjj * covkk);
                    mSCorrmat(i,tag) = corwt(rankX.col(j),rankX.col(k),Wi);
                    tag++;
                }
            }
        }
        emit tick(i,nRp);
    }
    mLCV = mStandardDev / mLocalMean;
    return true;
}
#ifdef ENABLE_OpenMP
bool GwmGWSSTaskThread::CalculateOmp(){
    mat rankX = mX;
    rankX.each_col([&](vec x) { x = rank(x); });
    int nVar = mX.n_cols, nRp = mDataPoints.n_rows;
    emit tick(0,nRp);
    int current = 0;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for(int i = 0; i < nRp; i++){
//        vec d = mSpatialWeight.distance()->distance(i);
        if(!checkCanceled())
        {
            vec w = mSpatialWeight.weightVector(i);
            double sumw = sum(w);
            vec Wi = w / sumw;
            mLocalMean.row(i) = Wi.t() * mX;
            if(mQuantile && !checkCanceled()){
    //            mat cor = cor(mX);
                mat quant = mat(3,nVar);
                for(int j = 0; j < nVar && !checkCanceled(); j++){
                    quant.col(j) = findq(mX.col(j), Wi);
                }
                mLocalMedian.row(i) = quant.row(1);
                mIQR.row(i) = quant.row(2) - quant.row(0);
                mQI.row(i) = (2 * quant.row(1) - quant.row(2) - quant.row(0))/mIQR.row(i);
            }
            mat centerized = mX.each_row() - mLocalMean.row(i);
            mLVar.row(i) = Wi.t() * (centerized % centerized);
            mStandardDev.row(i) = sqrt(mLVar.row(i));
            mLocalSkewness.row(i) = (Wi.t() * (centerized % centerized % centerized)) / (mLVar.row(i) % mStandardDev.row(i));
            if(nVar >= 2 && !checkCanceled()){
                int tag = 0;
                for(int j = 0; j < nVar-1 && !checkCanceled(); j++){
                    for(int k = j+1; k < nVar && !checkCanceled(); k++){
                        double covjk = covwt(mX.col(j), mX.col(k), Wi);
                        double sumW2 = sum(Wi % Wi);
                        double covjj = mLVar(i, j) / (1.0 - sumW2);
                        double covkk = mLVar(i, k) / (1.0 - sumW2);
                        mCovmat(i,tag) = covjk;
                        mCorrmat(i,tag) = covjk / sqrt(covjj * covkk);
                        mSCorrmat(i,tag) = corwt(rankX.col(j),rankX.col(k),Wi);
                        tag++;
                    }
                }
            }
            emit tick(current++,nRp);
        }
    }
    mLCV = mStandardDev / mLocalMean;
    return true;
}
#endif
vec GwmGWSSTaskThread::findq(const mat &x, const vec &w)
{
    int lw = w.n_rows;
    int lp = 3;
    vec q = vec(lp,fill::zeros);
    vec xo = sort(x);
    vec wo = w(sort_index(x));
    vec Cum = cumsum(wo);
    int cond = lw - 1;
    for(int j = 0; j < lp; j++){
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
        int nCol = (nVar+1)*nVar/2 - nVar;
        mCovmat   = mat(nRp, nCol, fill::zeros);
        mCorrmat  = mat(nRp, nCol, fill::zeros);
        mSCorrmat = mat(nRp, nCol, fill::zeros);
    }
}



void GwmGWSSTaskThread::createResultLayer(CreateResultLayerData data)
{
    QgsVectorLayer* srcLayer =  mDataLayer;
    int nVar = mVariables.size();
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    //避免图层名重复
    if(treeChildCount > 0)
    {
        layerName += QStringLiteral("_GWSS") + "(" + QString::number(treeChildCount) + ")";
    } else
    {
        layerName += QStringLiteral("_GWSS");
    }
    //节点记录标签
    treeChildCount++ ;

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
            for(uword j = 0; j < nVar-1; j++)
            {
                for (uword k = j+1; k < nVar; k++)
                {
                    QString variableName = title + "_" + mVariables[j].name + "." + mVariables[k].name;
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

void GwmGWSSTaskThread::setParallelType(const IParallelalbe::ParallelType &type)
{
    if (type & parallelAbility())
    {
        mParallelType = type;
        switch (type) {
        case IParallelalbe::ParallelType::SerialOnly:
//            mRegressionFunction = &GwmBasicGWRAlgorithm::regressionSerial;
            mCalFunciton = &GwmGWSSTaskThread::CalculateSerial;
            break;
#ifdef ENABLE_OpenMP
        case IParallelalbe::ParallelType::OpenMP:
            mCalFunciton = &GwmGWSSTaskThread::CalculateOmp;
            break;
#endif
        default:
            mCalFunciton = &GwmGWSSTaskThread::CalculateSerial;
            break;
        }
    }
}

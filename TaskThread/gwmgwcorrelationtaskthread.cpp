#include "gwmgwcorrelationtaskthread.h"
#include "gwmgwcorrelationtaskthread.h"
#include "SpatialWeight/gwmcrsdistance.h"
#ifdef ENABLE_OpenMP
#include <omp.h>
#endif

int GwmGWcorrelationTaskThread::treeChildCount = 0;

GwmEnumValueNameMapper<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> GwmGWcorrelationTaskThread::BandwidthSelectionCriterionTypeNameMapper = {
    std::make_pair(GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType::CV, tr("CV")),
    std::make_pair(GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType::AIC, tr("AIC"))
};

vec GwmGWcorrelationTaskThread::del(vec x, int rowcount){
    vec res;
    if(rowcount == 0)
        res = x.rows(rowcount+1,x.n_rows-1);
    else if(rowcount == x.n_rows-1)
        res = x.rows(0,x.n_rows-2);
    else
        res = join_cols(x.rows(0,rowcount - 1),x.rows(rowcount+1,x.n_rows-1));
    return res;
}

GwmGWcorrelationTaskThread::GwmGWcorrelationTaskThread() : GwmSpatialMultiscaleAlgorithm()
{

}

bool GwmGWcorrelationTaskThread::isValid()
{
    if (mDataLayer == nullptr)
        return false;

    if (mVariables.size() < 1)
        return false;

    int nVar = mVariables.size() * mVariablesY.size();

    if (mSpatialWeights.size() != nVar)
        return false;

    for (int i = 0; i < nVar; i++)
    {
        GwmBandwidthWeight* bw = mSpatialWeights[i].weight<GwmBandwidthWeight>();
        if (mBandwidthInitilize[i] == GwmMultiscaleGWRAlgorithm::Specified)
        {
            if (bw->adaptive())
            {
                if (bw->bandwidth() <= 1)
                    return false;
            }
            else
            {
                if (bw->bandwidth() < 0.0)
                    return false;
            }
        }
    }

    return true;
}

void GwmGWcorrelationTaskThread::run()
{
    if(!checkCanceled())
    {
        // 点位初始化
        initPoints();
        // 初始化
        initXY(mX,mY, mVariables,mVariablesY);
    }
    int nVar = mX.n_cols, nRp = mDataPoints.n_rows;
    int nVars = mX.n_cols * mY.n_cols;
    int nVarsY = mY.n_cols;

    //带宽优选
    for(uword i = 0 ; i<nVars && !checkCanceled();i++)
    {
        if(mBandwidthInitilize[i] == GwmMultiscaleGWRAlgorithm::Null)
        {
            mXi = mX.col(i/nVarsY);
            mYi = mY.col((i+nVarsY)%nVarsY);
            mBandwidthSelectCriterionFunction = bandwidthSizeCriterionVar(mBandwidthSelectionApproach[i]);
            mBandwidthSelectionCurrentIndex = i;
            GwmBandwidthWeight* bw0 = bandwidth(i);
            bool adaptive = bw0->adaptive();
            selector.setBandwidth(bw0);
            selector.setLower(adaptive ? 20 : 0.0);
            selector.setUpper(adaptive ? mDataPoints.n_rows : mSpatialWeights[i].distance()->maxDistance());
            GwmBandwidthWeight* bw = selector.optimize(this);
            if(bw)
            {
                mSpatialWeights[i].setWeight(bw);
            }
        }
    }
    (this->*mCalFunciton)();
    CreateResultLayerData resultLayerData;
    if(!checkCanceled())
    {
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
//普通计算
bool GwmGWcorrelationTaskThread::CalculateSerial(){
    mat rankX = mX;
    rankX.each_col([&](vec x) { x = rank(x); });
    mat rankY = mY;
    rankY.each_col([&](vec y) { y = rank(y); });
    int nVarX = mX.n_cols, nRp = mDataPoints.n_rows,nVarY = mY.n_cols;
    int nVar = nVarX * nVarY;
    emit tick(0,nRp);
    mat xy = join_rows(mX,mY);
    for(int z = 0;z < nVar && !checkCanceled(); z++)
    {
        for(int i = 0; i < nRp && !checkCanceled(); i++)
        {
            vec w = mSpatialWeights[z].weightVector(i);
            double sumw = sum(w);
            vec Wi = w / sumw;
            int colx = z/nVarY;
            int coly = (z+nVarY)%nVarY;
            mLocalMean.row(i) = trans(Wi) * xy;
            mat centerized = xy.each_row() - mLocalMean.row(i);
            mLVar.row(i) = Wi.t() * (centerized % centerized);
            double covjk = covwt(mX.col(colx), mY.col(coly), Wi);
            double sumW2 = sum(Wi % Wi);
            double covjj = mLVar(i, colx) / (1.0 - sumW2);
            double covkk = mLVar(i, coly+nVarX) / (1.0 - sumW2);
            mCovmat(i,z) = covjk;
            mCorrmat(i,z) = covjk / sqrt(covjj * covkk);
            mSCorrmat(i,z) = corwt(rankX.col(colx),rankY.col(coly),Wi);
            emit tick(i,nRp);
        }

    }
    return true;
}
//多线程计算
#ifdef ENABLE_OpenMP
bool GwmGWcorrelationTaskThread::CalculateOmp(){
    mat rankX = mX;
    rankX.each_col([&](vec x) { x = rank(x); });
    mat rankY = mY;
    rankY.each_col([&](vec y) { y = rank(y); });
    int nVarX = mX.n_cols, nRp = mDataPoints.n_rows,nVarY = mY.n_cols;
    int nVar = nVarX * nVarY;
    emit tick(0,nRp);
    int tag = 0;
    int current = 0;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for(int z = 0;z < nVar-1; z++)
    {
        for(int i = 0; i < nRp && !checkCanceled(); i++)
        {
            vec w = mSpatialWeights[z].weightVector(i);
            double sumw = sum(w);
            vec Wi = w / sumw;
            int colx = z/nVarY;
            int coly = z%nVarY;
            mLocalMean.row(i) = trans(Wi) * mX;
            mat centerized = mX.each_row() - mLocalMean.row(i);
            mLVar.row(i) = Wi.t() * (centerized % centerized);
            double covjk = covwt(mX.col(colx), mY.col(coly), Wi);
            double sumW2 = sum(Wi % Wi);
            double covjj = mLVar(i, colx) / (1.0 - sumW2);
            double covkk = mLVar(i, coly) / (1.0 - sumW2);
            mCovmat(i,tag) = covjk;
            mCorrmat(i,tag) = covjk / sqrt(covjj * covkk);
            mSCorrmat(i,tag) = corwt(rankX.col(colx),rankY.col(coly),Wi);
            tag++;
            emit tick(current++,nRp);
        }
    }
    return true;
}
#endif

//vec GwmGWcorrelationTaskThread::findq(const mat &x, const vec &w)
//{
//    int lw = w.n_rows;
//    int lp = 3;
//    vec q = vec(lp,fill::zeros);
//    vec xo = sort(x);
//    vec wo = w(sort_index(x));
//    vec Cum = cumsum(wo);
//    int cond = lw - 1;
//    for(int j = 0; j < lp; j++){
//        double k = 0.25 * (j + 1);
//        for(int i = 0; i < lw; i++){
//            if(Cum(i) > k){
//                cond = i - 1;
//                break;
//            }
//        }
//        if(cond < 0)
//        {
//            cond = 0;
//        }
//        q.row(j) = xo[cond];
//        cond = lw - 1;
//    }
//    return q;
//}

//初始化点坐标
void GwmGWcorrelationTaskThread::initPoints()
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

//    if (mSpatialWeights.distance()->type() == GwmDistance::CRSDistance || mSpatialWeights.distance()->type() == GwmDistance::MinkwoskiDistance)
//    {
//        GwmCRSDistance* d = static_cast<GwmCRSDistance*>(mSpatialWeights.distance());
//        d->setDataPoints(&mDataPoints);
//        d->setFocusPoints(&mDataPoints);
//    }
    for (const GwmSpatialWeight& sw : mSpatialWeights)
    {
        if (sw.distance()->type() == GwmDistance::CRSDistance || sw.distance()->type() == GwmDistance::MinkwoskiDistance)
        {
            GwmCRSDistance* d = sw.distance<GwmCRSDistance>();
            d->setDataPoints(&mDataPoints);
            d->setFocusPoints(&mDataPoints);
        }
    }
}
//初始化字段值
void GwmGWcorrelationTaskThread::initXY(mat& x,mat& y, const QList<GwmVariable>& indepVarsX,QList<GwmVariable>& indepVarsY)
{
    int nDp = mDataLayer->featureCount(), nVarX = indepVarsX.size();
    int nRp = mDataPoints.n_rows;
    int nVarY = indepVarsY.size();
    // Data layer and X,Y
    x = mat(nDp, nVarX, fill::zeros);
    y = mat(nDp, nVarY, fill::zeros);

    QgsFeatureIterator iterator = mDataLayer->getFeatures();
    QgsFeature f;
    bool ok = false;
    for (int i = 0; iterator.nextFeature(f); i++)
    {
        for (int k = 0; k < indepVarsX.size(); k++)
        {
            double vX = f.attribute(indepVarsX[k].name).toDouble(&ok);
            if (ok) x(i, k ) = vX;
            else emit error(tr("variable value cannot convert to a number. Set to 0."));     
        }
    }

    iterator = mDataLayer->getFeatures();
    for (int i = 0; iterator.nextFeature(f); i++)
    {
        for (int k = 0; k < indepVarsY.size(); k++)
        {
            double vY = f.attribute(indepVarsY[k].name).toDouble(&ok);
            if (ok) y(i, k ) = vY;
            else emit error(tr("variable value cannot convert to a number. Set to 0."));
        }
    }
//    mStandardDev = mat(nRp,nVar,fill::zeros);
//    mLocalSkewness = mat(nRp,nVar,fill::zeros);
//    mLCV = mat(nRp,nVar,fill::zeros);
//    if(mQuantile){
//        mLocalMedian = mat(nRp,nVar,fill::zeros);
//        mIQR = mat(nRp,nVar,fill::zeros);
//        mQI = mat(nRp,nVar,fill::zeros);
//    }
        int nCol = nVarX*nVarY;
        mLocalMean = mat(nRp,nVarX+nVarY,fill::zeros);
        mLVar = mat(nRp,nVarX+nVarY,fill::zeros);
        mCovmat   = mat(nRp, nCol, fill::zeros);
        mCorrmat  = mat(nRp, nCol, fill::zeros);
        mSCorrmat = mat(nRp, nCol, fill::zeros);
}


//创建结果图层
void GwmGWcorrelationTaskThread::createResultLayer(CreateResultLayerData data)
{
    QgsVectorLayer* srcLayer =  mDataLayer;
    //第一组变量
    int nVar = mVariables.size();
    //第二组变量
    int nVarY = mVariablesY.size();
    //相关系数计算结果
    int nVars = nVar * nVarY;
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    //避免图层名重复
    if(treeChildCount > 0)
    {
        layerName += QStringLiteral("_GWCorrelations") + "(" + QString::number(treeChildCount) + ")";
    } else
    {
        layerName += QStringLiteral("_GWCorrelations");
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
        for(int i = 0 ; i < nVars ; i++ )
        {
            QString variableName = title + "_" + mVariables[i/nVarY].name + "." + mVariablesY[(i+nVarY)%nVarY].name;
            fields.append(QgsField(variableName, QVariant::Double, QStringLiteral("double")));
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
//设置多线程字段
void GwmGWcorrelationTaskThread::setParallelType(const IParallelalbe::ParallelType &type)
{
    if (type & parallelAbility())
    {
        mParallelType = type;
        switch (type) {
        case IParallelalbe::ParallelType::SerialOnly:
//            mRegressionFunction = &GwmBasicGWRAlgorithm::regressionSerial;
            mCalFunciton = &GwmGWcorrelationTaskThread::CalculateSerial;
            break;
#ifdef ENABLE_OpenMP
        case IParallelalbe::ParallelType::OpenMP:
            mCalFunciton = &GwmGWcorrelationTaskThread::CalculateOmp;
            break;
#endif
        default:
            mCalFunciton = &GwmGWcorrelationTaskThread::CalculateSerial;
            break;
        }
    }
}
//CV值计算
double GwmGWcorrelationTaskThread::bandwidthSizeCriterionVarCVSerial(GwmBandwidthWeight *bandwidthWeight)
{
    int var = mBandwidthSelectionCurrentIndex;
    uword nDp = mDataPoints.n_rows;
    vec shat(2, fill::zeros);
    double cv = 0.0;
    for (uword i = 0; i < nDp && !checkCanceled(); i++)
    {
        vec d = mSpatialWeights[var].distance()->distance(i);
        vec w = bandwidthWeight->weight(d);
        w(i) = 0.0;
        mat xtw = trans(mXi % w);
        mat xtwx = xtw * mXi;
        mat xtwy = xtw * mYi;
        try
        {
            mat xtwx_inv = inv_sympd(xtwx);
            vec beta = xtwx_inv * xtwy;
            double res = mYi(i) - det(mXi(i) * beta);
            cv += res * res;
        }
        catch (...)
        {
            return DBL_MAX;
        }
    }
    if(!checkCanceled()) return cv;
    else return DBL_MAX;
}
//AIC值计算
double GwmGWcorrelationTaskThread::bandwidthSizeCriterionVarAICSerial(GwmBandwidthWeight *bandwidthWeight)
{
    int var = mBandwidthSelectionCurrentIndex;
    uword nDp = mDataPoints.n_rows;
    mat betas(1, nDp, fill::zeros);
    vec shat(2, fill::zeros);
    for (uword i = 0; i < nDp && !checkCanceled(); i++)
    {
        vec d = mSpatialWeights[var].distance()->distance(i);
        vec w = bandwidthWeight->weight(d);
        mat xtw = trans(mXi % w);
        mat xtwx = xtw * mXi;
        mat xtwy = xtw * mYi;
        try
        {
            mat xtwx_inv = inv_sympd(xtwx);
            betas.col(i) = xtwx_inv * xtwy;
            mat ci = xtwx_inv * xtw;
            mat si = mXi(i) * ci;
            shat(0) += si(0, i);
            shat(1) += det(si * si.t());
        }
        catch (std::exception e)
        {
            return DBL_MAX;
        }
    }
    double value = GwmGWcorrelationTaskThread::AICc(mXi, mYi, betas.t(), shat);
    if(!checkCanceled()) return value;
    else return DBL_MAX;
}
//带宽优选函数类型设置
GwmGWcorrelationTaskThread::BandwidthSizeCriterionFunction GwmGWcorrelationTaskThread::bandwidthSizeCriterionVar(GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType type)
{
    switch (type)
    {
        case GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType::CV:
            return &GwmGWcorrelationTaskThread::bandwidthSizeCriterionVarCVSerial;
            break;
        case GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType::AIC:
            return &GwmGWcorrelationTaskThread::bandwidthSizeCriterionVarAICSerial;
            break;
    }
}

void GwmGWcorrelationTaskThread::setSpatialWeights(const QList<GwmSpatialWeight> &spatialWeights)
{
    GwmSpatialMultiscaleAlgorithm::setSpatialWeights(spatialWeights);
}

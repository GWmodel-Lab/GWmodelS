#include "gwmgwpcataskthread.h"
#include <SpatialWeight/gwmcrsdistance.h>
#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"

#include <omp.h>
GwmGWPCATaskThread::GwmGWPCATaskThread() : GwmSpatialMonoscaleAlgorithm()
{

}

void GwmGWPCATaskThread::run()
{
    // 设置矩阵
    emit message(QString(tr("Setting data points ...")));
    initPoints();
    //
    emit message(QString(tr("Setting X and Y.")));
    initXY(mX,mVariables);
    //选带宽
    //这里判断是否选带宽
    if(mIsAutoselectBandwidth)
    {
        emit message(QString(tr("Automatically selecting bandwidth ...")));
        emit tick(0, 0);
        GwmBandwidthWeight* bandwidthWeight0 = static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight());
        mSelector.setBandwidth(bandwidthWeight0);
        double tmpMaxD = mSpatialWeight.distance()->maxDistance();
        double lower = bandwidthWeight0->adaptive() ? 2 : tmpMaxD / 5000;
        double upper = bandwidthWeight0->adaptive() ? mDataPoints.n_rows : tmpMaxD;
        mSelector.setLower(lower);
        mSelector.setUpper(upper);
        GwmBandwidthWeight* bandwidthWeight = mSelector.optimize(this);
        if(bandwidthWeight)
        {
            mSpatialWeight.setWeight(bandwidthWeight);
        }
    }
    emit message(QString(tr("Principle components analyzing ...")));
    //存储d的计算值
    mLatestWt = mat(mDataPoints.n_rows,1,fill::zeros);
    mat sdev;
    if(scoresCal())
    {
        mLocalPV = pca(mX,mLoadings,sdev,mScores);
        mVariance = sdev % sdev;
    }else if(scoresCal() == false){
        mLocalPV = pcaNotScores(mX,mLoadings,mVariance);
    }

    //准备resultlayer的数据

    //win_var_PC1 列
    // 取RW矩阵每一行最大的列的索引
    QList<QString> win_var_PC1;
    uvec iWinVar = index_max(mLoadings.slice(0), 1);
    for(int i = 0; i < mDataPoints.n_rows; i++)
    {
        win_var_PC1.append(mVariables.at(iWinVar(i)).name);
    }
    //Com.1_PV列
    //有几列生成几列
    CreateResultLayerData resultLayerData = {
        qMakePair(QString("Comp.%1_PV"), mLocalPV),
        qMakePair(QString("local_CP"), sum(mLocalPV, 1))
    };
    createResultLayer(resultLayerData,win_var_PC1);
    emit success();
}

bool GwmGWPCATaskThread::isValid()
{
    GwmBandwidthWeight* bandwidth = static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight());
    if(bandwidth){
        if(!mIsAutoselectBandwidth)
        {
            if(bandwidth->adaptive()){
                if(bandwidth->bandwidth() <= mVariables.size()){
                    return false;
                }
            }
        }
        if(mVariables.size() == 0){
            return false;
        }
        if(k()<=0 || k() > mVariables.size()){
            return false;
        }
    }else{
        return false;
    }
    return true;
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
    if (mSpatialWeight.distance()->type() == GwmDistance::CRSDistance || mSpatialWeight.distance()->type() == GwmDistance::MinkwoskiDistance)
    {
        GwmCRSDistance* d = mSpatialWeight.distance<GwmCRSDistance>();
        d->setDataPoints(&mDataPoints);
        d->setFocusPoints(&mDataPoints);
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

void GwmGWPCATaskThread::wpca(const mat &x, const vec &wt, mat &V, vec &S)
{
    //首先完成中心化
    mat xw = x.each_col() % wt;
    mat centerized = (x.each_row() - sum(xw) / sum(wt)).each_col() % sqrt(wt);
    //SVD
    mat U;
    svd(U,S,V,centerized);
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

void GwmGWPCATaskThread::setBandwidthSelectionCriterionType(const GwmGWPCATaskThread::BandwidthSelectionCriterionType &bandwidthSelectionCriterionType)
{
    mBandwidthSelectionCriterionType = bandwidthSelectionCriterionType;
    QMap<QPair<BandwidthSelectionCriterionType, IParallelalbe::ParallelType>, BandwidthSelectCriterionFunction> mapper = {
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, IParallelalbe::ParallelType::SerialOnly), &GwmGWPCATaskThread::bandwidthSizeCriterionCVSerial),
        std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, IParallelalbe::ParallelType::OpenMP), &GwmGWPCATaskThread::bandwidthSizeCriterionCVOmp),
        //std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, IParallelalbe::ParallelType::CUDA), &GwmGWPCATaskThread::bandwidthSizeCriterionCVCuda),
        //std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, IParallelalbe::ParallelType::SerialOnly), &GwmGWPCATaskThread::bandwidthSizeCriterionAICSerial),
        //std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, IParallelalbe::ParallelType::OpenMP), &GwmGWPCATaskThread::bandwidthSizeCriterionAICOmp),
        //std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, IParallelalbe::ParallelType::CUDA), &GwmGWPCATaskThread::bandwidthSizeCriterionAICCuda)
    };
    mBandwidthSelectCriterionFunction = mapper[qMakePair(bandwidthSelectionCriterionType, mParallelType)];
}

bool GwmGWPCATaskThread::isAutoselectBandwidth() const
{
    return mIsAutoselectBandwidth;
}

void GwmGWPCATaskThread::setIsAutoselectBandwidth(bool isAutoselectBandwidth)
{
    mIsAutoselectBandwidth = isAutoselectBandwidth;
}

void GwmGWPCATaskThread::setVariables(const QList<GwmVariable> &variables)
{
    mVariables = variables;
}

void GwmGWPCATaskThread::setParallelType(const IParallelalbe::ParallelType &type)
{
    if(type & parallelAbility())
    {
        mParallelType = type;
        switch (type) {
        case IParallelalbe::ParallelType::SerialOnly:
            setBandwidthSelectionCriterionType(mBandwidthSelectionCriterionType);
            mPcaFunction = &GwmGWPCATaskThread::pcaSerial;
            break;
        case IParallelalbe::ParallelType::OpenMP:
            setBandwidthSelectionCriterionType(mBandwidthSelectionCriterionType);
            mPcaFunction = &GwmGWPCATaskThread::pcaOmp;
            break;
        }
    }
}

void GwmGWPCATaskThread::createResultLayer(CreateResultLayerData data, QList<QString> winvar)
{
    QgsVectorLayer* srcLayer = mDataLayer;
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    layerName += QStringLiteral("_GWPCA");
    mResultLayer = new QgsVectorLayer(layerFileName, layerName, QStringLiteral("memory"));
    mResultLayer->setCrs(srcLayer->crs());

    // 设置字段
    QgsFields fields;
    for (QPair<QString, const mat&> item : data)
    {
        QString title = item.first;
        const mat& value = item.second;
        if (value.n_cols > 1)
        {
            for (int k = 0; k < value.n_cols; k++)
            {
                QString fieldName = title.arg(k+1);
                fields.append(QgsField(fieldName, QVariant::Double, QStringLiteral("double")));
            }
        }
        else
        {
            fields.append(QgsField(title, QVariant::Double, QStringLiteral("double")));
        }
    }
    fields.append(QgsField("win_var_PC1",QVariant::String,QStringLiteral("varchar"),255));
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
        feature.setAttribute("win_var_PC1",winvar[i]);

        mResultLayer->addFeature(feature);
    }
    mResultLayer->commitChanges();
}

double GwmGWPCATaskThread::bandwidthSizeCriterionCVSerial(GwmBandwidthWeight *weight)
{
    int n = mX.n_rows;
    int m = mX.n_cols;
    double score = 0;
    //主循环开始
    //主循环
    for (int i = 0; i < n; i++)
    {
        vec distvi = mSpatialWeight.distance()->distance(i);
        vec wt = weight->weight(distvi);
        wt(i) = 0;
        //取wt大于0的部分
        //临时变量?很麻烦
        uvec positive = find(wt > 0);
        vec newWt = wt.elem(positive);
        mat newX = mX.rows(positive);
        //判断length(newWt)
        if(newWt.n_rows <=1)
        {
            score = DBL_MAX;
            break;
        }
        //调用PCA函数
        //事先准备好的S和V
        mat V;
        vec S;
        wpca(newX, newWt, V, S);
        V = V.cols(0, mK - 1);
        V = V * trans(V);
        score = score + pow(sum(mX.row(i) - mX.row(i) * V),2);
    }
    return score;
}

double GwmGWPCATaskThread::bandwidthSizeCriterionCVOmp(GwmBandwidthWeight *weight)
{
    int n = mX.n_rows;
    int m = mX.n_cols;
    double score = 0;
    //主循环开始
    //主循环
    bool flag = true;
    vec score_all(mOmpThreadNum, fill::zeros);
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < n; i++)
    {
        if(flag)
        {
            int thread = omp_get_thread_num();
            vec distvi = mSpatialWeight.distance()->distance(i);
            vec wt = weight->weight(distvi);
            qDebug() << weight->bandwidth();
            wt(i) = 0;
            //取wt大于0的部分
            //临时变量?很麻烦
            uvec positive = find(wt > 0);
            vec newWt = wt.elem(positive);
            mat newX = mX.rows(positive);
            //判断length(newWt)
            if(newWt.n_rows <=1)
            {
                flag=false;
            }else{
                //调用PCA函数
                //事先准备好的S和V
                mat V;
                vec S;
                wpca(newX, newWt, V, S);
                V = V.cols(0, mK - 1);
                V = V * trans(V);
                score_all(thread) += pow(sum(mX.row(i) - mX.row(i) * V),2);
            }
        }
    }
    score = sum(score_all);
    return score;
}

bool GwmGWPCATaskThread::scoresCal() const
{
    return mScoresCal;
}

void GwmGWPCATaskThread::setScoresCal(bool scoresCal)
{
    mScoresCal = scoresCal;
}

mat GwmGWPCATaskThread::pcaSerial(const mat &x, cube &loadings, mat &sdev, cube &scores)
{
    int nDp = mDataPoints.n_rows, nVar = mVariables.size();
    //存储d的计算值
    mat d_all(nVar, nDp,fill::zeros);
    // 初始化矩阵
    loadings = cube(nDp, nVar, mK, fill::zeros);
    scores = cube(nDp, mK, nDp, fill::zeros);
    for(int i=0;i<nDp;i++)
    {
        //vec distvi = mSpatialWeight.distance()->distance(i);
        vec wt = mSpatialWeight.spatialWeight(i);
        //取wt大于0的部分
        //临时变量?很麻烦
        uvec positive = find(wt > 0);
        vec newWt = wt.elem(positive);
        mat newX = x.rows(positive);
        if(newWt.n_rows<=5)
        {
            break;
        }
        //调用PCA函数
        //事先准备好的D和V
        mat V;
        vec d;
        wpca(newX,newWt,V,d);
        //存储最新的wt
        mLatestWt = newWt;
        d_all.col(i) = d;
        //计算loadings
        for(int j = 0; j < mK; j++)
        {
            loadings.slice(j).row(i) = trans(V.col(j));
        }
        //计算scores
        //如果点数大于4096不保存scores
        if(mDataPoints.n_rows <= 4096)
        {
            mat scorei(nDp, mK, fill::zeros);
            for(int j = 0; j < mK; j++)
            {
                mat score = newX.each_row() % trans(V.col(j));
                scorei.col(j) = sum(score, 1);
            }
            scores.slice(i) = scorei;
        }
    }
    //R代码中的d1计算
    d_all = trans(d_all);
    mat variance = (d_all / pow(sum(mLatestWt),0.5)) % (d_all / pow(sum(mLatestWt),0.5));
    //计算sdev
    sdev = sqrt(variance);
    //dResult1.print();
    //取dResult1的前K列
    mat pv = variance.cols(0, mK - 1).each_col() % (1.0 / sum(variance,1)) * 100.0;
    return pv;
}

mat GwmGWPCATaskThread::pcaOmp(const mat &x, cube &loadings, mat &sdev, cube &scores)
{
    int nDp = mDataPoints.n_rows, nVar = mVariables.size();
    //存储d的计算值
    mat d_all(nVar, nDp, fill::zeros);
    // 初始化矩阵
    loadings = cube(nDp, nVar, mK, fill::zeros);
    scores = cube(nDp, mK, nDp, fill::zeros);
#pragma omp parallel for num_threads(mOmpThreadNum)
    for(int i=0;i<nDp;i++)
    {
        //vec distvi = mSpatialWeight.distance()->distance(i);
        vec wt = mSpatialWeight.spatialWeight(i);
        //取wt大于0的部分
        //临时变量?很麻烦
        uvec positive = find(wt > 0);
        vec newWt = wt.elem(positive);
        mat newX = x.rows(positive);
        if(newWt.n_rows<=5)
        {
            break;
        }
        //调用PCA函数
        //事先准备好的D和V
        mat V;
        vec d;
        wpca(newX,newWt,V,d);
        //存储最新的wt
        if(i == nDp - 1)
        {
            mLatestWt = newWt;
        }
        d_all.col(i) = d;
        //计算loadings
        for(int j=0;j<mK;j++)
        {
            loadings.slice(j).row(i) = trans(V.col(j));
        }
        //计算scores
        if(mDataPoints.n_rows <= 4096)
        {
            mat scorei(nDp, mK, fill::zeros);
            for(int j = 0; j < mK; j++)
            {
                mat score = newX.each_row() % trans(V.col(j));
                scorei.col(j) = sum(score, 1);
            }
            scores.slice(i) = scorei;
        }
        //计算sdev
    }
    //R代码中的d1计算
    d_all = trans(d_all);
    mat variance = (d_all / pow(sum(mLatestWt),0.5)) % (d_all / pow(sum(mLatestWt),0.5));
    //计算sdev
    sdev = sqrt(variance);
    //dResult1.print();
    //取dResult1的前K列
    mat pv = variance.cols(0,mK-1).each_col() % (1 / sum(variance,1)) *100;
    return pv;
}

mat GwmGWPCATaskThread::pcaNotScoresSerial(const mat &x, cube &loadings, mat &variance)
{
    int nDp = mDataPoints.n_rows, nVar = mVariables.size();
    //存储d的计算值
    mat d_all(nVar, nDp,fill::zeros);
    // 初始化矩阵
    loadings = cube(nDp, nVar, mK, fill::zeros);
    //scores = cube(nDp, mK, nDp, fill::zeros);
    for(int i=0;i<nDp;i++)
    {
        //vec distvi = mSpatialWeight.distance()->distance(i);
        vec wt = mSpatialWeight.spatialWeight(i);
        //取wt大于0的部分
        //临时变量?很麻烦
        uvec positive = find(wt > 0);
        vec newWt = wt.elem(positive);
        mat newX = x.rows(positive);
        if(newWt.n_rows<=5)
        {
            break;
        }
        //调用PCA函数
        //事先准备好的D和V
        mat V;
        vec d;
        wpca(newX,newWt,V,d);
        //存储最新的wt
        mLatestWt = newWt;
        d_all.col(i) = d;
        //计算loadings
        for(int j = 0; j < mK; j++)
        {
            loadings.slice(j).row(i) = trans(V.col(j));
        }
    }
    //R代码中的d1计算
    d_all = trans(d_all);
    variance = (d_all / pow(sum(mLatestWt),0.5)) % (d_all / pow(sum(mLatestWt),0.5));
    //dResult1.print();
    //取dResult1的前K列
    mat pv = variance.cols(0, mK - 1).each_col() % (1.0 / sum(variance,1)) * 100.0;
    return pv;
}

mat GwmGWPCATaskThread::pcaNotScoresOmp(const mat &x, cube &loadings, mat &variance)
{
    int nDp = mDataPoints.n_rows, nVar = mVariables.size();
    //存储d的计算值
    mat d_all(nVar, nDp,fill::zeros);
    // 初始化矩阵
    loadings = cube(nDp, nVar, mK, fill::zeros);
    //scores = cube(nDp, mK, nDp, fill::zeros);
#pragma omp parallel for num_threads(mOmpThreadNum)
    for(int i=0;i<nDp;i++)
    {
        //vec distvi = mSpatialWeight.distance()->distance(i);
        vec wt = mSpatialWeight.spatialWeight(i);
        //取wt大于0的部分
        //临时变量?很麻烦
        uvec positive = find(wt > 0);
        vec newWt = wt.elem(positive);
        mat newX = x.rows(positive);
        if(newWt.n_rows<=5)
        {
            break;
        }
        //调用PCA函数
        //事先准备好的D和V
        mat V;
        vec d;
        wpca(newX,newWt,V,d);
        //存储最新的wt
        if(i == nDp - 1)
        {
            mLatestWt = newWt;
        }
        d_all.col(i) = d;
        //计算loadings
        for(int j = 0; j < mK; j++)
        {
            loadings.slice(j).row(i) = trans(V.col(j));
        }
    }
    //R代码中的d1计算
    d_all = trans(d_all);
    variance = (d_all / pow(sum(mLatestWt),0.5)) % (d_all / pow(sum(mLatestWt),0.5));
    //dResult1.print();
    //取dResult1的前K列
    mat pv = variance.cols(0, mK - 1).each_col() % (1.0 / sum(variance,1)) * 100.0;
    return pv;
}

#include "gwmgwpcataskthread.h"
#include <SpatialWeight/gwmcrsdistance.h>
#include "TaskThread/gwmgeographicalweightedregressionalgorithm.h"

GwmGWPCATaskThread::GwmGWPCATaskThread() : GwmSpatialMonoscaleAlgorithm()
{

}

void GwmGWPCATaskThread::run()
{
    // 设置矩阵
    initPoints();
//    GwmCRSDistance d(mDataLayer->featureCount(), mDataLayer->crs().isGeographic());
//    d.setDataPoints(&mDataPoints);
//    d.setFocusPoints(&mDataPoints);
    //
    initXY(mX,mVariables);
    //选带宽
    //这里判断是否选带宽
    if(mIsAutoselectBandwidth)
    {
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
    //存储d的计算值
//    mSpatialWeight.setDistance(d);
//    mSpatialWeight.setWeight(GwmBandwidthWeight(100, true, GwmBandwidthWeight::Gaussian));;
    mat dResult(mDataPoints.n_rows, mVariables.size(),fill::zeros);
    //存储最新的wt
    vec latestWt(mDataPoints.n_rows,1,fill::zeros);
    //存储R代码中的W
    mat RW(mDataPoints.n_rows,mVariables.size(),fill::zeros);
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
                newWt(j) = wt(k);
                newX.row(j) = mX.row(k);
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
        dResult.row(i) = trans(D);
        //dResult.row(0).print();
        RW.row(i) = trans(V.col(0));
    }
    //dResult.print();
    //R代码中的d1计算
    mat tmp(mDataPoints.n_rows, mVariables.size(),fill::zeros);
    dResult1 = tmp;
    dResult1 = (dResult / pow(sum(latestWt),0.5)) % (dResult / pow(sum(latestWt),0.5));
    //dResult1.print();
    //取dResult1的前K列
    localPV = dResult1.cols(0,mk-1).each_col() % (1 / sum(dResult1,1)) *100;
    localPV.print();
    //R代码 的 Local variance
    //t(apply(d1, 2, summary))
    const vec p = { 0.0, 0.25, 0.5, 0.75, 1.0 };
    for(int i=0;i<dResult1.n_cols;i++)
    {
        vec q = quantile(dResult1.col(i), p);
        q.print("Comp1:");
    }
    //R代码 的 Local Proportion of Variance
    //t(apply(local.PV, 2, summary))
    for(int i=0;i<localPV.n_cols;i++)
    {
        vec q = quantile(localPV.col(i), p);
        q.print("Comp2:");
    }
    //Cumulative summary(rowSums(local.PV))
    sum(localPV,1).print();
    vec localPVSum = sum(localPV,1);
    vec Cumulative = quantile(localPVSum,p);
    //Cumulative.print();cc/

    //准备resultlayer的数据

    //win_var_PC1 列
    // 取RW矩阵每一行最大的列的索引
    vec RWmaxIndex(mDataPoints.n_rows,fill::zeros);
    QList<QString> win_var_PC1;
    for(int i=0;i<RWmaxIndex.n_rows;i++)
    {
        RWmaxIndex.row(i) = RW.row(i).index_max();
        win_var_PC1.append(mVariables.at(RW.row(i).index_max()).name);
    }
    //Com.1_PV列
    //有几列生成几列
    CreateResultLayerData resultLayerData = {
        qMakePair(QString("%1"), localPV),
        qMakePair(QString("local_CP"), localPVSum),
        //qMakePair(QString("win_var_PC1"), win_var_PC1)
    };
    createResultLayer(resultLayerData,win_var_PC1);
    emit success();
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
        vec distvi = mSpatialWeight.distance()->distance(i);
        vec wt = weight->weight(distvi);
        qDebug() << weight->bandwidth();
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
                newWt(j) = wt(k);
                newX.row(j) = mX.row(k);
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
                QChar c(k+1);
                QString variableName = QString("Comp.%1_PV").arg(k+1);
                QString fieldName = title.arg(variableName);
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

#include "gwmlcrgwrtaskthread.h"

GwmLcrGWRTaskThread::GwmLcrGWRTaskThread():GwmGeographicalWeightedRegressionAlgorithm()
{

}

double GwmLcrGWRTaskThread::getMlambda() const
{
    return mlambda;
}

void GwmLcrGWRTaskThread::setHashatmatix(bool value)
{
    hashatmatix = value;
}

bool GwmLcrGWRTaskThread::getHashatmatix() const
{
    return hashatmatix;
}

double GwmLcrGWRTaskThread::getMcnThresh() const
{
    return mcnThresh;
}

void GwmLcrGWRTaskThread::run()
{
    // 点位初始化
    emit message(QString(tr("Setting data points")) + (hasRegressionLayer() ? tr(" and regression points") : "") + ".");
    initPoints();
    // 设置矩阵
    initXY(mX, mY, mDepVar, mIndepVars);
    //选带宽
    //这里判断是否选带宽
    if(mIsAutoselectBandwidth)
    {
        GwmBandwidthWeight* bandwidthWeight0 = static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight());
        //GwmBandwidthSizeSelector selector;
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
    mat betas(mDataPoints.n_rows,mX.n_cols,fill::zeros);
    vec localcn(mDataPoints.n_rows,fill::zeros);
    vec locallambda(mDataPoints.n_rows,fill::zeros);
    vec hatrow(mDataPoints.n_rows,fill::zeros);
    //yhat赋值
    mBetas = regression(mX, mY);
    vec mYHat = fitted(mX,mBetas);
    vec mResidual = mY - mYHat;
    mDiagnostic.RSS = sum(mResidual % mResidual);
    mDiagnostic.ENP = 2*this->trs - this->trsts;
    mDiagnostic.EDF = mDataPoints.n_rows - mDiagnostic.ENP;
    //mDiagnostic.RSquare =
    double s2 = mDiagnostic.RSS / (mDataPoints.n_rows - mDiagnostic.ENP);
    mDiagnostic.AIC = mDataPoints.n_rows * (log(2*M_PI*s2)+1) + 2*(mDiagnostic.ENP + 1);
    mDiagnostic.AICc = mDataPoints.n_rows * (log(2*M_PI*s2)) + mDataPoints.n_rows*( (1+mDiagnostic.ENP/mDataPoints.n_rows) / (1-(mDiagnostic.ENP+2)/mDataPoints.n_rows) );
    // enp 、 edf
    // s2
    // aic 、 aicc
    //调用gwr.lcr.cv.contrib
    //生成诊断信息
    // 诊断
    //mDiagnostic = CalcDiagnostic(mX, mY, mBetas, mShat);
    CreateResultLayerData resultLayerData = {
        qMakePair(QString("%1"), mBetas),
        qMakePair(QString("y"), mY),
//        qMakePair(QString("yhat"), yhat),
//        qMakePair(QString("residual"), res),
//        qMakePair(QString("Stud_residual"), stu_res),
//        qMakePair(QString("%1_SE"), mBetasSE),
//        qMakePair(QString("%1_TV"), betasTV),
//        qMakePair(QString("localR2"), localR2)
    };
    createResultLayer(resultLayerData);
    emit success();
}

double GwmLcrGWRTaskThread::criterion(GwmBandwidthWeight *weight)
{
    //行数
    double n = mX.n_rows;
    //列数
    double m = mX.n_cols;
    //初始化矩阵
    mat betas = mat(n,m,fill::zeros);
    vec localcn(n,fill::zeros);
    vec locallambda(n,fill::zeros);
    //取mX不含第一列的部分
    mat mXnot1 = mat(mX.n_rows,mX.n_cols-1,fill::zeros);
    for(int i=0;i<mX.n_cols-1;i++)
    {
        mXnot1.col(i) = mX.col(i+1);
    }
    //主循环
    for (int i = 0; i < n; i++)
    {
//        vec distvi = distance(i);
//        vec wgt = gwWeight(distvi, bw, kernel, adaptive);
        vec distvi = mSpatialWeight.distance()->distance(i);
        vec wgt = weight->weight(distvi);
        //vec wgt = mSpatialWeight.spatialWeight(mDataPoints.row(i),mDataPoints);
        wgt(i) = 0;
        mat wgtspan(1,mXnot1.n_cols,fill::ones);
        mat wgtspan1(1,mX.n_cols,fill::ones);
        //计算xw
        mat xw = mXnot1 % (wgt * wgtspan);
        //计算x1w
        mat x1w = mX % (wgt * wgtspan1);
        //计算用于SVD分解的矩阵
        //x1w按列求和
//        for(int j = 0;j<x1w.n_cols;j++)
//        {
//            mat tmp(x1w.n_rows,x1w.n_cols,fill::ones);
//            tmp = x1w % x1w;
//            x1w.col(j) = x1w.col(j) / sqrt(sum(tmp.col(j)));
//        }
        //计算svd.x
        //mat U,V均为正交矩阵，S为奇异值构成的列向量
        mat U,V;
        colvec S;
        svd(U,S,V,x1w.each_row() / sqrt(sum(x1w % x1w, 0)));
        //赋值操作
        S.print();
        //qDebug() << S(m);
        localcn(i)=S(0)/S(m-1);
        locallambda(i) = mlambda;
        if(mlambdaAdjust){
            if(localcn(i)>mcnThresh){
                locallambda(i) = (S(0) - mcnThresh*S(m-1)) / (mcnThresh - 1);
            }
        }
        betas.row(i) = trans( ridgelm(wgt,locallambda(i)) );
    }
    //yhat赋值
    vec mYHat = fitted(mX,betas);
//    for(int i=0;i<mYHat.n_rows;i++)
//    {
//        mat tmp(mX.n_rows,mX.n_cols,fill::zeros);
//        tmp = (mX % betas);
//        mYHat.row(i) = sum(tmp.row(i));
//    }
    //计算residual
    vec mResidual = mY - mYHat;
    //计算cv
    double cv = sum(mResidual % mResidual);
    return cv;
}

bool GwmLcrGWRTaskThread::getIsAutoselectBandwidth() const
{
    return mIsAutoselectBandwidth;
}

//vec GwmLcrGWRTaskThread::LcrCVContrib(double bw, int kernel, bool adaptive, double lambda, bool lambdaAdjust, double cnThresh)
//{
//    //行数
//    double n = mX.n_rows;
//    //列数
//    double m = mX.n_cols;
//    //初始化矩阵
//    mat betas = mat(n,m,fill::zeros);
//    vec localcn(n,fill::zeros);
//    vec locallambda(n,fill::zeros);
//    //取mX不含第一列的部分
//    mat mXnot1 = mat(mX.n_rows,mX.n_cols-1,fill::zeros);
//    for(int i=0;i<mX.n_cols-1;i++)
//    {
//        mXnot1.col(i) = mX.col(i+1);
//    }
//    //主循环
//    for (int i = 0; i < n; i++)
//    {
//        vec distvi = distance(i);
//        vec wgt = gwWeight(distvi, bw, kernel, adaptive);
//        wgt(i) = 0;
//        mat wgtspan(1,mXnot1.n_cols,fill::ones);
//        mat wgtspan1(1,mX.n_cols,fill::ones);
//        //计算xw
//        mat xw = mXnot1 % (wgt * wgtspan);
//        //计算x1w
//        mat x1w = mX % (wgt * wgtspan1);
//        //计算用于SVD分解的矩阵
//        //x1w按列求和
//        for(int j = 0;j<x1w.n_cols;j++)
//        {
//            mat tmp(x1w.n_rows,x1w.n_cols,fill::ones);
//            tmp = x1w % x1w;
//            x1w.col(j) = x1w.col(j) / sqrt(sum(tmp.col(j)));
//        }
//        //计算svd.x
//        //mat U,V均为正交矩阵，S为奇异值构成的列向量
//        mat U,V;
//        colvec S;
//        svd(U,S,V,x1w.each_row() / sqrt(sum(x1w % x1w, 0)));
//        //赋值操作
//        S.print();
//        //qDebug() << S(m);
//        localcn(i)=S(0)/S(m-1);
//        locallambda(i) = lambda;
//        if(lambdaAdjust){
//            if(localcn(i)>cnThresh){
//                locallambda(i) = (S(0) - cnThresh*S(m-1)) / (cnThresh - 1);
//            }
//        }
//        mBetas.row(i) = trans( ridgelm(wgt,locallambda(i)) );
//    }
//    //yhat赋值
//    mYHat = fitted(mX,mBetas);
//    //计算cv向量
//    vec cv(mY.n_rows,fill::zeros);
//    cv = mY - mYHat;
//    return cv;
//}

vec GwmLcrGWRTaskThread::ridgelm(const vec &w, double lambda)
{
    //X默认加了1
    //默认add.int为False
    mat wspan(1, mX.n_cols, fill::ones);
    mat Xw = mX % (sqrt(w) * wspan);
    mat yw = mY % (sqrt(w));
    //求标准差
    //取mX不含第一列的部分
    mat mXnot1 = mX.cols(1, mX.n_cols - 1);
    //标准差结果矩阵
    mat Xsd(1, mX.n_cols, fill::ones);
    Xsd.cols(1, mX.n_cols - 1) = stddev(mX.cols(1, mX.n_cols - 1),0);
    //Xsd = trans(Xsd);
    //计算Xws
    //mat Xws = mat(Xw.n_rows,Xw.n_cols,fill::ones);
//    for(int i=0;i<Xw.n_rows;i++)
//    {
//        Xws.col(i) = Xw.col(i)/Xsd(0,i);
//    }
    if (abs(w(0) - 1.0) < 1e-6) {
        Xsd.print();
        Xw.print("xw");
        w.print("w");
    }
    mat Xws = Xw.each_row() / Xsd;
    //计算y的sd值
    // double sumTmp = 0;
    // for(int i=0;i<mY.n_rows;i++){
    //     for(int j=0;j<mY.n_cols;j++){
    //         sumTmp+=pow((mY(i,j)-sum(mY)),2);
    //     }
    // }
    double ysd = stddev(yw.col(0));
    mat yws = yw / ysd;
    //计算b值
    //计算crossprod(Xws)
    mat tmpCrossprodXws = trans(Xws)*Xws;
    //生成diag矩阵
    mat tmpdiag = eye(Xws.n_cols,Xws.n_cols);
    //方程求解
    mat tmpXX = tmpCrossprodXws+lambda*tmpdiag;
    mat tmpYY = trans(Xws)*yws;
    vec resultb = inv(tmpXX)*(tmpYY)*ysd/trans(Xsd);
    //如何返回？？
    return resultb;
}




//double GwmLcrGWRTaskThread::getFixedBwUpper()
//{
//    QgsRectangle extent = this->mLayer->extent();
//    bool longlat = mLayer->crs().isGeographic();
//    mat extentDp(2, 2, fill::zeros);
//    extentDp(0, 0) = extent.xMinimum();
//    extentDp(0, 1) = extent.yMinimum();
//    extentDp(1, 0) = extent.xMaximum();
//    extentDp(1, 1) = extent.yMaximum();

//    vec dist;
//    QMap<QString, QVariant> parameters = mDistSrcParameters.toMap();
//    double p = parameters["p"].toDouble();

//    QString filename = mDistSrcParameters.toString();
//    qint64 featureCount = mFeatureList.size();
//    QFile dmat(filename);
//    double maxDist;
//    switch(mDistSrcType)
//    {
//        case DistanceSourceType::DMatFile:
//            if (dmat.open(QFile::QIODevice::ReadOnly))
//            {
//                QDataStream in(&dmat);
//                //跳过8字节
//                in.skipRawData(8);
//                //存储读出来的数据
//                double tmp;
//                in >> tmp;
//                maxDist =  tmp;
//                //判断是否读到文件末尾
//                while(in.atEnd() != true){
//                    in >> tmp;
//                    if(tmp > maxDist){
//                        //更新最大值
//                        maxDist = tmp;
//                    }
//                }
//                return maxDist;
//            }else{
//                return DBL_MAX;
//            }
//        case DistanceSourceType::Minkowski:
//            dist = gwDist(extentDp, extentDp, 0, p, 0.0, longlat, false);
//            return dist(1);
//        default:
//            dist = gwDist(extentDp, extentDp, 0, 2.0, 0.0, longlat, false);
//            return dist(1);
//    }
//}

arma::mat GwmLcrGWRTaskThread::regression(const arma::mat &x, const arma::vec &y)
{
    mat betas(mDataPoints.n_rows,x.n_cols,fill::zeros);
    vec localcn(mDataPoints.n_rows,fill::zeros);
    vec locallambda(mDataPoints.n_rows,fill::zeros);
    vec hatrow(mDataPoints.n_rows,fill::zeros);
//    double trs = 0;
//    double trsts = 0;
    for(int i=0;i<mDataPoints.n_rows;i++)
    {
//        vec distvi = distance(i);
//        //distvi.print();
//        vec wi = gwWeight(distvi,bw,mBandwidthKernelFunction,madaptive);
        vec wi = mSpatialWeight.spatialWeight(i);
        //wi.print();
        //计算xw
        //取mX不含第一列的部分
        mat mXnot1 = x.cols(1, x.n_cols - 1);
        mat wispan(1,mXnot1.n_cols,fill::ones);
        mat wispan1(1,x.n_cols,fill::ones);
        //计算xw
        mat xw = mXnot1 % (wi * wispan);
        //计算x1w
        mat x1w = x % (wi * wispan1);
        //计算svd.x
        //mat U,V均为正交矩阵，S为奇异值构成的列向量
        //sqrt(sum((x1w % x1w),2));
        mat U,V;
        colvec S;
        svd(U,S,V,x1w.each_row() / sqrt(sum(x1w % x1w, 0)));
        //qDebug() << mX.n_cols;
        //赋值操作
        localcn(i)=S(0)/S(x.n_cols-1);
        locallambda(i) = mlambda;
        if(mlambdaAdjust){
            if(localcn(i)>mcnThresh){
                locallambda(i) = (S(0) - mcnThresh*S(x.n_cols-1)) / (mcnThresh - 1);
            }
        }
        betas.row(i) = trans( ridgelm(wi,locallambda(i)) );
        //如果没有给regressionpoint
        if(hashatmatix)
        {
            mat xm = x;
            mat xtw = trans(x % (wi * wispan1));
            mat xtwx = xtw * x;
            mat xtwxinv = inv(xtwx);
            rowvec hatrow = x1w.row(i) * xtwxinv * trans(x1w);
            this->trs += hatrow(i);
            this->trsts += sum(hatrow % hatrow);
        }
        emit tick(i+1, mDataPoints.n_rows);
    }
    return betas;
}

void GwmLcrGWRTaskThread::createResultLayer(CreateResultLayerData data)
{
    QgsVectorLayer* srcLayer = mRegressionLayer ? mRegressionLayer : mDataLayer;
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    layerName += QStringLiteral("_GWR");
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
            for (uword k = 0; k < value.n_cols; k++)
            {
                QString variableName = k == 0 ? QStringLiteral("Intercept") : mIndepVars[k - 1].name;
                QString fieldName = title.arg(variableName);
                fields.append(QgsField(fieldName, QVariant::Double, QStringLiteral("double")));
            }
        }
        else
        {
            fields.append(QgsField(title, QVariant::Double, QStringLiteral("double")));
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

GwmDiagnostic GwmLcrGWRTaskThread::CalcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat)
{
    vec r = y - sum(betas % x, 1);
    double rss = sum(r % r);
    int n = x.n_rows;
    double AIC = n * log(rss / n) + n * log(2 * datum::pi) + n + shat(0);
    double AICc = n * log(rss / n) + n * log(2 * datum::pi) + n * ((n + shat(0)) / (n - 2 - shat(0)));
    double edf = n - 2 * shat(0) + shat(1);
    double enp = 2 * shat(0) - shat(1);
    double yss = sum((y - mean(y)) % (y - mean(y)));
    double r2 = 1 - rss / yss;
    double r2_adj = 1 - (1 - r2) * (n - 1) / (edf - 1);
    return { rss, AIC, AICc, enp, edf, r2, r2_adj };
}

bool GwmLcrGWRTaskThread::isValid()
{
    if (GwmGeographicalWeightedRegressionAlgorithm::isValid())
    {
        GwmBandwidthWeight* bandwidth = static_cast<GwmBandwidthWeight*>(mSpatialWeight.weight());

        if(!mIsAutoselectBandwidth)
        {
            if(bandwidth->adaptive()){
                if (bandwidth->bandwidth() <= mIndepVars.size()) return false;
            }else{

            }
        }
        return true;
    }
    else return false;
}

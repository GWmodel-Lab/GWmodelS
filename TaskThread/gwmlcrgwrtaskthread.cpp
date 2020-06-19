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
    double s2 = mDiagnostic.RSS / (mDataPoints.n_rows - mDiagnostic.ENP);
    mDiagnostic.AIC = mDataPoints.n_rows * (log(2*M_PI*s2)+1) + 2*(mDiagnostic.ENP + 1);
    mDiagnostic.AICc = mDataPoints.n_rows * (log(2*M_PI*s2)) + mDataPoints.n_rows*( (1+mDiagnostic.ENP/mDataPoints.n_rows) / (1-(mDiagnostic.ENP+2)/mDataPoints.n_rows) );
    // enp 、 edf
    // s2
    // aic 、 aicc
    //调用gwr.lcr.cv.contrib
    //生成诊断信息
    // 诊断
    CreateResultLayerData resultLayerData = {
        qMakePair(QString("%1"), mBetas),
        qMakePair(QString("y"), mY),
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
    if (abs(w(0) - 1.0) < 1e-6) {
        Xsd.print();
        Xw.print("xw");
        w.print("w");
    }
    mat Xws = Xw.each_row() / Xsd;
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

arma::mat GwmLcrGWRTaskThread::regression(const arma::mat &x, const arma::vec &y)
{
    mat betas(mDataPoints.n_rows,x.n_cols,fill::zeros);
    vec localcn(mDataPoints.n_rows,fill::zeros);
    vec locallambda(mDataPoints.n_rows,fill::zeros);
    vec hatrow(mDataPoints.n_rows,fill::zeros);
    for(int i=0;i<mDataPoints.n_rows;i++)
    {
        vec wi = mSpatialWeight.spatialWeight(i);
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

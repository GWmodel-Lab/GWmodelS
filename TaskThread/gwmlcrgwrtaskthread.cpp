#include "gwmlcrgwrtaskthread.h"

GwmLcrGWRTaskThread::GwmLcrGWRTaskThread():GwmGWRTaskThread()
{

}

void GwmLcrGWRTaskThread::run()
{
    // 设置矩阵
    if (!setXY())
    {
        return;
    }
    //
    if(mBandwidthType == BandwidthType::Adaptive){
        madaptive = true;
    }else{
        madaptive = false;
    }
//    qDebug() << madaptive;
//    qDebug() << mBandwidthKernelFunction;
//    qDebug() << mlambda;
//    qDebug() << mlambdaAdjust;
//    qDebug() << mcnThresh;
    //选择带宽
    double bw;
    if(isBandwidthSizeAutoSel){
        bw = LcrBandWidthSelect(mBandwidthKernelFunction,mlambda,mlambdaAdjust,mcnThresh,madaptive);
    }else{
        bw = mBandwidthSize;
    }
    //double bw = 100;
    //arrays for the results
    //mFeatureList.length()
    mat betas(mFeatureList.length(),mX.n_cols,fill::zeros);
    vec localcn(mFeatureList.length(),fill::zeros);
    vec locallambda(mFeatureList.length(),fill::zeros);
    vec hatrow(mFeatureList.length(),fill::zeros);
    double trs = 0;
    double trsts = 0;
    //main loop
    for(int i=0;i<mFeatureList.length();i++)
    {
        vec distvi = distance(i);
        //distvi.print();
        vec wi = gwWeight(distvi,bw,mBandwidthKernelFunction,madaptive);
        //wi.print();
        //计算xw
        //取mX不含第一列的部分
        mat mXnot1 = mX.cols(1, mX.n_cols - 1);
        mat wispan(1,mXnot1.n_cols,fill::ones);
        mat wispan1(1,mX.n_cols,fill::ones);
        //计算xw
        mat xw = mXnot1 % (wi * wispan);
        //计算x1w
        mat x1w = mX % (wi * wispan1);
        //计算svd.x
        //mat U,V均为正交矩阵，S为奇异值构成的列向量
        //sqrt(sum((x1w % x1w),2));
        mat U,V;
        colvec S;
        svd(U,S,V,x1w.each_row() / sqrt(sum(x1w % x1w, 0)));
        //qDebug() << mX.n_cols;
        //赋值操作
        localcn(i)=S(0)/S(mX.n_cols-1);
        locallambda(i) = mlambda;
        if(mlambdaAdjust){
            if(localcn(i)>mcnThresh){
                locallambda(i) = (S(0) - mcnThresh*S(mX.n_cols-1)) / (mcnThresh - 1);
            }
        }
        mBetas.row(i) = trans( ridgelm(wi,locallambda(i)) );
        //如果没有给regressionpoint
        if(!mRegressionLayer)
        {
            mat xm = mX;
            mat xtw = trans(mX % (wi * wispan1));
            mat xtwx = xtw * mX;
            mat xtwxinv = inv(xtwx);
            rowvec hatrow = x1w.row(i) * xtwxinv * trans(x1w);
            trs += hatrow(i);
            trsts += sum(hatrow % hatrow);
        }
        emit tick(i+1, mFeatureList.length());
    }
    //end of main loop
    //yhat赋值
    mYHat = fitted(mX,mBetas);
//    for(int i=0;i<mYHat.n_rows;i++)
//    {
//        mat tmp(mX.n_rows,mX.n_cols,fill::zeros);
//        tmp = (mX % betas);
//        mYHat.row(i) = sum(tmp.row(i));
//    }
    mResidual = mY - mYHat;
    mDiagnostic.RSS = sum(mResidual % mResidual);
    mDiagnostic.ENP = 2*trs - trsts;
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
    createResultLayer();
    emit success();
}

double GwmLcrGWRTaskThread::LcrCV(double bw, int kernel, bool adaptive, double lambda, bool lambdaAdjust, double cnThresh)
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
        vec distvi = distance(i);
        vec wgt = gwWeight(distvi, bw, kernel, adaptive);
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
        locallambda(i) = lambda;
        if(lambdaAdjust){
            if(localcn(i)>cnThresh){
                locallambda(i) = (S(0) - cnThresh*S(m-1)) / (cnThresh - 1);
            }
        }
        mBetas.row(i) = trans( ridgelm(wgt,locallambda(i)) );
    }
    //yhat赋值
    mYHat = fitted(mX,mBetas);
//    for(int i=0;i<mYHat.n_rows;i++)
//    {
//        mat tmp(mX.n_rows,mX.n_cols,fill::zeros);
//        tmp = (mX % betas);
//        mYHat.row(i) = sum(tmp.row(i));
//    }
    //计算residual
    mResidual = mY - mYHat;
    //计算cv
    double cv = sum(mResidual % mResidual);
    return cv;
}

vec GwmLcrGWRTaskThread::LcrCVContrib(double bw, int kernel, bool adaptive, double lambda, bool lambdaAdjust, double cnThresh)
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
        vec distvi = distance(i);
        vec wgt = gwWeight(distvi, bw, kernel, adaptive);
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
        locallambda(i) = lambda;
        if(lambdaAdjust){
            if(localcn(i)>cnThresh){
                locallambda(i) = (S(0) - cnThresh*S(m-1)) / (cnThresh - 1);
            }
        }
        mBetas.row(i) = trans( ridgelm(wgt,locallambda(i)) );
    }
    //yhat赋值
    mYHat = fitted(mX,mBetas);
    //计算cv向量
    vec cv(mY.n_rows,fill::zeros);
    cv = mY - mYHat;
    return cv;
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

double GwmLcrGWRTaskThread::gold(pfApproach p,double xL, double xU, bool adaptBw, int kernel, bool adaptive,double lambda, bool lambdaAdjust,double cnThresh)
{
    const double eps = 1e-4;
    const double R = (sqrt(5)-1)/2;
    int iter = 0;
    double d = R * (xU - xL);
    double x1 = adaptBw ? floor(xL + d) : (xL + d);
    double x2 = adaptBw ? round(xU - d) : (xU - d);
    double f1 = (this->*p)(x1, kernel, adaptive, lambda, lambdaAdjust, cnThresh);
    double f2 = (this->*p)(x2, kernel, adaptive, lambda, lambdaAdjust, cnThresh);
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
            f1 = (this->*p)(x1, kernel, adaptive, lambda, lambdaAdjust, cnThresh);
        }
        else
        {
            xU = x1;
            x1 = x2;
            x2 = adaptBw ? floor(xU - d) : (xU - d);
            f1 = f2;
            f2 = (this->*p)(x2, kernel, adaptive, lambda, lambdaAdjust, cnThresh);
        }
        iter = iter + 1;
        xopt = (f1 < f2) ? x1 : x2;
        d1 = f2 - f1;
    }
    return xopt;
}

double GwmLcrGWRTaskThread::LcrBandWidthSelect(int kernel, double lambda, bool lambdaAdjust, double cnThresh, bool adaptive)
{
    double upper, lower;
    upper = adaptive ? mX.n_rows : getFixedBwUpper();
    lower = adaptive ? 20 : 0.0;
    double bw;
    bw = gold(&GwmLcrGWRTaskThread::LcrCV,lower,upper,adaptive,kernel,adaptive,lambda,lambdaAdjust,cnThresh);
    //显示带宽和CV值
    double cvShow = GwmLcrGWRTaskThread::LcrCV(bw,kernel,adaptive,lambda,lambdaAdjust,cnThresh);
    emit message("BandWidth select....");
    return bw;
}

double GwmLcrGWRTaskThread::getFixedBwUpper()
{
    QgsRectangle extent = this->mLayer->extent();
    bool longlat = mLayer->crs().isGeographic();
    mat extentDp(2, 2, fill::zeros);
    extentDp(0, 0) = extent.xMinimum();
    extentDp(0, 1) = extent.yMinimum();
    extentDp(1, 0) = extent.xMaximum();
    extentDp(1, 1) = extent.yMaximum();

    vec dist;
    QMap<QString, QVariant> parameters = mDistSrcParameters.toMap();
    double p = parameters["p"].toDouble();

    QString filename = mDistSrcParameters.toString();
    qint64 featureCount = mFeatureList.size();
    QFile dmat(filename);
    double maxDist;
    switch(mDistSrcType)
    {
        case DistanceSourceType::DMatFile:
            if (dmat.open(QFile::QIODevice::ReadOnly))
            {
                QDataStream in(&dmat);
                //跳过8字节
                in.skipRawData(8);
                //存储读出来的数据
                double tmp;
                in >> tmp;
                maxDist =  tmp;
                //判断是否读到文件末尾
                while(in.atEnd() != true){
                    in >> tmp;
                    if(tmp > maxDist){
                        //更新最大值
                        maxDist = tmp;
                    }
                }
                return maxDist;
            }else{
                return DBL_MAX;
            }
        case DistanceSourceType::Minkowski:
            dist = gwDist(extentDp, extentDp, 0, p, 0.0, longlat, false);
            return dist(1);
        default:
            dist = gwDist(extentDp, extentDp, 0, 2.0, 0.0, longlat, false);
            return dist(1);
    }
}

void GwmLcrGWRTaskThread::createResultLayer()
{
    emit message("Creating result layer...");
    QgsVectorLayer* srcLayer = mRegressionLayer ? mRegressionLayer : mLayer;
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    if (mBandwidthType == BandwidthType::Fixed)
    {
        layerName += QString("_B%1%2").arg(mBandwidthSizeOrigin, 0, 'f', 3).arg(mBandwidthSize);
    }
    else
    {
        layerName += QString("_B%1").arg(int(mBandwidthSize));
    }
    mResultLayer = new QgsVectorLayer(layerFileName, layerName, QStringLiteral("memory"));
    mResultLayer->setCrs(srcLayer->crs());

    QgsFields fields;
    fields.append(QgsField(QStringLiteral("Intercept"), QVariant::Double, QStringLiteral("double")));
    for (int index : mIndepVarsIndex)
    {
        QString srcName = mLayer->fields().field(index).name();
        QString name = srcName;
        fields.append(QgsField(name, QVariant::Double, QStringLiteral("double")));
    }
    if (hasHatMatrix)
    {
        fields.append(QgsField(QStringLiteral("y"), QVariant::Double, QStringLiteral("double")));
        fields.append(QgsField(QStringLiteral("yhat"), QVariant::Double, QStringLiteral("double")));
        fields.append(QgsField(QStringLiteral("residual"), QVariant::Double, QStringLiteral("double")));
    }
    mResultLayer->dataProvider()->addAttributes(fields.toList());
    mResultLayer->updateFields();

    mResultLayer->startEditing();
    if (hasHatMatrix)
    {
        int indepSize = mIndepVarsIndex.size() + 1;
        for (int f = 0; f < mFeatureList.size(); f++)
        {
            int curCol = 0;
            QgsFeature srcFeature = mFeatureList[f];
            QgsFeature feature(fields);
            feature.setGeometry(srcFeature.geometry());
            for (int a = 0; a < indepSize; a++)
            {
                int fieldIndex = a + curCol;
                QString attributeName = fields[fieldIndex].name();
                double attributeValue = mBetas(f, a);
                feature.setAttribute(attributeName, attributeValue);
            }
            curCol += indepSize;
            feature.setAttribute(fields[curCol++].name(), mY(f));
            feature.setAttribute(fields[curCol++].name(), mYHat(f));
            feature.setAttribute(fields[curCol++].name(), mResidual(f));
            mResultLayer->addFeature(feature);
        }
    }
    else
    {
        for (int f = 0; f < mFeatureList.size(); f++)
        {
            QgsFeature srcFeature = mFeatureList[f];
            QgsFeature feature(fields);
            feature.setGeometry(srcFeature.geometry());
            for (int a = 0; a < fields.size(); a++)
            {
                QString attributeName = fields[a].name();
                double attributeValue = mBetas(f, a);
                feature.setAttribute(attributeName, attributeValue);
            }
            mResultLayer->addFeature(feature);
        }
    }
    mResultLayer->commitChanges();
}

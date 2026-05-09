#include "gwmlocalcollinearitygwralgorithm.h"

#include <armadillo>

#ifdef ENABLE_OpenMP
#include <omp.h>
#endif

int GwmLocalCollinearityGWRAlgorithm::treeChildCount = 0;

using namespace arma;

GwmLocalCollinearityGWRAlgorithm::GwmLocalCollinearityGWRAlgorithm():GwmGeographicalWeightedRegressionAlgorithm(),
    mLCGWRCore(std::make_unique<gwm::GWRLocalCollinearity>())
{

}

double GwmLocalCollinearityGWRAlgorithm::lambda() const
{
    return mLambda;
}

void GwmLocalCollinearityGWRAlgorithm::setHasHatmatix(bool value)
{
    mHasHatmatix = value;
}

bool GwmLocalCollinearityGWRAlgorithm::hasHatmatix() const
{
    return mHasHatmatix;
}

double GwmLocalCollinearityGWRAlgorithm::cnThresh() const
{
    return mCnThresh;
}

void GwmLocalCollinearityGWRAlgorithm::setCanceled(bool canceled)
{
    // selector.setCanceled(canceled);
    // mSpatialWeight.distance()->setCanceled(canceled);
    return GwmTaskThread::setCanceled(canceled);
}

void GwmLocalCollinearityGWRAlgorithm::run()
{
    if(!checkCanceled())
    {
        // 点位初始化
        emit message(QString(tr("Setting data points")) + (hasRegressionLayer() ? tr(" and regression points") : "") + ".");
        initPoints();
        // 设置矩阵
        initXY(mX, mY, mDepVar, mIndepVars);
        mLCGWRCore->setCoords(mDataPoints);
        mLCGWRCore->setDependentVariable(mY);
        mLCGWRCore->setIndependentVariables(mX);
        mLCGWRCore->setSpatialWeight(mSpatialWeight);
        mLCGWRCore->setLambda(mLambda);
        mLCGWRCore->setCnThresh(mCnThresh);
        // mLCGWRCore->setHasHatMatrix(mHasHatMatrix);
        // mLCGWRCore->setBandwidthSelectionCriterion(mBandwidthSelectionCriterionType);
        mLCGWRCore->setIsAutoselectBandwidth(mIsAutoselectBandwidth);
    }

    if(!checkCanceled())
    {
        mat betas(mDataPoints.n_rows,mX.n_cols,fill::zeros);
        vec localcn(mDataPoints.n_rows,fill::zeros);
        vec locallambda(mDataPoints.n_rows,fill::zeros);
        vec hatrow(mDataPoints.n_rows,fill::zeros);
        //yhat赋值
        emit message("Regressoin...");
        // mBetas = regression(mX, mY);


        mLCGWRCore->setTelegram(std::make_unique<GwmTaskThreadTelegram>(this));
        mLCGWRCore->setParallelType(mParallelType);
        mLCGWRCore->setOmpThreadNum(mOmpThreadNum);
        qDebug() << "core parallelType =" << mLCGWRCore->parallelType();
        qDebug() << "core parallelAbility =" << mLCGWRCore->parallelAbility();

        QElapsedTimer timer;
        timer.start();

        mBetas = mLCGWRCore->fit();

        qint64 elapsed = timer.elapsed();
        qDebug() << "fit() time =" << elapsed << "ms";

        // std::cout << "mBetas = \n" << mBetas << std::endl;

        gwm::BandwidthWeight* bw = mLCGWRCore->spatialWeight().weight<gwm::BandwidthWeight>();
        mSpatialWeight.setWeight(bw);

        criterionList = mLCGWRCore->bandwidthSelectionCriterionList();
        QVector<QPair<double,double>> qlist;
        for (const auto &item : criterionList)
            qlist.append(qMakePair(item.first, item.second));
        QVariant data = QVariant::fromValue(qlist);
        emit plot(data, &GwmBandwidthSizeSelector::PlotBandwidthResult);

        //vec mYHat = fitted(mX,mBetas);
        vec mYHat = sum(mBetas % mX,1);
        vec mResidual = mY - mYHat;
        mDiagnostic0 = mLCGWRCore->diagnostic();
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
        emit tick(100,100);
        emit success();
    }
    if(checkCanceled())
    {
        return;
    }
}

bool GwmLocalCollinearityGWRAlgorithm::isAutoselectBandwidth() const
{
    return mIsAutoselectBandwidth;
}

void GwmLocalCollinearityGWRAlgorithm::setBandwidthSelectionCriterionType(const gwm::GWRBasic::BandwidthSelectionCriterionType &bandwidthSelectionCriterionType)
{
     mBandwidthSelectionCriterionType = bandwidthSelectionCriterionType;
     QMap<QPair<gwm::GWRBasic::BandwidthSelectionCriterionType, gwm::ParallelType>, BandwidthSelectCriterionFunction> mapper = {
         std::make_pair(qMakePair(gwm::GWRBasic::CV, gwm::ParallelType::SerialOnly), &GwmLocalCollinearityGWRAlgorithm::bandwidthSizeCriterionCVSerial),
    #ifdef ENABLE_OpenMP
         std::make_pair(qMakePair(gwm::GWRBasic::CV, gwm::ParallelType::OpenMP), &GwmLocalCollinearityGWRAlgorithm::bandwidthSizeCriterionCVOmp),
    #endif
         //std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, gwm::ParallelType::CUDA), &GwmLcrGWRTaskThread::bandwidthSizeCriterionCVCuda),
         //std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, gwm::ParallelType::SerialOnly), &GwmLcrGWRTaskThread::bandwidthSizeCriterionAICSerial),
         //std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, gwm::ParallelType::OpenMP), &GwmLcrGWRTaskThread::bandwidthSizeCriterionAICOmp),
         //std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, gwm::ParallelType::CUDA), &GwmLcrGWRTaskThread::bandwidthSizeCriterionAICCuda)
     };
     mBandwidthSelectCriterionFunction = mapper[qMakePair(bandwidthSelectionCriterionType, mParallelType)];
}

vec GwmLocalCollinearityGWRAlgorithm::ridgelm(const vec &w, double lambda)
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

mat GwmLocalCollinearityGWRAlgorithm::regression(const mat &x, const vec &y)
{
    return (this->*mRegressionFunction)(x, y);
}

void GwmLocalCollinearityGWRAlgorithm::createResultLayer(CreateResultLayerData data)
{
    QgsVectorLayer* srcLayer = mRegressionLayer ? mRegressionLayer : mDataLayer;
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    //避免图层名重复
    if(treeChildCount > 0)
    {
        layerName += ( QStringLiteral("_LCGWR") + "(" + QString::number(treeChildCount) + ")" );
    } else
    {
        layerName += QStringLiteral("_LCGWR");
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

double GwmLocalCollinearityGWRAlgorithm::bandwidthSizeCriterionCVSerial(GwmBandwidthWeight *weight)
{
    int mBandwidthCounter = 0;
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
    for (int i = 0; i < n && !checkCanceled(); i++)
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
        //S.print();
        //qDebug() << S(m);
        localcn(i)=S(0)/S(m-1);
        locallambda(i) = mLambda;
        if(mLambdaAdjust){
            if(localcn(i)>mCnThresh){
                locallambda(i) = (S(0) - mCnThresh*S(m-1)) / (mCnThresh - 1);
            }
        }
        betas.row(i) = trans( ridgelm(wgt,locallambda(i)) );
        mBandwidthCounter++;
        if (mBandwidthCounter < 10)
            emit tick(mBandwidthCounter * 10 + i * 5 / n, 100);
    }
    //yhat赋值
    //vec mYHat = fitted(mX,betas);
    vec yhat = sum(betas % mX,1);
    //计算residual
    vec residual = mY - yhat;
    //计算cv
    if(!checkCanceled())
    {
        double cv = sum(residual % residual);
        if (isfinite(cv))
        {
            QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
                    .arg(weight->adaptive() ? "Adaptive" : "Fixed")
                    .arg(weight->bandwidth())
                    .arg(cv);
            emit message(msg);
            return cv;
        }
        else return DBL_MAX;
    }
    else return DBL_MAX;
}
#ifdef ENABLE_OpenMP
double GwmLocalCollinearityGWRAlgorithm::bandwidthSizeCriterionCVOmp(GwmBandwidthWeight *weight)
{
    //vec cv_all(mOmpThreadNum, fill::zeros);
    //qDebug() << 'omp';
    //行数
    int n = mX.n_rows;
    //列数
    int m = mX.n_cols;
    //初始化矩阵
    mat betas = mat(n,m,fill::zeros);
    vec localcn(n,fill::zeros);
    vec locallambda(n,fill::zeros);
    //取mX不含第一列的部分
    mat mXnot1 = mX.cols(1, mX.n_cols - 1);
    //主循环
    int current = 0;
    const int selectorStep = static_cast<int>(selector.bandwidthCriterion().size());
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < n; i++)
    {
        if(!checkCanceled())
        {
            //int thread = omp_get_thread_num();
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
            //S.print();
            //qDebug() << S(m);
            localcn(i)=S(0)/S(m-1);
            locallambda(i) = mLambda;
            if(mLambdaAdjust){
                if(localcn(i)>mCnThresh){
                    locallambda(i) = (S(0) - mCnThresh*S(m-1)) / (mCnThresh - 1);
                }
            }
            betas.row(i) = trans( ridgelm(wgt,locallambda(i)) );
            // if(selector.counter<10)
            //     emit tick(selector.counter*10 + current * 10 / n, 100);
            if (selectorStep < 10)
                emit tick(selectorStep * 10 + current * 10 / n, 100);
            current++;
        }
    }
    //yhat赋值
    //vec mYHat = fitted(mX,betas);
    vec yhat = sum(betas % mX,1);
    //计算residual
    vec residual = mY - yhat;
    //计算cv
    if(!checkCanceled())
    {
        double cv = sum(residual % residual);
        if (isfinite(cv))
        {
            QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
                    .arg(weight->adaptive() ? "Adaptive" : "Fixed")
                    .arg(weight->bandwidth())
                    .arg(cv);
            emit message(msg);
            return cv;
        }
        else return DBL_MAX;
    }
    else return DBL_MAX;
}
#endif
mat GwmLocalCollinearityGWRAlgorithm::regressionSerial(const mat &x, const vec &y)
{
    mat betas(mDataPoints.n_rows,x.n_cols,fill::zeros);
    vec localcn(mDataPoints.n_rows,fill::zeros);
    vec locallambda(mDataPoints.n_rows,fill::zeros);
    vec hatrow(mDataPoints.n_rows,fill::zeros);
    for(int i=0;i<mDataPoints.n_rows && !checkCanceled();i++)
    {
        vec wi = mSpatialWeight.weightVector(i);
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
        locallambda(i) = mLambda;
        if(mLambdaAdjust){
            if(localcn(i)>mCnThresh){
                locallambda(i) = (S(0) - mCnThresh*S(x.n_cols-1)) / (mCnThresh - 1);
            }
        }
        betas.row(i) = trans( ridgelm(wi,locallambda(i)) );
        //如果没有给regressionpoint
        if(mHasHatmatix && !checkCanceled())
        {
            mat xm = x;
            mat xtw = trans(x % (wi * wispan1));
            mat xtwx = xtw * x;
            mat xtwxinv = inv(xtwx);
            rowvec hatrow = x1w.row(i) * xtwxinv * trans(x1w);
            this->mTrS += hatrow(i);
            this->mTrStS += sum(hatrow % hatrow);
        }
        emit tick(i, mDataPoints.n_rows);
    }
    return betas;
}
#ifdef ENABLE_OpenMP
mat GwmLocalCollinearityGWRAlgorithm::regressionOmp(const mat &x, const vec &y)
{
    mat betas(mDataPoints.n_rows,x.n_cols,fill::zeros);
    vec localcn(mDataPoints.n_rows,fill::zeros);
    vec locallambda(mDataPoints.n_rows,fill::zeros);
    vec hatrow(mDataPoints.n_rows,fill::zeros);

    mat shat_all(2, mOmpThreadNum, fill::zeros);
    int current = 0;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for(int i=0;i<mDataPoints.n_rows;i++)
    {
        if(!checkCanceled())
        {
            int thread = omp_get_thread_num();
            vec wi = mSpatialWeight.weightVector(i);
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
            locallambda(i) = mLambda;
            if(mLambdaAdjust){
                if(localcn(i)>mCnThresh){
                    locallambda(i) = (S(0) - mCnThresh*S(x.n_cols-1)) / (mCnThresh - 1);
                }
            }
            betas.row(i) = trans( ridgelm(wi,locallambda(i)) );
            //如果没有给regressionpoint
            if(mHasHatmatix && !checkCanceled())
            {
                mat xm = x;
                mat xtw = trans(x % (wi * wispan1));
                mat xtwx = xtw * x;
                mat xtwxinv = inv(xtwx);
                rowvec hatrow = x1w.row(i) * xtwxinv * trans(x1w);
                shat_all(0, thread) += hatrow(i);
                shat_all(1, thread) += sum(hatrow % hatrow);
                //this->mTrS += hatrow(i);
                //this->mTrStS += sum(hatrow % hatrow);
            }
            emit tick(current++, mDataPoints.n_rows);
        }
    }
    vec shat = sum(shat_all,1);
    this->mTrS = sum(shat.row(0));
    this->mTrStS = sum(shat.row(1));
    return betas;
}
#endif
void GwmLocalCollinearityGWRAlgorithm::setParallelType(const gwm::ParallelType &type)
{
    mParallelType = type;
    mLCGWRCore->setParallelType(type);
}

bool GwmLocalCollinearityGWRAlgorithm::lambdaAdjust() const
{
    return mLambdaAdjust;
}

void GwmLocalCollinearityGWRAlgorithm::setLambdaAdjust(bool lambdaAdjust)
{
    mLambdaAdjust = lambdaAdjust;
}

GwmDiagnostic GwmLocalCollinearityGWRAlgorithm::CalcDiagnostic(const mat& x, const vec& y, const mat& betas, const vec& shat)
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

bool GwmLocalCollinearityGWRAlgorithm::isValid()
{
    if (GwmGeographicalWeightedRegressionAlgorithm::isValid())
    {
        gwm::BandwidthWeight* bandwidth = static_cast<gwm::BandwidthWeight*>(mSpatialWeight.weight());

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

void GwmLocalCollinearityGWRAlgorithm::setCnThresh(double cnThresh)
{
    mCnThresh = cnThresh;
}

void GwmLocalCollinearityGWRAlgorithm::setLambda(double lambda)
{
    mLambda = lambda;
}

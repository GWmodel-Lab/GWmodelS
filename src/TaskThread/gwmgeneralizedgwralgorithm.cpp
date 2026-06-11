#include "gwmgeneralizedgwralgorithm.h"

//#include "GWmodel/GWmodel.h"
//#include "gwmggwrbandwidthselectionthread.h"
#include "GWmodel/gwmgeneralizedlinearmodel.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <exception>
#include <chrono>

#ifdef ENABLE_OpenMP
#include <omp.h>
#endif
using namespace std;
int GwmGeneralizedGWRAlgorithm::treeChildCount = 0;

GwmEnumValueNameMapper<GwmGeneralizedGWRAlgorithm::Family> GwmGeneralizedGWRAlgorithm::FamilyValueNameMapper = {
    std::make_pair(GwmGeneralizedGWRAlgorithm::Family::Poisson, "Poisson"),
    std::make_pair(GwmGeneralizedGWRAlgorithm::Family::Binomial, "Binomial")
};

QMap<QString, double> GwmGeneralizedGWRAlgorithm::TolUnitDict = {
    std::make_pair(QString("e -3"), 0.001),
    std::make_pair(QString("e -5"), 0.00001),
    std::make_pair(QString("e -7"), 0.0000001),
    std::make_pair(QString("e -10"), 0.0000000001)
};


GwmGeneralizedGWRAlgorithm::GwmGeneralizedGWRAlgorithm() : GwmGeographicalWeightedRegressionAlgorithm(),
    mGGWRCore(std::make_unique<gwm::GWRGeneralized>())
{
    // Set default values
    mFamily = Family::Poisson;
    mTol = 1e-5;
    mTolUnit = "e -5";
    mMaxiter = 20;
}

void GwmGeneralizedGWRAlgorithm::setCanceled(bool canceled)
{
    if (mGlm) mGlm->setCanceled(canceled);
    // mBandwidthSizeSelector.setCanceled(canceled);
    // mSpatialWeight.distance()->setCanceled(canceled);
    return GwmTaskThread::setCanceled(canceled);
}

void GwmGeneralizedGWRAlgorithm::run()
{
    if(!checkCanceled())
    {
        // 点位初始化
        emit message(QString(tr("Setting data points")) + (hasRegressionLayer() ? tr(" and regression points") : "") + ".");
        initPoints();

        mGGWRCore->setCoords(mDataPoints);

        // 如果有回归图层
        if(hasRegressionLayer())
        {
            mGGWRCore->setRegressionData(mRegressionPoints);
            mGGWRCore->setHasRegressionData(true);
        }
    }
    if(!checkCanceled())
    {
        // 初始化
        emit message(QString(tr("Setting X and Y.")));
        initXY(mX, mY, mDepVar, mIndepVars);

        mGGWRCore->setDependentVariable(mY);
        mGGWRCore->setIndependentVariables(mX);
        mGGWRCore->setSpatialWeight(mSpatialWeight);

    }
    if(!checkCanceled())
    {
        //设置GGWR参数
        emit message(tr("Setting parameters."));
        mGGWRCore->setFamily(convertFamily(mFamily));
        mGGWRCore->setTol(mTol);
        mGGWRCore->setMaxiter(mMaxiter);
        mGGWRCore->setHasHatMatrix(mHasHatMatrix);
        mGGWRCore->setBandwidthSelectionCriterionType(convertCriterionType(mBandwidthSelectionCriterionType));
        mGGWRCore->setIsAutoselectBandwidth(mIsAutoselectBandwidth);
        mGGWRCore->setParallelType(mParallelType);
        mGGWRCore->setOmpThreadNum(mOmpThreadNum);
        mGGWRCore->setTelegram(std::make_unique<GwmTaskThreadTelegram>(this));
    }

    // ========== 阶段4: 执行拟合 ==========

    criterionList = mGGWRCore->bandwidthSelectorCriterions();
    if (!checkCanceled() && !hasRegressionLayer())
    {
        if (mIsAutoselectBandwidth)
        {
            emit message(QString(tr("Automatically selecting bandwidth ...")));
            qDebug() << "mParallelType:" << static_cast<int>(mParallelType)
                     << "-> mGGWRCore parallelType:" << static_cast<int>(mGGWRCore->parallelType())
                     << "parallelAbility:" << static_cast<int>(mGGWRCore->parallelAbility());
            auto start_time = std::chrono::high_resolution_clock::now();
            mBetas = mGGWRCore->fit();
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            qDebug() << "fit() execution time (auto-select bandwidth):" << duration.count() << "ms, Threads:" << mOmpThreadNum;
            // if(mHasHatMatrix)
            // {
            //     arma::mat tempS;
            //     mBetas = mGGWRCore->fit(mX, mY, mBetasSE, mShat, mQDiag, tempS);
            // }
            // else
            // {
            //     mBetas = mGGWRCore->fit();
            // }


            const auto &coreW = mGGWRCore->spatialWeight().weight();
            if (coreW && !checkCanceled())
            {
                gwm::BandwidthWeight& bw = mGGWRCore->spatialWeight().weight<gwm::BandwidthWeight>();
                emit message(tr("bandwidth selected: %1").arg(bw.bandwidth()));

                mSpatialWeight.setWeight(bw);
                criterionList = mGGWRCore->bandwidthSelectorCriterions();

                // 绘图数据
                QVector<QPair<double,double>> qlist;
                for (const auto &item : criterionList)
                    qlist.append(qMakePair(item.first, item.second));
                QVariant data = QVariant::fromValue(qlist);
                emit plot(data, &GwmBandwidthSizeSelector::PlotBandwidthResult);
            }
        }
        else
        {
            emit message(QString(tr("Fitting GGWR model...")));
            qDebug() << "mParallelType:" << static_cast<int>(mParallelType)
                     << "-> mGGWRCore parallelType:" << static_cast<int>(mGGWRCore->parallelType())
                     << "parallelAbility:" << static_cast<int>(mGGWRCore->parallelAbility());
            auto start_time = std::chrono::high_resolution_clock::now();
            mBetas = mGGWRCore->fit();
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            qDebug() << "fit() execution time (no auto-select):" << duration.count() << "ms, Threads:" << mOmpThreadNum;
        }
    }
    else if (!checkCanceled() && hasRegressionLayer())
    {
        emit message(QString(tr("Fitting GGWR model...")));
        auto start_time = std::chrono::high_resolution_clock::now();
        mBetas = mGGWRCore->fit();
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        qDebug() << "fit() execution time (regression layer):" << duration.count() << "ms, Threads:" << mOmpThreadNum;
    }

    // 优选带宽
    // if (mIsAutoselectBandwidth && !checkCanceled())
    // {
    //     emit message(QString(tr("Automatically selecting bandwidth ...")));
    //     //emit tick(0, 0);
    //     if ((mSpatialWeight.distance()->type() == gwm::Distance::DistanceType::CRSDistance || mSpatialWeight.distance()->type() == gwm::Distance::DistanceType::MinkwoskiDistance) && !checkCanceled())
    //     {
    //         gwm::CRSDistance* d = static_cast<gwm::CRSDistance*>(mSpatialWeight.distance());
    //         d->makeParameter({ mDataPoints, mDataPoints });
    //     }
    //     gwm::BandwidthWeight* bandwidthWeight0 = mSpatialWeight.weight<gwm::BandwidthWeight>();
    //     mBandwidthSizeSelector.setBandwidth(bandwidthWeight0);
    //     double lower = bandwidthWeight0->adaptive() ? 20 : 0.0;
    //     double upper = bandwidthWeight0->adaptive() ? mDataPoints.n_rows : mSpatialWeight.distance()->maxDistance();
    //     mBandwidthSizeSelector.setLower(lower);
    //     mBandwidthSizeSelector.setUpper(upper);

    //     gwm::BandwidthWeight* bandwidthWeight = !checkCanceled() ? mBandwidthSizeSelector.optimize(mGGWRCore.get()) : nullptr;
    //     emit message(tr("Bandwidth Selected: %1").arg(bandwidthWeight->bandwidth()));

    //     // plot
    //     if (bandwidthWeight && !checkCanceled())
    //     {
    //         mSpatialWeight.setWeight(bandwidthWeight);
    //         // 绘图
    //         QVariant data = QVariant::fromValue(mBandwidthSizeSelector.bandwidthCriterion());
    //         emit plot(data, &GwmBandwidthSizeSelector::PlotBandwidthResult);
    //     }
    //     if ((mSpatialWeight.distance()->type() == gwm::Distance::DistanceType::CRSDistance || mSpatialWeight.distance()->type() == gwm::Distance::DistanceType::MinkwoskiDistance) && !checkCanceled())
    //     {
    //         gwm::CRSDistance* d = static_cast<gwm::CRSDistance*>(mSpatialWeight.distance());
    //         d->makeParameter({ mDataPoints, mDataPoints });

    //     }
    // }else if(!checkCanceled())
    // {

    // }

    // 老算法
    // int nVar = mX.n_cols;
    // int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
    // if(!checkCanceled()) mBetas = mat(nVar, nRp, fill::zeros);
    // if (mHasHatMatrix && !checkCanceled())
    // {
    //     mBetasSE = mat( nVar,nDp, fill::zeros);
    //     mShat = vec(2,fill::zeros);
    // }

    // emit message(tr("Calculating Distance Matrix..."));
    // mWtMat1 = mat(nDp,nDp,fill::zeros);
    // if(!checkCanceled()){
    //     mWtMat2 = mat(nRp,nDp,fill::zeros);
    // }
    // if(mRegressionLayer && !checkCanceled()){
    //     for(int i = 0; i < nRp && !checkCanceled(); i++){
    //         vec weight = mSpatialWeight.weightVector(i);
    //         mWtMat2.col(i) = weight;
    //         emit tick(i, nRp);
    //     }
    //     if ((mSpatialWeight.distance()->type() == gwm::Distance::DistanceType::CRSDistance || mSpatialWeight.distance()->type() == gwm::Distance::DistanceType::MinkwoskiDistance) && !checkCanceled())
    //     {
    //         gwm::CRSDistance* d = static_cast<gwm::CRSDistance*>(mSpatialWeight.distance());
    //         d->makeParameter({ mDataPoints, mDataPoints });
    //     }
    //     for(int i = 0; i < nDp && !checkCanceled(); i++){
    //         vec weight = mSpatialWeight.weightVector(i);
    //         mWtMat1.col(i) = weight;
    //         emit tick(i, nDp);
    //     }
    // }
    // else{
    //     for(int i = 0; i < nRp && !checkCanceled(); i++){
    //         vec weight = mSpatialWeight.weightVector(i);
    //         mWtMat2.col(i) = weight;
    //         emit tick(i, nRp);
    //     }
    //     mWtMat1 = mWtMat2;
    // }

    // bool isAllCorrect = true;
    // if(!checkCanceled())
    // {
    //     CalGLMModel(mX,mY);
    //     mBetas = (this->*mGGWRRegressionFunction)(mX,mY);
    // }
    // if(checkCanceled())
    // {
    //     return;
    // }

    // if(mHasHatMatrix && !checkCanceled()){
    //     if(mFamily == Family::Poisson){
    //         mat betasTV = mBetas / mBetasSE;
    //         mBetas = trans(mBetas);
    //         mBetasSE = trans(mBetasSE);
    //         betasTV = trans(betasTV);
    //         double trS = mShat(0);
    //         double trStS = mShat(1);

    //         mat yhat = exp(Fitted(mX,mBetas));
    //         mat res = mY - yhat;

    //         //计算诊断信息
    //         double AIC = mGwDev + 2 * trS;
    //         double AICc = AIC + 2*trS*(trS+1)/(nDp-trS-1);
    //         double R2 = 1 - mGwDev/(mGLMDiagnostic.NullDev);  // pseudo.R2 <- 1 - gw.dev/null.dev
    //         vec vDiags(4);
    //         vDiags(0) = AIC;
    //         vDiags(1) = AICc;
    //         vDiags(2) = mGwDev;
    //         vDiags(3) = R2;
    //         mDiagnostic = GwmGGWRDiagnostic(vDiags);

    //         mResultList.push_back(qMakePair(QString("%1"), mBetas));
    //         mResultList.push_back(qMakePair(QString("y"), mY));
    //         mResultList.push_back(qMakePair(QString("yhat"), yhat));
    //         mResultList.push_back(qMakePair(QString("residual"), res));
    //         mResultList.push_back(qMakePair(QString("%1_SE"), mBetasSE));
    //         mResultList.push_back(qMakePair(QString("%1_TV"), betasTV));
    //     }
    //     else{
    //         mat n = vec(mY.n_rows,fill::ones);
    //         mBetas = trans(mBetas);

    //         double trS = mShat(0);
    //         double trStS = mShat(1);

    //         vec yhat = Fitted(mX,mBetas);
    //         yhat = exp(yhat)/(1+exp(yhat));

    //         vec res = mY - yhat;
    //         vec Dev = log(1/( (mY-n+yhat) % (mY-n+yhat) ) );
    //         double gwDev = sum(Dev);
    //         vec residual2 = res % res;
    //         double rss = sum(residual2);
    //         for(int i = 0; i < nDp && !checkCanceled(); i++){
    //             mBetasSE.col(i) = sqrt(mBetasSE.col(i));
    // //            mBetasTV.col(i) = mBetas.col(i) / mBetasSE.col(i);
    //         }
    //         mBetasSE = trans(mBetasSE);
    //         mat betasTV = mBetas / mBetasSE;

    //         double AIC = gwDev + 2 * trS;
    //         double AICc = AIC + 2*trS*(trS+1)/(nDp-trS-1);
    //         double R2 = 1 - gwDev/(mGLMDiagnostic.NullDev);  // pseudo.R2 <- 1 - gw.dev/null.dev
    //         vec vDiags(4);
    //         vDiags(0) = AIC;
    //         vDiags(1) = AICc;
    //         vDiags(2) = gwDev;
    //         vDiags(3) = R2;
    //         mDiagnostic = GwmGGWRDiagnostic(vDiags);

    //         mResultList.push_back(qMakePair(QString("%1"), mBetas));
    //         mResultList.push_back(qMakePair(QString("y"), mY));
    //         mResultList.push_back(qMakePair(QString("yhat"), yhat));
    //         mResultList.push_back(qMakePair(QString("residual"), res));
    //         mResultList.push_back(qMakePair(QString("%1_SE"), mBetasSE));
    //         mResultList.push_back(qMakePair(QString("%1_TV"), betasTV));
    //     }
    // }
    // else{
    //     mBetas = trans(mBetas);
    //     mResultList.push_back(qMakePair(QString("%1"), mBetas));
    // }



    emit message(tr("get diagnostic info from new function..."));
    if (mHasHatMatrix && !checkCanceled())
    {
        //uword nDp = mDataPoints.n_rows;

        // 从内核库获取诊断信息
        auto kernelDiag = mGGWRCore->getDiagnostic();
        auto kernelGLMDiag = mGGWRCore->getGLMDiagnostic();
        mDiagnostic = convertDiagnostic(kernelDiag);
        mGLMDiagnostic = convertGLMDiagnostic(kernelGLMDiag);

        // 获取Hat矩阵相关数据
        mShat = mGGWRCore->sHat();
        mBetas = mGGWRCore->betas();
        mBetasSE = mGGWRCore->betasSE();
        mS = mGGWRCore->s();

        // ========== 仅保留 QDiag 计算，mBetasSE/mShat/mS 直接使用内核结果 ==========
        uword nDp = mDataPoints.n_rows;
        mQDiag = vec(nDp, fill::zeros);

        // 优先使用完整 S 直接重建 QDiag
        const bool hasFullS = (mS.n_rows == nDp && mS.n_cols == nDp);
        if (hasFullS)
        {
            mQDiag = rebuildQDiagFromS(mS);
        }
        else
        {
            // 无完整 S 时，仅为 QDiag 重建所需，回推 IRLS 权重并逐点计算
            vec eta = sum(mBetas % mX, 1);
            vec mu;
            if (mFamily == Family::Poisson)
            {
                mu = exp(eta);
                mWt2 = mu;
                myAdj = eta + (mY - mu) / mu;
            }
            else // Binomial
            {
                mu = exp(eta) / (1.0 + exp(eta));
                mWt2 = mu % (1.0 - mu);
                myAdj = eta + (mY - mu) / (mu % (1.0 - mu));
            }

            mWtMat2 = mGGWRCore->getWtMat2();

            for (uword i = 0; i < nDp && !checkCanceled(); i++)
            {
                try
                {
                    vec wi = mWtMat2.col(i);
                    mat ci, s_ri;
                    gwRegHatmatrix(mX, myAdj, wi % mWt2, i, ci, s_ri);

                    vec p = -trans(s_ri);
                    p(i) += 1.0;
                    mQDiag += p % p;
                }
                catch (const std::exception& e)
                {
                    emit error(e.what());
                }
            }
        }

        // 计算拟合值和残差
        vec yhat;
        vec res;
        mat betasTV;
        vec eta = sum(mBetas % mX, 1);

        if (mFamily == Family::Poisson)
        {
            yhat = exp(eta);
            res = mY - yhat;
        }
        else // Binomial
        {
            yhat = exp(eta) / (1.0 + exp(eta));
            res = mY - yhat;
        }

        betasTV = mBetas / mBetasSE;

        // 创建结果图层数据
        mResultList.clear();
        mResultList.push_back(qMakePair(QString("%1"), mBetas));
        mResultList.push_back(qMakePair(QString("y"), mY));
        mResultList.push_back(qMakePair(QString("yhat"), yhat));
        mResultList.push_back(qMakePair(QString("residual"), res));
        mResultList.push_back(qMakePair(QString("%1_SE"), mBetasSE));
        mResultList.push_back(qMakePair(QString("%1_TV"), betasTV));

        emit message(tr("update diagnostic success."));
    }
    else if (!checkCanceled())
    {
        // 没有Hat矩阵的情况
        mResultList.clear();
        mResultList.push_back(qMakePair(QString("%1"), mBetas));
    }


    // 添加 F-test 计算（在 createResultLayer 之前）
    if (mHasHatMatrix && mHasFTest && !checkCanceled())
    {
        uword nDp = mDataPoints.n_rows;
        double trQtQ = DBL_MAX;

        // 计算 trQtQ
        bool isStoreS = (nDp <= 8192);
        if (isStoreS && mS.n_rows == nDp)
        {
            // 如果存储了完整的 S 矩阵，直接计算 trQtQ
            mat EmS = eye(nDp, nDp) - mS;
            mat Q = trans(EmS) * EmS;
            trQtQ = sum(diagvec(trans(Q) * Q));
        }
        else
        {
            // 对于大数据集，需要迭代计算 trQtQ
            // 这里简化处理：使用近似值或跳过 F-test
            // 如果需要完整实现，可以参考 BasicGWR 的 calcTrQtQ 方法
            trQtQ = DBL_MAX;  // 暂时设为最大值，表示无法计算
        }

        if (trQtQ < DBL_MAX && !checkCanceled())
        {
            FTestParameters fTestParams;
            fTestParams.nDp = nDp;
            fTestParams.nVar = mX.n_cols;
            fTestParams.trS = mShat(0);
            fTestParams.trStS = mShat(1);
            fTestParams.trQ = sum(mQDiag);
            fTestParams.trQtQ = trQtQ;

            // 计算 GWR RSS
            vec yhat;
            if (mFamily == Family::Poisson)
            {
                yhat = exp(sum(mBetas % mX, 1));
            }
            else // Binomial
            {
                vec nu = sum(mBetas % mX, 1);
                yhat = exp(nu) / (1 + exp(nu));
            }
            vec res = mY - yhat;
            fTestParams.gwrRSS = sum(res % res);

            fTest(fTestParams);
        }
        else if (mHasFTest)
        {
            // 如果无法计算 trQtQ，输出警告信息
            emit message(tr("F-test cannot be calculated: trQtQ computation failed (data too large or S matrix not available)"));
        }
    }

    // Create Result Layer
    if(!checkCanceled())
    {
        // if (isAllCorrect)
        // {
        createResultLayer(mResultList,QStringLiteral("_GGWR"));
        // }
        emit tick(100,100);
        emit success();
    }
}

void GwmGeneralizedGWRAlgorithm::CalGLMModel(const mat &x, const vec &y)
{
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
    int nVar = x.n_cols;
    emit message(tr("Calibrating GLM model..."));
    mGlm = new GwmGeneralizedLinearModel();
    mGlm->setX(x);
    mGlm->setY(y);
    mGlm->setFamily(mFamily);
    mGlm->fit();
    double nulldev = mGlm->nullDev();
    double dev = mGlm->dev();
    double pseudor2 = 1- dev/nulldev;
    double aic = dev + 2 * nVar;
    double aicc = aic + 2 * nVar * (nVar + 1)/(nDp - nVar - 1);
    vec vGLMDiags(5);
    vGLMDiags(0) = aic;
    vGLMDiags(1) = aicc;
    vGLMDiags(2) = nulldev;
    vGLMDiags(3) = dev;
    vGLMDiags(4) = pseudor2;
    mGLMDiagnostic = GwmGLMDiagnostic(vGLMDiags);
}

mat GwmGeneralizedGWRAlgorithm::regressionPoissonSerial(const mat &x, const vec &y)
{
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
    int nVar = x.n_cols;
    mat betas = mat(nVar, nRp, fill::zeros);

    mat mu = (this->*mCalWtFunction)(x,y,mWtMat1);
    mGwDev = 0.0;
    for(int i = 0; i < nDp && !checkCanceled(); i++){
        if(y[i] != 0){
            mGwDev = mGwDev +  2*(y[i]*(log(y[i]/mu[i])-1)+mu[i]);
        }
        else{
            mGwDev = mGwDev + 2* mu[i];
        }
    }
    emit message(tr("Calibrating GGWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;
    bool isStoreS = (nDp <= 8192);
    mat ci, s_ri, S(isStoreS ? nDp : 1, nDp, fill::zeros);
    if(mHasHatMatrix && !checkCanceled()){
        for(int i = 0; i < nDp && !checkCanceled(); i++){
            try{
                vec wi = mWtMat2.col(i);
                vec gwsi = gwRegHatmatrix(x, myAdj, wi % mWt2, i, ci, s_ri);
                betas.col(i) = gwsi;
                mat invwt2 = 1.0 / mWt2;
                S.row(isStoreS ? i : 0) = s_ri;
                mat temp = mat(ci.n_rows,ci.n_cols);
                for(int j = 0; j < ci.n_rows && !checkCanceled(); j++){
                    temp.row(j) = ci.row(j) % trans(invwt2);
                }
                mBetasSE.col(i) = diag(temp * trans(ci));

                mShat(0) += s_ri(0, i);
                mShat(1) += det(s_ri * trans(s_ri));

                mBetasSE.col(i) = sqrt(mBetasSE.col(i));
    //            mBetasTV.col(i) = mBetas.col(i) / mBetasSE.col(i);

                emit tick(i, nDp);
            }
            catch (std::exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }

        }
    }
    else{
        for(int i = 0; i < nRp && !checkCanceled(); i++){
            try{
                vec wi = mWtMat2.col(i);
                vec gwsi = gwReg(x, myAdj, wi * mWt2, i);
                betas.col(i) = gwsi;
                emit tick(i,nRp);
            }
            catch (std::exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
    }
    return betas;
}
#ifdef ENABLE_OpenMP
mat GwmGeneralizedGWRAlgorithm::regressionPoissonOmp(const mat &x, const vec &y)
{
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
    int nVar = x.n_cols;
    mat betas = mat(nVar, nRp, fill::zeros);

    mat mu = (this->*mCalWtFunction)(x,y,mWtMat1);

    mGwDev = 0.0;
    for(int i = 0; i < nDp && !checkCanceled(); i++){
        if(y[i] != 0){
            mGwDev = mGwDev +  2*(y[i]*(log(y[i]/mu[i])-1)+mu[i]);
        }
        else{
            mGwDev = mGwDev + 2* mu[i];
        }
    }
    emit message(tr("Calibrating GGWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;
    bool isStoreS = (nDp <= 8192);
    mat S(isStoreS ? nDp : 1, nDp, fill::zeros);
    int current = 0;
    if(mHasHatMatrix && !checkCanceled()){
        mat shat = mat(2,mOmpThreadNum,fill::zeros);       
#pragma omp parallel for num_threads(mOmpThreadNum)
        for(int i = 0; i < nDp; i++){
            mat ci,s_ri;
            if(!checkCanceled())
            {
                try{
                    int thread = omp_get_thread_num();
                    vec wi = mWtMat2.col(i);
                    vec gwsi = gwRegHatmatrix(x, myAdj, wi % mWt2, i, ci, s_ri);
                    betas.col(i) = gwsi;
                    mat invwt2 = 1.0 / mWt2;
                    S.row(isStoreS ? i : 0) = s_ri;
                    mat temp = mat(ci.n_rows,ci.n_cols);
                    for(int j = 0; j < ci.n_rows && !checkCanceled(); j++){
                        temp.row(j) = ci.row(j) % trans(invwt2);
                    }
                    mBetasSE.col(i) = diag(temp * trans(ci));

                    shat(0,thread) += s_ri(0, i);
                    shat(1,thread) += det(s_ri * trans(s_ri));

                    mBetasSE.col(i) = sqrt(mBetasSE.col(i));
        //            mBetasTV.col(i) = mBetas.col(i) / mBetasSE.col(i);

                    emit tick(current++, nDp);
                }
                catch (std::exception e) {
                    isAllCorrect = false;
                    emit error(e.what());
                }
            }
        }
        mShat(0) = sum(trans(shat.row(0)));
        mShat(1) = sum(trans(shat.row(1)));
    }
    else{
#pragma omp parallel for num_threads(mOmpThreadNum)
        for(int i = 0; i < nRp; i++){
            if(!checkCanceled())
            {
                try{
                    vec wi = mWtMat2.col(i);
                    vec gwsi = gwReg(x, myAdj, wi * mWt2, i);
                    betas.col(i) = gwsi;
                    emit tick(current++, nRp);
                }
                catch (std::exception e) {
                    isAllCorrect = false;
                    emit error(e.what());
                }
            }
        }
    }
    return betas;
}
#endif
#ifdef ENABLE_OpenMP
mat GwmGeneralizedGWRAlgorithm::regressionBinomialOmp(const mat &x, const vec &y)
{
    int nVar = x.n_cols;
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
//    mat S = mat(nDp,nDp);
//    mat n = vec(mY.n_rows,fill::ones);
    mat betas = mat(nVar, nRp, fill::zeros);

    vec mu = (this->*mCalWtFunction)(x,y,mWtMat1);
    emit message(tr("Calibrating GGWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;

    bool isStoreS = (nDp <= 8192);
    mat S(isStoreS ? nDp : 1, nDp, fill::zeros);
//    mat S = mat(uword(0), uword(0));
    int current = 0;
    if(mHasHatMatrix){
        mat shat = mat(mOmpThreadNum,2,fill::zeros);
#pragma omp parallel for num_threads(mOmpThreadNum)
        for(int i = 0; i < nDp; i++){
            mat ci,s_ri;
            if(!checkCanceled())
            {
                try {
                    int thread = omp_get_thread_num();
                    vec wi = mWtMat1.col(i);
                    vec gwsi = gwRegHatmatrix(x, myAdj, wi % mWt2, i, ci, s_ri);
                    betas.col(i) = gwsi;
                    mat invwt2 = 1.0 / mWt2;
                    S.row(isStoreS ? i : 0) = s_ri;
                    mat temp = mat(ci.n_rows,ci.n_cols);
                    for(int j = 0; j < ci.n_rows && !checkCanceled(); j++){
                        temp.row(j) = ci.row(j) % trans(invwt2);
                    }
                    mBetasSE.col(i) = diag(temp * trans(ci));
                    shat(thread,0) += s_ri(0, i);
                    shat(thread,1) += det(s_ri * trans(s_ri));
                    emit tick(current++, nDp);
                }
                catch (std::exception e) {
                    isAllCorrect = false;
                    emit error(e.what());
                }
            }
        }
        mShat(0) = sum(shat.col(0));
        mShat(1) = sum(shat.col(1));
    }
    else{
#pragma omp parallel for num_threads(mOmpThreadNum)
        for(int i = 0; i < nRp; i++){
            if(!checkCanceled())
            {
                try {
                    vec wi = mWtMat2.col(i);
                    vec gwsi = gwReg(x, myAdj, wi * mWt2, i);
                    mBetas.col(i) = gwsi;
                    emit tick(current++, nRp);
                }
                catch (std::exception e) {
                    isAllCorrect = false;
                    emit error(e.what());
                }
            }
        }
    }

    return betas;
}
#endif
mat GwmGeneralizedGWRAlgorithm::regressionBinomialSerial(const mat &x, const vec &y)
{
    int nVar = x.n_cols;
    int nDp = mDataLayer->featureCount(), nRp = mRegressionLayer ? mRegressionLayer->featureCount() : nDp;
//    mat S = mat(nDp,nDp);
//    mat n = vec(mY.n_rows,fill::ones);
    mat betas = mat(nVar, nRp, fill::zeros);

    vec mu = (this->*mCalWtFunction)(x,y,mWtMat1);
    emit message(tr("Calibrating GGWR model..."));
    emit tick(0, nDp);
    bool isAllCorrect = true;

    bool isStoreS = (nDp <= 8192);
    mat ci;
    mat s_ri, S(isStoreS ? nDp : 1, nDp, fill::zeros);
//    mat S = mat(uword(0), uword(0));
    if(mHasHatMatrix && !checkCanceled()){
        for(int i = 0; i < nDp && !checkCanceled(); i++){
            try {
                vec wi = mWtMat1.col(i);
                vec gwsi = gwRegHatmatrix(x, myAdj, wi % mWt2, i, ci, s_ri);
                betas.col(i) = gwsi;
                mat invwt2 = 1.0 / mWt2;
                S.row(isStoreS ? i : 0) = s_ri;
                mat temp = mat(ci.n_rows,ci.n_cols);
                for(int j = 0; j < ci.n_rows && !checkCanceled(); j++){
                    temp.row(j) = ci.row(j) % trans(invwt2);
                }
                mBetasSE.col(i) = diag(temp * trans(ci));
                mShat(0) += s_ri(0, i);
                mShat(1) += det(s_ri * trans(s_ri));
                emit tick(i, nDp);
            }
            catch (std::exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }

    }
    else{
        for(int i = 0; i < nRp && !checkCanceled(); i++){
            try {
                vec wi = mWtMat2.col(i);
                vec gwsi = gwReg(x, myAdj, wi * mWt2, i);
                mBetas.col(i) = gwsi;
                emit tick(i, nRp);
            }
            catch (std::exception e) {
                isAllCorrect = false;
                emit error(e.what());
            }
        }
    }
    return betas;
}

double GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionCVSerial(GwmBandwidthWeight *bandwidthWeight)
{
    int mBandwidthCounter = 0;
    int n = mDataPoints.n_rows;
    vec cv = vec(n);
    mat wt = mat(n,n);
    for (int i = 0; i < n && !checkCanceled(); i++)
    {
        vec d = mSpatialWeight.distance()->distance(i);
        vec w = bandwidthWeight->weight(d);
        w.row(i) = 0;
        wt.col(i) = w;
        mBandwidthCounter++;
        if (mBandwidthCounter < 10)
            emit tick(mBandwidthCounter * 10 + i * 5 / n, 100);
    }
    if (!checkCanceled()) (this->*mCalWtFunction)(mX,mY,wt);
    for (int i = 0; i < n && !checkCanceled(); i++){
        mat wi = wt.col(i) % mWt2;
        vec gwsi = gwReg(mX, myAdj, wi, i);
        mat yhatnoi = mX.row(i) * gwsi;
        if(mFamily == GwmGeneralizedGWRAlgorithm::Family::Poisson){
            cv.row(i) = mY.row(i) - exp(yhatnoi);
        }
        else{
            cv.row(i) = mY.row(i) - exp(yhatnoi)/(1+exp(yhatnoi));
        }
        mBandwidthCounter++;
        if (mBandwidthCounter < 10)
            emit tick(mBandwidthCounter * 10 + i * 5 / n, 100);
    }
    vec cvsquare = trans(cv) * cv ;
    double res = sum(cvsquare);
//    this->mBwScore.insert(bw,res);
    if(!checkCanceled())
    {
        QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
                .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
                .arg(bandwidthWeight->bandwidth())
                .arg(res);
        emit message(msg);
        return res;
    }   
    else return DBL_MAX;

}
#ifdef ENABLE_OpenMP
double GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionCVOmp(GwmBandwidthWeight *bandwidthWeight)
{
    int n = mDataPoints.n_rows;
    vec cv = vec(n);
    mat wt = mat(n,n);
    // int current1 = 0, current2 = 0;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < n; i++)
    {
        if(!checkCanceled())
        {
            vec d = mSpatialWeight.distance()->distance(i);
            vec w = bandwidthWeight->weight(d);
            w.row(i) = 0;
            wt.col(i) = w;
            // if(mBandwidthSizeSelector.counter<10)
            //     emit tick(mBandwidthSizeSelector.counter*10 + current1 * 5 / n, 100);
            if (i % std::max(1, n / 10) == 0)
                emit tick(i * 100 / n, 100);
            // current1++;
        }
    }
    if (!checkCanceled()) (this->*mCalWtFunction)(mX,mY,wt);
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < n; i++){
        if(!checkCanceled())
        {
            mat wi = wt.col(i) % mWt2;
            vec gwsi = gwReg(mX, myAdj, wi, i);
            mat yhatnoi = mX.row(i) * gwsi;
            if(mFamily == GwmGeneralizedGWRAlgorithm::Family::Poisson){
                cv.row(i) = mY.row(i) - exp(yhatnoi);
            }
            else{
                cv.row(i) = mY.row(i) - exp(yhatnoi)/(1+exp(yhatnoi));
            }
            // if(mBandwidthSizeSelector.counter<10)
            //    emit tick(mBandwidthSizeSelector.counter*10 + current2 * 5 / n + 5, 100);
            if (i % std::max(1, n / 10) == 0)
                emit tick(i * 100 / n, 100);
            // current2++;
        }
    }
    vec cvsquare = trans(cv) * cv ;
    double res = sum(cvsquare);
//    this->mBwScore.insert(bw,res);
    if(!checkCanceled())
    {
        QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
                .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
                .arg(bandwidthWeight->bandwidth())
                .arg(res);
        emit message(msg);
        return res;
    }
    else return DBL_MAX;
}
#endif
double GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionAICSerial(GwmBandwidthWeight *bandwidthWeight)
{
    int mBandwidthCounter = 0;
    int n = mDataPoints.n_rows;
    vec cv = vec(n);
    mat S = mat(n,n);
    mat wt = mat(n,n);    
    for (int i = 0; i < n && !checkCanceled(); i++)
    {
        vec d = mSpatialWeight.distance()->distance(i);
        vec w = bandwidthWeight->weight(d);
        wt.col(i) = w;
        mBandwidthCounter++;
        if (mBandwidthCounter < 10)
            emit tick(mBandwidthCounter * 10 + i * 5 / n, 100);
    }
    if(!checkCanceled()) (this->*mCalWtFunction)(mX,mY,wt);
    vec trS = vec(1,fill::zeros);
    for (int i = 0; i < n && !checkCanceled(); i++){
        vec wi = wt.col(i) % mWt2;
        mat Ci = CiMat(mX,wi);
        S.row(i) = mX.row(i) * Ci;
        trS(0) += S(i,i);
        mBandwidthCounter++;
        if (mBandwidthCounter < 10)
            emit tick(mBandwidthCounter * 10 + i * 5 / n, 100);
    }
    double AICc;
    if(!checkCanceled())
    {
        if(S.is_finite()){
            double trs = double(trS(0));
            AICc = -2*mLLik + 2*trs + 2*trs*(trs+1)/(n-trs-1);
        }
        else{
            AICc = qInf();
        }
    }

    if(!checkCanceled())
    {
        QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
                .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
                .arg(bandwidthWeight->bandwidth())
                .arg(AICc);
        emit message(msg);
        return AICc;
    }
    else return DBL_MAX;
}
#ifdef ENABLE_OpenMP
double GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionAICOmp(GwmBandwidthWeight *bandwidthWeight)
{
    int n = mDataPoints.n_rows;
    vec cv = vec(n);
    mat S = mat(n,n);
    mat wt = mat(n,n);
    // int current1 = 0, current2 = 0;
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < n; i++)
    {
        if(!checkCanceled())
        {
            vec d = mSpatialWeight.distance()->distance(i);
            vec w = bandwidthWeight->weight(d);
            wt.col(i) = w;
            // if(mBandwidthSizeSelector.counter<10)
            //    emit tick(mBandwidthSizeSelector.counter*10 + current1 * 5 / n, 100);
            if (i % std::max(1, n / 10) == 0)
                emit tick(i * 100 / n, 100);
            // current1++;
        }
    }
    if (!checkCanceled())  (this->*mCalWtFunction)(mX,mY,wt);
    vec trS = vec(mOmpThreadNum,fill::zeros);
#pragma omp parallel for num_threads(mOmpThreadNum)
    for (int i = 0; i < n; i++){
        if(!checkCanceled())
        {
            int thread = omp_get_thread_num();
            vec wi = wt.col(i) % mWt2;
            mat Ci = CiMat(mX,wi);
            S.row(i) = mX.row(i) * Ci;
            trS(thread) += S(i,i);
            // if(mBandwidthSizeSelector.counter<10)
            //     emit tick(mBandwidthSizeSelector.counter*10 + current2 * 5 / n + 5, 100);
            if (i % std::max(1, n / 10) == 0)
                emit tick(i * 100 / n, 100);
            // current2++;
        }
    }
    double AICc;
    if(!checkCanceled())
    {
        if(S.is_finite()){
            double trs = double(sum(trS));
            AICc = -2*mLLik + 2*trs + 2*trs*(trs+1)/(n-trs-1);
        }
        else{
            AICc = qInf();
        }
    }

    if(!checkCanceled())
    {
        QString msg = QString(tr("%1 bandwidth: %2 (CV Score: %3)"))
                .arg(bandwidthWeight->adaptive() ? "Adaptive" : "Fixed")
                .arg(bandwidthWeight->bandwidth())
                .arg(AICc);
        emit message(msg);
        return AICc;
    }
    else return DBL_MAX;
}
#endif
mat GwmGeneralizedGWRAlgorithm::PoissonWtSerial(const mat &x, const vec &y, mat wt){
    int varn = x.n_cols;
    int dpn = x.n_rows;
    mat betas = mat(varn, dpn, fill::zeros);
    mat S = mat(dpn,dpn);
    int itCount = 0;
    double oldLLik = 0.0;
    vec mu = y + 0.1;
    vec nu = log(mu);
    vec cv = vec(dpn);
    mWt2 = ones(dpn);
    mLLik = 0;

    while(!checkCanceled()){
        myAdj = nu + (y - mu)/mu;
        for (int i = 0; i < dpn && !checkCanceled(); i++)
        {
            vec wi = wt.col(i);
            vec gwsi = gwReg(x, myAdj, wi % mWt2, i);
            betas.col(i) = gwsi;
        }
        mat betas1 = trans(betas);
        nu = Fitted(x,betas1);
        mu = exp(nu);
        oldLLik = mLLik;
        vec lliktemp = dpois(y,mu);
        mLLik = sum(lliktemp);
        if (abs((oldLLik - mLLik)/mLLik) < mTol)
            break;
        mWt2 = mu;
        itCount++;
        if (itCount == mMaxiter)
            break;
    }
//    return cv;
    return mu;
}
#ifdef ENABLE_OpenMP
mat GwmGeneralizedGWRAlgorithm::PoissonWtOmp(const mat &x, const vec &y, mat wt){
    int varn = x.n_cols;
    int dpn = x.n_rows;
    mat betas = mat(varn, dpn, fill::zeros);
    mat S = mat(dpn,dpn);
    int itCount = 0;
    double oldLLik = 0.0;
    vec mu = y + 0.1;
    vec nu = log(mu);
    vec cv = vec(dpn);
    mWt2 = ones(dpn);
    mLLik = 0;
    while(!checkCanceled()){
        myAdj = nu + (y - mu)/mu;
        for (int i = 0; i < dpn && !checkCanceled(); i++)
        {
            vec wi = wt.col(i);
            vec gwsi = gwReg(x, myAdj, wi % mWt2, i);
            betas.col(i) = gwsi;
        }
        mat betas1 = trans(betas);
        nu = Fitted(x,betas1);
        mu = exp(nu);
        oldLLik = mLLik;
        vec lliktemp = dpois(y,mu);
        mLLik = sum(lliktemp);
        if (abs((oldLLik - mLLik)/mLLik) < mTol)
            break;
        mWt2 = mu;
        itCount++;
        if (itCount == mMaxiter)
            break;
    }
//    return cv;
    return mu;
}
#endif
mat GwmGeneralizedGWRAlgorithm::BinomialWtSerial(const mat &x, const vec &y, mat wt){
    int varn = x.n_cols;
    int dpn = x.n_rows;
    mat betas = mat(varn, dpn, fill::zeros);
    mat S = mat(dpn,dpn);
    mat n = vec(y.n_rows,fill::ones);
    int itCount = 0;
//    double lLik = 0.0;
    double oldLLik = 0.0;
    vec mu = vec(dpn,fill::ones) * 0.5;
    vec nu = vec(dpn,fill::zeros);
//    vec cv = vec(dpn);
    mWt2 = ones(dpn);
    mLLik = 0;
    while(!checkCanceled()){
        //计算公式有调整
        myAdj = nu + (y - mu)/(mu % (1 - mu));
        for (int i = 0; i < dpn && !checkCanceled(); i++)
        {
            vec wi = wt.col(i);
            vec gwsi = gwReg(x, myAdj, wi % mWt2, i);
            betas.col(i) = gwsi;
        }
        mat betas1 = trans(betas);
        nu = Fitted(x,betas1);
        mu = exp(nu)/(1 + exp(nu));
        oldLLik = mLLik;
        mLLik = sum(lchoose(n,y) + (n-y)%log(1 - mu/n) + y%log(mu/n));
        if (abs((oldLLik - mLLik)/mLLik) < mTol)
            break;
        mWt2 = n%mu%(1-mu);
        itCount++;
        if (itCount == mMaxiter)
            break;
    }
    return mu;
}
#ifdef ENABLE_OpenMP
mat GwmGeneralizedGWRAlgorithm::BinomialWtOmp(const mat &x, const vec &y, mat wt){
    int varn = x.n_cols;
    int dpn = x.n_rows;
    mat betas = mat(varn, dpn, fill::zeros);
    mat S = mat(dpn,dpn);
    mat n = vec(y.n_rows,fill::ones);
    int itCount = 0;
//    double lLik = 0.0;
    double oldLLik = 0.0;
    vec mu = vec(dpn,fill::ones) * 0.5;
    vec nu = vec(dpn,fill::zeros);
//    vec cv = vec(dpn);
    mWt2 = ones(dpn);
    mLLik = 0;
    while(!checkCanceled()){
        //计算公式有调整
        myAdj = nu + (y - mu)/(mu % (1 - mu));
        for (int i = 0; i < dpn && !checkCanceled(); i++)
        {
            vec wi = wt.col(i);
            vec gwsi = gwReg(x, myAdj, wi % mWt2, i);
            betas.col(i) = gwsi;
        }
        mat betas1 = trans(betas);
        nu = Fitted(x,betas1);
        mu = exp(nu)/(1 + exp(nu));
        oldLLik = mLLik;
        mLLik = sum(lchoose(n,y) + (n-y)%log(1 - mu/n) + y%log(mu/n));
        if (abs((oldLLik - mLLik)/mLLik) < mTol)
            break;
        mWt2 = n%mu%(1-mu);
        itCount++;
        if (itCount == mMaxiter)
            break;
    }
    return mu;
}
#endif
void GwmGeneralizedGWRAlgorithm::createResultLayer(CreateResultLayerData data,QString name)
{
    //避免图层名重复
    if(treeChildCount > 0)
    {
        name = name + "(" + QString::number(treeChildCount) + ")";
    }
    //节点记录标签
    treeChildCount++ ;

    QgsVectorLayer* srcLayer = mRegressionLayer ? mRegressionLayer : mDataLayer;
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    layerName += name;
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


void GwmGeneralizedGWRAlgorithm::setBandwidthSelectionCriterionType(const BandwidthSelectionCriterionType &bandwidthSelectionCriterionType)
{
    // mBandwidthSelectionCriterionType = bandwidthSelectionCriterionType;
    // QMap<QPair<BandwidthSelectionCriterionType, gwm::ParallelType>, BandwidthSelectCriterionFunction> mapper = {
    //     std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, gwm::ParallelType::SerialOnly), &GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionCVSerial),
    // #ifdef ENABLE_OpenMP
    //     std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, gwm::ParallelType::OpenMP), &GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionCVOmp),
    // #endif
    //     std::make_pair(qMakePair(BandwidthSelectionCriterionType::CV, gwm::ParallelType::CUDA), &GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionCVSerial),
    //     std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, gwm::ParallelType::SerialOnly), &GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionAICSerial),
    // #ifdef ENABLE_OpenMP
    //     std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, gwm::ParallelType::OpenMP), &GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionAICOmp),
    // #endif
    //     std::make_pair(qMakePair(BandwidthSelectionCriterionType::AIC, gwm::ParallelType::CUDA), &GwmGeneralizedGWRAlgorithm::bandwidthSizeGGWRCriterionAICSerial)
    // };
    // mBandwidthSelectCriterionFunction = mapper[qMakePair(bandwidthSelectionCriterionType, mParallelType)];
}

void GwmGeneralizedGWRAlgorithm::setParallelType(const gwm::ParallelType &type)
{
    mParallelType = type;
    mGGWRCore->setParallelType(type);
    setBandwidthSelectionCriterionType(mBandwidthSelectionCriterionType);
    setFamily(mFamily);
}

mat GwmGeneralizedGWRAlgorithm::diag(mat a){
    int n = a.n_rows;
    mat res = mat(uword(0), uword(0));
    if(a.n_cols > 1){
        res = vec(a.n_rows);
        for(int i = 0; i < n; i++){
            res[i] = a.row(i)[i];
        }
    }
    else{
        res = mat(a.n_rows,a.n_rows);
        mat base = eye(n,n);
        for(int i = 0; i < n; i++){
            res.row(i) = a[i] * base.row(i);
        }
    }
    return res;
}

//GWR clalibration
vec GwmGeneralizedGWRAlgorithm::gwReg(const mat& x, const vec &y, const vec &w, int focus)
{
    mat wspan(1, x.n_cols, fill::ones);
    mat xtw = trans(x % (w * wspan));
    mat xtwx = xtw * x;
    mat xtwy = xtw * y;
    mat xtwx_inv = inv(xtwx);
    vec beta = xtwx_inv * xtwy;
    return beta;
}

vec GwmGeneralizedGWRAlgorithm::gwRegHatmatrix(const mat &x, const vec &y, const vec &w, int focus, mat& ci, mat& s_ri)
{
    mat wspan(1, x.n_cols, fill::ones);
    mat xtw = trans(x % (w * wspan));
    mat xtwx = xtw * x;
    mat xtwy = xtw * y;
    mat xtwx_inv = inv(xtwx);
    vec beta = xtwx_inv * xtwy;
    ci = xtwx_inv * xtw;
    s_ri = x.row(focus) * ci;
    return beta;
}

mat GwmGeneralizedGWRAlgorithm::dpois(mat y,mat mu){
    int n = y.n_rows;
    mat res = vec(n);
    mat pdf = lgamma(y+1);
    res = -mu + y%log(mu) - pdf;
    return res;
}

mat GwmGeneralizedGWRAlgorithm::lchoose(mat n,mat k){
    int nrow = n.n_rows;
    mat res = vec(nrow);
//    for(int i = 0;i < nrow; i++){
//        res.row(i) = lgamma(n[i]+1) - lgamma(n[i]-k[i]+1) - lgamma(k[i]+1);
//    }
    res = lgamma(n+1) - lgamma(n-k+1) - lgamma(k+1);
    return res;
}

mat GwmGeneralizedGWRAlgorithm::dbinom(mat y,mat m,mat mu){
    int n = y.n_rows;
    mat res = vec(n);
    for(int i = 0;i < n; i++){
        double pdf = gsl_ran_binomial_pdf(int(y[i]), mu[i], int(m[i]));
        res[i] = log(pdf);
    }
    return res;
}

mat GwmGeneralizedGWRAlgorithm::lgammafn(mat x){
    int n = x.n_rows;
    mat res = vec(n,fill::zeros);
    for(int j = 0; j < n ; j++){
        res[j] = lgamma(x[j]);
    }
    return res;
}

mat GwmGeneralizedGWRAlgorithm::CiMat(const mat& x, const vec &w)
{
    return inv(trans(x) * diagmat(w) * x) * trans(x) * diagmat(w);
}

bool GwmGeneralizedGWRAlgorithm::setFamily(Family family){
    mFamily = family;
    QMap<QPair<Family, gwm::ParallelType>, GGWRRegressionFunction> mapper = {
        std::make_pair(qMakePair(Family::Poisson, gwm::ParallelType::SerialOnly), &GwmGeneralizedGWRAlgorithm::regressionPoissonSerial),
    #ifdef ENABLE_OpenMP
        std::make_pair(qMakePair(Family::Poisson, gwm::ParallelType::OpenMP), &GwmGeneralizedGWRAlgorithm::regressionPoissonOmp),
    #endif
//        std::make_pair(qMakePair(Family::Poisson, gwm::ParallelType::CUDA), &GwmGeneralizedGWRAlgorithm::regressionPoissonSerial),
        std::make_pair(qMakePair(Family::Binomial, gwm::ParallelType::SerialOnly), &GwmGeneralizedGWRAlgorithm::regressionBinomialSerial),
    #ifdef ENABLE_OpenMP
        std::make_pair(qMakePair(Family::Binomial, gwm::ParallelType::OpenMP), &GwmGeneralizedGWRAlgorithm::regressionBinomialOmp),
    #endif
//        std::make_pair(qMakePair(Family::Binomial, gwm::ParallelType::CUDA), &GwmGeneralizedGWRAlgorithm::regressionBinomialSerial)
    };
    mGGWRRegressionFunction = mapper[qMakePair(family, mParallelType)];
    QMap<QPair<Family, gwm::ParallelType>, CalWtFunction> mapper1 = {
        std::make_pair(qMakePair(Family::Poisson, gwm::ParallelType::SerialOnly), &GwmGeneralizedGWRAlgorithm::PoissonWtSerial),
    #ifdef ENABLE_OpenMP
        std::make_pair(qMakePair(Family::Poisson, gwm::ParallelType::OpenMP), &GwmGeneralizedGWRAlgorithm::PoissonWtOmp),
    #endif
//        std::make_pair(qMakePair(Family::Poisson, gwm::ParallelType::CUDA), &GwmGeneralizedGWRAlgorithm::PoissonWtSerial),
        std::make_pair(qMakePair(Family::Binomial, gwm::ParallelType::SerialOnly), &GwmGeneralizedGWRAlgorithm::BinomialWtSerial),
    #ifdef ENABLE_OpenMP
        std::make_pair(qMakePair(Family::Binomial, gwm::ParallelType::OpenMP), &GwmGeneralizedGWRAlgorithm::BinomialWtOmp),
    #endif
//        std::make_pair(qMakePair(Family::Binomial, gwm::ParallelType::CUDA), &GwmGeneralizedGWRAlgorithm::BinomialWtSerial)
    };
    mCalWtFunction = mapper1[qMakePair(family, mParallelType)];
    return true;
}

// below is new functions

// 转换 Family 枚举
gwm::GWRGeneralized::Family GwmGeneralizedGWRAlgorithm::convertFamily(Family family)
{
    return (family == Family::Poisson) ?
               gwm::GWRGeneralized::Family::Poisson :
               gwm::GWRGeneralized::Family::Binomial;
}

// 转换 BandwidthSelectionCriterionType 枚举
gwm::GWRGeneralized::BandwidthSelectionCriterionType GwmGeneralizedGWRAlgorithm::convertCriterionType(BandwidthSelectionCriterionType type)
{
    return (type == AIC) ?
               gwm::GWRGeneralized::BandwidthSelectionCriterionType::AIC :
               gwm::GWRGeneralized::BandwidthSelectionCriterionType::CV;
}

// 转换诊断信息
GwmGGWRDiagnostic GwmGeneralizedGWRAlgorithm::convertDiagnostic(const gwm::GWRGeneralizedDiagnostic& kernel)
{
    GwmGGWRDiagnostic app;
    app.RSS = kernel.RSS;
    app.AIC = kernel.AIC;
    app.AICc = kernel.AICc;
    app.RSquare = kernel.RSquare;
    return app;
}

// 转换GLM诊断信息
GwmGLMDiagnostic GwmGeneralizedGWRAlgorithm::convertGLMDiagnostic(const gwm::GLMDiagnostic& kernel)
{
    GwmGLMDiagnostic app;
    app.NullDev = kernel.NullDev;
    app.Dev = kernel.Dev;
    app.AIC = kernel.AIC;
    app.AICc = kernel.AICc;
    app.RSquare = kernel.RSquare;
    return app;
}

// F-Test Calculation
void GwmGeneralizedGWRAlgorithm::fTest(FTestParameters params)
{
    emit message("F Test");
    GwmFTestResult f1, f2, f4;
    QList<GwmFTestResult> f3;
    double v1 = params.trS, v2 = params.trStS;
    int nDp = params.nDp, nVar = params.nVar;
    emit tick(0, nVar + 3);

    double RSSg = params.gwrRSS;
    vec betao = solve(mX, mY);
    vec residual = mY - mX * betao;
    double RSSo = sum(residual % residual);
    double DFo = nDp - nVar;
    double delta1 = 1.0 * nDp - 2 * v1 + v2;
    double sigma2delta1 = RSSg / delta1;
    double trQ = params.trQ, trQtQ = params.trQtQ;
    double lDelta1 = trQ;
    double lDelta2 = trQtQ;

    // F1 Test
    if(!checkCanceled())
    {
        f1.s = (RSSg/lDelta1)/(RSSo/DFo);
        f1.df1 = lDelta1 * lDelta1 / lDelta2;
        f1.df2 = DFo;
        f1.p = gsl_cdf_fdist_P(f1.s, f1.df1, f1.df2);
        emit tick(1, nVar + 3);
    }

    // F2 Test
    if(!checkCanceled())
    {
        f2.s = ((RSSo-RSSg)/(DFo-lDelta1))/(RSSo/DFo);
        f2.df1 = (DFo-lDelta1) * (DFo-lDelta1) / (DFo - 2 * lDelta1 + lDelta2);
        f2.df2 = DFo;
        f2.p = gsl_cdf_fdist_Q(f2.s, f2.df1, f2.df2);
        emit tick(2, nVar + 3);
    }

    // F3 Test
    if(!checkCanceled())
    {
        vec vk2(nVar, fill::zeros);
        for (int i = 0; i < nVar && !checkCanceled(); i++)
        {
            vec betasi = mBetas.col(i);
            vec betasJndp = vec(nDp, fill::ones) * (sum(betasi) * 1.0 / nDp);
            vk2(i) = (1.0 / nDp) * det(trans(betasi - betasJndp) * betasi);
        }

        // 参考 BasicGWR 的实现，简化错误处理
        for (int i = 0; i < nVar && !checkCanceled(); i++)
        {
            vec diagB = calcDiagBSerial(i);
            if (!checkCanceled())
            {
                // 如果返回 DBL_MAX，说明计算失败，跳过该变量
                if (diagB(0) == DBL_MAX || diagB(1) == DBL_MAX)
                {
                    GwmFTestResult f3i;
                    f3i.s = 0.0;
                    f3i.df1 = 0.0;
                    f3i.df2 = 0.0;
                    f3i.p = 1.0;
                    f3.append(f3i);
                    continue;
                }
                
                double g1 = diagB(0);
                double g2 = diagB(1);
                double numdf = g1 * g1 / g2;
                
                // 检查计算结果的有效性
                if (g1 <= 0 || g2 <= 0 || numdf <= 0 || !isfinite(numdf))
                {
                    GwmFTestResult f3i;
                    f3i.s = 0.0;
                    f3i.df1 = 0.0;
                    f3i.df2 = 0.0;
                    f3i.p = 1.0;
                    f3.append(f3i);
                    continue;
                }
                
                GwmFTestResult f3i;
                f3i.s = (vk2(i) / g1) / sigma2delta1;
                f3i.df1 = numdf;
                f3i.df2 = f1.df1;
                f3i.p = gsl_cdf_fdist_Q(f3i.s, numdf, f1.df1);
                f3.append(f3i);
                emit tick(3 + i, nVar + 3);
            }
        }
    }

    // F4 Test
    if(!checkCanceled())
    {
        f4.s = RSSg / RSSo;
        f4.df1 = delta1;
        f4.df2 = DFo;
        f4.p = gsl_cdf_fdist_P(f4.s, f4.df1, f4.df2);
        emit tick(nVar + 3, nVar + 3);
    }

    // 保存结果
    if(!checkCanceled())
    {
        mF1TestResult = f1;
        mF2TestResult = f2;
        mF3TestResult = f3;
        mF4TestResult = f4;
    }
}


// 计算 F3 Test 所需的 diagB（针对 GGWR）
// 参考 BasicGWR 的实现，使用 inv_sympd 提高数值稳定性
vec GwmGeneralizedGWRAlgorithm::calcDiagBSerial(int i)
{
    arma::uword nDp = mX.n_rows, nVar = mX.n_cols;
    vec diagB(nDp, fill::zeros), c(nDp, fill::zeros);
    mat wspan(1, nVar, fill::ones);
    
    // 第一遍循环：计算 c（所有数据点的系数矩阵第 i 列的平均值）
    for (arma::uword j = 0; j < nDp && !checkCanceled(); j++)
    {
        vec wj = mWtMat2.col(j);
        vec weights = wj % mWt2;
        
        // 检查权重有效性
        if (sum(weights) < 1e-10 || any(weights < 0) || !weights.is_finite())
        {
            emit error("Invalid weights in calcDiagB (first loop).");
            return { DBL_MAX, DBL_MAX };
        }
        
        mat xtw = trans(mX % (weights * wspan));
        try {
            // 使用 inv_sympd 替代 pinv，与 BasicGWR 保持一致
            mat C = trans(xtw) * inv_sympd(xtw * mX);
            c += C.col(i);
        } catch (...) {
            emit error("Matrix seems to be singular in calcDiagB (first loop).");
            return { DBL_MAX, DBL_MAX };
        }
    }
    
    // 第二遍循环：计算 diagB
    for (arma::uword k = 0; k < nDp && !checkCanceled(); k++)
    {
        vec wk = mWtMat2.col(k);
        vec weights = wk % mWt2;
        
        // 检查权重有效性
        if (sum(weights) < 1e-10 || any(weights < 0) || !weights.is_finite())
        {
            emit error("Invalid weights in calcDiagB (second loop).");
            return { DBL_MAX, DBL_MAX };
        }
        
        mat xtw = trans(mX % (weights * wspan));
        try {
            // 使用 inv_sympd 替代 pinv，与 BasicGWR 保持一致
            mat C = trans(xtw) * inv_sympd(xtw * mX);
            vec b = C.col(i);
            diagB += (b % b - (1.0 / nDp) * (b % c));
        } catch (...) {
            emit error("Matrix seems to be singular in calcDiagB (second loop).");
            return { DBL_MAX, DBL_MAX };
        }
    }
    
    diagB = 1.0 / nDp * diagB;
    return { sum(diagB), sum(diagB % diagB) };
}


vec GwmGeneralizedGWRAlgorithm::rebuildQDiagFromS(const arma::mat& S)
{
    // Q = (I - S)^T (I - S)
    // qdiag = diag(Q) = colSums( (I - S) % (I - S) )
    arma::uword n = S.n_rows;
    arma::mat EmS = arma::eye(n, n) - S;
    return arma::trans(arma::sum(EmS % EmS, 0));   // n x 1
}

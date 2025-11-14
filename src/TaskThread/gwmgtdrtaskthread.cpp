#include "gwmgtdrtaskthread.h"
#include <exception>
#include <gwmodel.h>
#include "SpatialWeight/gwmcrsdistance.h"
#ifdef ENABLE_OpenMP
#include <omp.h>
#endif

using namespace std;
using namespace gwm;

int GwmGTDRTaskThread::treeChildCount = 0;

GwmGTDRTaskThread::GwmGTDRTaskThread()
{
    mGTDRCore = std::make_unique<gwm::GTDR>();

}

GwmGTDRTaskThread::GwmGTDRTaskThread(const GwmAlgorithmMetaGTDR& meta) : mMeta(meta)
{

    mGTDRCore = std::make_unique<gwm::GTDR>();

    // Check parameter
    QString metaError;
    if (!meta.validate(metaError))
    {
        throw std::bad_alloc();
    }

    mLayer = meta.layer;
    mIndepVars = meta.independentVariables;
    mDepVar = meta.dependentVariable;

    // Spatial Weight
    uword nDim = mIndepVars.size();
    std::vector<SpatialWeight> spatials;
    mBandwidthHolders.clear();
    mDistanceHolders.clear();
    mBandwidthHolders.reserve(nDim);
    mDistanceHolders.reserve(nDim);
    for (size_t i = 0; i < nDim; i++)
    {
        //OneDimDistance distance;
        //BandwidthWeight bandwidth(meta.weightBandwidthSize, meta.weightBandwidthAdaptive, meta.weightBandwidthKernel);
        //spatials.push_back(SpatialWeight(&bandwidth, &distance));

        auto bw = std::make_unique<BandwidthWeight>(meta.weightBandwidthSize, meta.weightBandwidthAdaptive, meta.weightBandwidthKernel);
        auto dist = std::make_unique<OneDimDistance>();

        BandwidthWeight* bwRaw = bw.get();
        OneDimDistance* distRaw = dist.get();
        mBandwidthHolders.push_back(std::move(bw));
        mDistanceHolders.push_back(std::move(dist));

        spatials.emplace_back(bwRaw, distRaw);
    }
    mAlgorithm.setSpatialWeights(spatials);
    // Parallel
    mAlgorithm.setParallelType(meta.parallelType);
    switch (meta.parallelType)
    {
    case gwm::ParallelType::OpenMP:
        mAlgorithm.setOmpThreadNum(meta.parallelOmpThreads);
        break;
    default:
        break;
    }
    // Others
    mAlgorithm.setHasHatMatrix(meta.hatmatrix);
    // delete distance;
}

void GwmGTDRTaskThread::run()
{
    emit tick(0, 0);
    if (!checkCanceled())
    {
        emit message(tr("Extracting data and coordinates."));
        mAlgorithm.setCoords(initPoints(mLayer));
        emit message(tr("1"));
        initXY(mX, mY, mDepVar, mIndepVars);
        emit message(tr("2"));
        mAlgorithm.setIndependentVariables(mX);
        mAlgorithm.setDependentVariable(mY);
        emit message(tr("variable set"));

        //set parameters for OneDimDistance: Each dimension has its own dependent variable column
        const auto& sws = mAlgorithm.spatialWeights();
        if (sws.size() == 0) {
            emit error(tr("GTDR invalid: spatialWeights empty before parameterization."));
            return;
        }
        // mX 列0是截距，从列1开始对应各自变量
        if (mX.n_cols < 2 || sws.size() != size_t(mX.n_cols - 1)) {
            emit message(tr("[WARN] spatialWeights size (%1) != indep columns (%2)")
                         .arg(sws.size()).arg(mX.n_cols - 1));
        }
        arma::uword p = mX.n_cols >= 1 ? mX.n_cols - 1 : 0;
        for (arma::uword k = 0; k < p && k < sws.size(); ++k) {
            auto* od = sws[k].distance<gwm::OneDimDistance>();
            if (!od) {
                emit error(tr("GTDR invalid: spatialWeights[%1] is not OneDimDistance.").arg(int(k)));
                return;
            }
            arma::vec col = mX.col(k + 1);  // 自变量第k列（跳过截距列）
            // focus 和 data 都使用该列（计算一维距离 |x_i - x_j|）
            od->makeParameter({ col, col });
        }
    }

    mAlgorithm.setTelegram(std::make_unique<GwmTaskThreadTelegram>(this));
    if(!checkCanceled() && mMeta.bandwidthAuto)
    {
        // Bandwidth size selection
        emit message(tr("Automatically selecting bandwidth..."));

        vector<gwm::BandwidthWeight*> vecBandwidthWeight0 ;

        // 1. Prepare data
        mGTDRCore->setCoords(mAlgorithm.coords());
        mGTDRCore->setDependentVariable(mY);
        mGTDRCore->setIndependentVariables(mX);

        // 2. Copy spatial weight
        std::vector<gwm::SpatialWeight> tempSpatialWeights = mAlgorithm.spatialWeights();
        mGTDRCore->setSpatialWeights(tempSpatialWeights);//

        const auto& sws = mGTDRCore->spatialWeights();
        for (size_t i = 0; i < sws.size() && i < mX.n_cols - 1; ++i)
        {
            auto* od = sws[i].distance<gwm::OneDimDistance>();
            if (od)
            {
                arma::vec col = mX.col(i + 1);
                od->makeParameter({ col, col });
            }
        }
        //mGTDRCore->setSpatialWeights(tempSpatialWeights);

        // 3. Set other parameters of GTDR
        mGTDRCore->setParallelType(mMeta.parallelType);
        if (mMeta.parallelType == gwm::ParallelType::OpenMP)
        {
            mGTDRCore->setOmpThreadNum(mMeta.parallelOmpThreads);
        }
        mGTDRCore->setHasHatMatrix(mMeta.hatmatrix);
        mGTDRCore->setBandwidthCriterionType(mMeta.bandwidthCriterionType);
        mGTDRCore->setTelegram(std::make_unique<GwmTaskThreadTelegram>(this));

        // 4. 收集所有维度的带宽权重指针
        std::vector<gwm::BandwidthWeight*> bandwidths;
        //const auto& sws = mGTDRCore->spatialWeights();
        for (const auto& sw : sws)
        {
            auto* bw = sw.weight<gwm::BandwidthWeight>();
            if (bw)
            {
                bandwidths.push_back(bw);
                
                // 设置初始带宽值（如果当前值不合理）
                double lower = bw->adaptive() ? (mIndepVars.size() + 1) : 0.0;
                double upper = bw->adaptive() ? mX.n_rows : sw.distance()->maxDistance();
                if (bw->bandwidth() <= lower || bw->bandwidth() >= upper || !isfinite(bw->bandwidth()))
                {
                    double initBw = bw->adaptive() 
                        ? std::max(20.0, upper * 0.618) 
                        : upper * 0.618;
                    bw->setBandwidth(initBw);
                }
            }
        }

        if (bandwidths.empty())
        {
            emit error(tr("GTDR invalid: no bandwidth weights found."));
            return;
        }

        // 5. 创建优化器并执行优化
        emit message(tr("Initializing GTDRBandwidthOptimizer for %1 dimensions...").arg(bandwidths.size()));

        // 参数验证和调整
        double eps = std::max(1e-4, mMeta.bandwidthOptimizeEps);  // 最小 1e-4
        double step = (mMeta.bandwidthOptimizeStep > 0 && mMeta.bandwidthOptimizeStep <= 0.5)
                          ? mMeta.bandwidthOptimizeStep
                          : 0.1;
        size_t maxIter = (mMeta.bandwidthOptimizeMaxIter > 0 && mMeta.bandwidthOptimizeMaxIter <= 1000)
                             ? mMeta.bandwidthOptimizeMaxIter
                             : 500;
        emit message(tr("Optimization parameters: eps=%1, step=%2, maxIter=%3")
                         .arg(eps, 0, 'g', 6).arg(step).arg(maxIter));
        // 创建优化器
        gwm::GTDRBandwidthOptimizer optimizer(bandwidths);
        // 执行优化
        QElapsedTimer timer;
        timer.start();

        try
        {
            int resultCode = optimizer.optimize(
                mGTDRCore.get(),      // GTDR 实例
                mX.n_rows,            // featureCount
                maxIter,              // maxIter
                eps,                  // eps
                step                  // step
                );

            qint64 elapsed = timer.elapsed();

            if (resultCode == 0)  // GSL_SUCCESS
            {
                emit message(tr("Bandwidth optimization completed successfully in %1 ms").arg(elapsed));

                // 6. 获取优化后的带宽值并更新到 mAlgorithm
                const auto& optimizedSws = mGTDRCore->spatialWeights();
                auto& algorithmSws = mAlgorithm.spatialWeights();

                for (size_t i = 0; i < optimizedSws.size() && i < algorithmSws.size(); ++i)
                {
                    auto* optimizedBw = optimizedSws[i].weight<gwm::BandwidthWeight>();
                    auto* algorithmBw = algorithmSws[i].weight<gwm::BandwidthWeight>();

                    if (optimizedBw && algorithmBw)
                    {
                        // 将优化后的带宽值复制到 mAlgorithm
                        algorithmBw->setBandwidth(optimizedBw->bandwidth());

                        QString varName = i < mIndepVars.size()
                                              ? mIndepVars[i].name
                                              : QString("Dimension_%1").arg(i);

                        emit message(tr("Dimension %1 (%2): optimized bandwidth = %3")
                                         .arg(i).arg(varName).arg(optimizedBw->bandwidth()));
                    }
                }
            }
            else
            {
                emit error(tr("Bandwidth optimization failed with code: %1").arg(resultCode));
                // 可以继续使用初始带宽值
            }
        }
        catch (const std::exception& e)
        {
            emit error(tr("Bandwidth optimization exception: %1").arg(e.what()));
        }

        // 7. 关闭库内部优化（因为我们已经手动优化了）
        mAlgorithm.setEnableBandwidthOptimize(false);

        // 
        // mAlgorithm.fit();
        // emit message(tr("Fit completed."));

        // 获取优化后的带宽值
        // const auto& sws = mAlgorithm.spatialWeights();
        // for (size_t i = 0; i < sws.size(); ++i)
        // {
        //     auto* bw = sws[i].weight<gwm::BandwidthWeight>();
        //     if (bw) {
        //         emit message(tr("Dimension %1: optimized bandwidth = %2")
        //                          .arg(i).arg(bw->bandwidth()));
        //     }
        // }
    }else{
        emit message(tr("Bandwidth size defined."));
        mAlgorithm.setEnableBandwidthOptimize(false);
        // mAlgorithm.fit();
        // emit message(tr("Fit completed."));
    }

    // Run algorithm;
    if (checkCanceled()) return;
    try
    {
        //mAlgorithm.setTelegram(std::make_unique<GwmTaskThreadTelegram>(this));
        //if(!mAlgorithm.isValid())
        //{
        //    emit error(tr("GTDR invalid: check X/Y/coords/spatialWeights consistency."));
        //    return;
        //}
        // 1) 基本维度检查
        // emit message(tr("[DBG] X shape: %1, %2").arg(mX.n_rows).arg(mX.n_cols));
        // emit message(tr("[DBG] Y shape: %1").arg(mY.n_rows));
        // arma::mat coords = mAlgorithm.coords();
        // emit message(tr("[DBG] coords shape: %1, %2").arg(coords.n_rows).arg(coords.n_cols));

        // if (mX.n_rows == 0 || mY.n_rows == 0 || coords.n_rows == 0) {
        //     emit error(tr("GTDR invalid: X/Y/coords empty."));
        //     return;
        // }
        // if (mX.n_rows != mY.n_rows || mX.n_rows != coords.n_rows) {
        //     emit error(tr("GTDR invalid: X/Y/coords row-size mismatch: X=%1, Y=%2, C=%3")
        //                .arg(mX.n_rows).arg(mY.n_rows).arg(coords.n_rows));
        //     return;
        // }
        // 2) 空间权重检查
        //     const auto& sws = mAlgorithm.spatialWeights();
        //     emit message(QString("[DBG] spatialWeights count: %1").arg(sws.size()));
        //     if (sws.empty()) {
        //         emit error(tr("GTDR invalid: spatialWeights empty."));
        //         return;
        //     }
        // 一般 GTDR 用“每个维度一个权重”，这里检查数量与自变量个数的一致性
        //     if (sws.size() != size_t(mIndepVars.size())) {
        //         emit message(QString("[WARN] spatialWeights size (%1) != indepVars size (%2)")
        //                      .arg(sws.size()).arg(mIndepVars.size()));
        //     }
        //     bool hasNull = false;
        //     for (size_t i = 0; i < sws.size(); ++i) {
        //         bool wok = sws[i].weight() != nullptr;
        //         bool dok = sws[i].distance() != nullptr;
        //         emit message(QString("[DBG] sw[%1]: weight=%2, distance=%3")
        //                      .arg(int(i)).arg(wok ? "ok" : "NULL").arg(dok ? "ok" : "NULL"));
        //         if (!wok || !dok) hasNull = true;
        //     }
        //     if (hasNull) {
        //         emit error(tr("GTDR invalid: some spatialWeights have NULL weight/distance."));
        //         return;
        //     }
        // 3) 取一个样本点尝试生成权重向量，排除 distance/weight 内部再用到空指针
        /**/
        // try {
        //     const SpatialWeight& sw0 = sws.front();
        //     arma::vec w0 = sw0.weightVector(0);    // 若这里抛异常/崩溃，distance/weight 内部还在引用空对象
        //     emit message(QString("[DBG] sw[0].weightVector(0) len=%1, first=%2")
        //                  .arg(w0.n_rows).arg(w0.n_rows ? w0(0) : 0.0, 0, 'g', 10));
        // } catch (const std::exception& ex) {
        //     emit error(QString("GTDR invalid: weightVector test failed: %1").arg(ex.what()));
        //     return;
        // } catch (...) {
        //     emit error(tr("GTDR invalid: weightVector test failed: unknown exception."));
        //     return;
        // }
        // 4) lib 层面的 isValid() 兜底
        //if (!mAlgorithm.isValid()) {
        //    emit error(tr("GTDR invalid: check X/Y/coords/spatialWeights consistency."));
        //    return;
        //}
        //检查结束
        
        mAlgorithm.fit();
        emit message(tr("fit"));
        mDiagnostic=mAlgorithm.diagnostic();
        emit message(tr("diagnostic"));
        mBetas = mAlgorithm.betas();
        mBetasSE = mAlgorithm.betasSE();

        int nDp = mX.n_rows;
        vec shat = mAlgorithm.sHat();
        double trs = shat(0);
        double trStS = shat(1);
        //注意：仅mAlgorithm.hasHatMatrix()为true时，才计算sigmaHat
        //后续需要添加检查机制
        double sigmaHat = mDiagnostic.RSS / (nDp - 2 * trs + trStS);
        vec qDiag = mAlgorithm.qDiag();
        mBetasSE = sqrt(sigmaHat * mBetasSE);
        vec yhat = mAlgorithm.Fitted(mX, mBetas);
        vec res = mY - yhat;
        vec stu_res = res / sqrt(sigmaHat * qDiag);
        mat betasTV = mBetas/mBetasSE;
        const auto& spatialWeights = mAlgorithm.spatialWeights();
        const SpatialWeight& sw = spatialWeights.front(); // 或选择合适维度的 sw
        vec dybar2 = (mY - mean(mY)) % (mY - mean(mY));
        vec dyhat2 = (mY - yhat) % (mY - yhat);
        vec localR2 = vec(nDp, fill::zeros);
        for (uword i = 0; i < nDp && !checkCanceled(); i++)
        {
            vec w = sw.weightVector(i);
            double tss = sum(dybar2 % w);
            double rss = sum(dyhat2 % w);
            localR2(i) = (tss - rss) / tss;
        }

        if(!checkCanceled())
        {   
            mResultList.push_back(qMakePair(QString("%1"), mBetas));
            mResultList.push_back(qMakePair(QString("y"), mY));
            mResultList.push_back(qMakePair(QString("yhat"), yhat));
            mResultList.push_back(qMakePair(QString("residual"), res));
            mResultList.push_back(qMakePair(QString("%1_SE"), mBetasSE));
            mResultList.push_back(qMakePair(QString("%1_TV"), betasTV));
            mResultList.push_back(qMakePair(QString("localR2"), localR2));
        }
        if(!checkCanceled())
        {
            createResultLayer(mResultList);
            emit success();
            emit tick(100, 100);
        }
    }
    catch(const std::exception& e)
    {
        emit error(QString(e.what()));
    }
}

mat GwmGTDRTaskThread::initPoints(QgsVectorLayer* layer)
{
    int nDp = layer->featureCount();
    mat points(nDp, 2, fill::zeros);
    QgsFeatureIterator iterator = layer->getFeatures();
    QgsFeature f;
    for (int i = 0; iterator.nextFeature(f); i++)
    {
        QgsPointXY centroPoint = f.geometry().centroid().asPoint();
        points(i, 0) = centroPoint.x();
        points(i, 1) = centroPoint.y();
    }
    return points;
}

void GwmGTDRTaskThread::initXY(mat &x, mat &y, const GwmVariable &depVar, const QList<GwmVariable> &indepVars)
{
    emit message(tr("10"));
    int nDp = mLayer->featureCount(), nVar = indepVars.size() + 1;
    // Data layer and X,Y
    emit message(tr("11"));
    x = mat(nDp, nVar, fill::zeros);
    y = vec(nDp, fill::zeros);
    emit message(tr("11"));
    QgsFeatureIterator iterator = mLayer->getFeatures();
    QgsFeature f;
    bool ok = false;
    emit message(tr("12"));
    for (int i = 0; iterator.nextFeature(f); i++)
    {
    emit message(tr("13"));
        double vY = f.attribute(depVar.name).toDouble(&ok);
        if (ok)
        {
            y(i) = vY;
            x(i, 0) = 1.0;
            for (int k = 0; k < indepVars.size(); k++)
            {
                double vX = f.attribute(indepVars[k].name).toDouble(&ok);
                if (ok) x(i, k + 1) = vX;
                else emit error(tr("Independent variable value cannot convert to a number. Set to 0."));
            }
        }
        else emit error(tr("Dependent variable value cannot convert to a number. Set to 0."));
    }
    emit message(tr("13"));

    
    // if (hasRegressionLayer())
    // {
    //     // 检查回归点图层是否包含了所有变量
    //     QStringList fieldNameList = mRegressionLayer->fields().names();
    //     bool flag = fieldNameList.contains(depVar.name);
    //     for (auto field : indepVars)
    //     {
    //         flag = flag && fieldNameList.contains(field.name);
    //     }
    //     mHasRegressionLayerXY = flag;
    //     if (flag)
    //     {
    //         // 设置回归点X和回归点Y
    //         int regressionPointsSize = mRegressionLayer->featureCount();
    //         mRegressionLayerY = vec(regressionPointsSize, fill::zeros);
    //         mRegressionLayerX = mat(regressionPointsSize, indepVars.size() + 1, fill::zeros);
    //         QgsFeatureIterator iterator = mRegressionLayer->getFeatures();
    //         QgsFeature f;
    //         bool ok = false;
    //         for (int i = 0; iterator.nextFeature(f); i++)
    //         {
    //             double vY = f.attribute(depVar.name).toDouble(&ok);
    //             if (ok)
    //             {
    //                 mRegressionLayerY(i) = vY;
    //                 mRegressionLayerX(i, 0) = 1.0;
    //                 for (int k = 0; k < indepVars.size(); k++)
    //                 {
    //                     double vX = f.attribute(indepVars[k].name).toDouble(&ok);
    //                     if (ok) mRegressionLayerX(i, k + 1) = vX;
    //                 }
    //             }
    //         }
    //     }
    // }
}

void GwmGTDRTaskThread::createResultLayer(CreateResultLayerData data)
{
    QgsVectorLayer* srcLayer = mLayer;
    int nVar = mIndepVars.size();
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    //避免图层名重复
    if(treeChildCount > 0)
    {
        layerName += QStringLiteral("_GTDR") + "(" + QString::number(treeChildCount) + ")";
    } else
    {
        layerName += QStringLiteral("_GTDR");
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

    // test code
    emit message(tr("[GTDR] result fields=%1, features=%2")
    .arg(mResultLayer->fields().count())
    .arg(int(mResultLayer->featureCount())));
    // 打印前两条要素的前5个属性，避免UI阻塞
    int cnt = 0;
    for (auto it = mResultLayer->getFeatures(); cnt < 2 && it.nextFeature(f); ++cnt) {
        QStringList vals;
        for (int k = 0; k < std::min(5, mResultLayer->fields().count()); ++k)
            vals << f.attribute(k).toString();
        emit message(tr("[GTDR] feat%1 attrs: %2").arg(cnt).arg(vals.join(",")));
    }
}

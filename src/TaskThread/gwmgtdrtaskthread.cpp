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
    //mGTDRCore = std::make_unique<gwm::GTDR>();
    isOptSuccess = false;

}

GwmGTDRTaskThread::GwmGTDRTaskThread(const GwmAlgorithmMetaGTDR& meta) : mMeta(meta)
{

    //mGTDRCore = std::make_unique<gwm::GTDR>();
    isOptSuccess = false;

    // Check parameter
    QString metaError;
    if (!meta.validate(metaError))
    {
        throw std::bad_alloc();
    }

    mLayer = meta.layer;
    mIndepVars = meta.independentVariables;
    mDepVar = meta.dependentVariable;

    if(mMeta.weightingVariables.isEmpty()){
        emit error(tr("Weighting variables are empty."));
    }

    // Spatial Weight
    uword nDim = meta.weightingVariables.size();
    bool hasTimeStamp = !meta.timeStampVariable.name.isEmpty() && meta.timeStampVariable.index >= 0; // 检查时间戳是否存在（index >= 0 表示有效）
    if (hasTimeStamp)
    {
        nDim++;  // 如果存在时间戳，增加一个维度
    }

    std::vector<SpatialWeight> spatials;
    mBandwidthHolders.clear();
    mDistanceHolders.clear();
    mBandwidthHolders.reserve(nDim);
    mDistanceHolders.reserve(nDim);
    for (size_t i = 0; i < meta.weightingVariables.size(); i++)
    {
        // 获取该维度的初始带宽值和核函数类型
        // 如果列表不为空，使用列表中的值；否则使用单个默认值（向后兼容）
        double bwSize;
        gwm::BandwidthWeight::KernelFunctionType kernel;

        if (i < meta.weightBandwidthSizes.size() && i < meta.weightBandwidthKernels.size())
        {
            // 使用列表中的值（每个维度不同）
            bwSize = meta.weightBandwidthSizes[i];
            kernel = meta.weightBandwidthKernels[i];
        }
        else
        {
            // 向后兼容：如果列表为空，使用单个默认值
            bwSize = meta.weightBandwidthSize;
            kernel = meta.weightBandwidthKernel;
        }

        auto bw = std::make_unique<BandwidthWeight>(bwSize, meta.weightBandwidthAdaptive, kernel);
        auto dist = std::make_unique<OneDimDistance>();

        BandwidthWeight* bwRaw = bw.get();
        OneDimDistance* distRaw = dist.get();
        mBandwidthHolders.push_back(std::move(bw));
        mDistanceHolders.push_back(std::move(dist));

        spatials.emplace_back(bwRaw, distRaw);
    }
    // 为时间戳创建空间权重（如果存在）
    if (hasTimeStamp)
    {
        // 时间戳的带宽和核函数应该在参数列表的最后（因为TIMESTAMP项在最后）
        double bwSize;
        gwm::BandwidthWeight::KernelFunctionType kernel;
        
        size_t timeStampIndex = meta.weightingVariables.size();  // 时间戳在列表中的索引
        if (timeStampIndex < meta.weightBandwidthSizes.size() && timeStampIndex < meta.weightBandwidthKernels.size())
        {
            bwSize = meta.weightBandwidthSizes[timeStampIndex];
            kernel = meta.weightBandwidthKernels[timeStampIndex];
        }
        else
        {
            // 使用默认值
            bwSize = meta.weightBandwidthSize;
            kernel = meta.weightBandwidthKernel;
        }
        
        auto bw = std::make_unique<BandwidthWeight>(bwSize, meta.weightBandwidthAdaptive, kernel);
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

void GwmGTDRTaskThread::initWeightingVariables(mat& weightingData, const QList<GwmVariable>& weightingVars)
{
    int nDp = mLayer->featureCount();
    int nWeightingVars = weightingVars.size();

    // 检查是否有时间戳变量
    bool hasTimeStamp = !mMeta.timeStampVariable.name.isEmpty() && mMeta.timeStampVariable.index >= 0;
    int nTotalVars = nWeightingVars + (hasTimeStamp ? 1 : 0);

    weightingData = mat(nDp, nTotalVars, arma::fill::zeros);

    QgsFeatureIterator iterator = mLayer->getFeatures();
    QgsFeature f;
    bool ok = false;

    for (int i = 0; iterator.nextFeature(f); i++)
    {
        for (int k = 0; k < nWeightingVars; k++)
        {
            const GwmVariable& var = weightingVars[k];

            // 检查是否为空间坐标（使用特殊命名约定）
            if (var.name == QStringLiteral("__X_COORD__"))
            {
                // 提取 X 坐标
                if (f.hasGeometry() && !f.geometry().isEmpty())
                {
                    QgsPointXY centroPoint = f.geometry().centroid().asPoint();
                    weightingData(i, k) = centroPoint.x();
                }
                else
                {
                    emit error(tr("Feature %1 has no geometry for X coordinate extraction.").arg(i));
                }
            }
            else if (var.name == QStringLiteral("__Y_COORD__"))
            {
                // 提取 Y 坐标
                if (f.hasGeometry() && !f.geometry().isEmpty())
                {
                    QgsPointXY centroPoint = f.geometry().centroid().asPoint();
                    weightingData(i, k) = centroPoint.y();
                }
                else
                {
                    emit error(tr("Feature %1 has no geometry for Y coordinate extraction.").arg(i));
                }
            }
            else
            {
                // 普通属性变量
                double v = f.attribute(var.name).toDouble(&ok);
                if (ok)
                {
                    weightingData(i, k) = v;
                }
                else
                {
                    emit error(tr("Weighting variable '%1' value cannot convert to a number. Set to 0.").arg(var.name));
                }
            }
        }

        // 提取时间戳变量的数据（如果存在）
        if (hasTimeStamp)
        {
            int timeStampCol = nWeightingVars;  // 时间戳在最后一列
            double v = f.attribute(mMeta.timeStampVariable.name).toDouble(&ok);
            if (ok)
            {
                weightingData(i, timeStampCol) = v;
            }
            else
            {
                emit error(tr("Time stamp variable '%1' value cannot convert to a number. Set to 0.").arg(mMeta.timeStampVariable.name));
            }
        }
    }
}

void GwmGTDRTaskThread::run()
{
    emit tick(0, 0);
    if(!checkCanceled())
    {
        emit message(tr("Establishing coordinate matrix."));
        initWeightingVariables(mWeightingData, mMeta.weightingVariables);

        // 创建时空-变量坐标矩阵
        int nDp = mLayer->featureCount();
        bool hasTimeStamp = !mMeta.timeStampVariable.name.isEmpty() && mMeta.timeStampVariable.index >= 0;
        int nWeightingVars = mMeta.weightingVariables.size();
        int nTotalVars = nWeightingVars + (hasTimeStamp ? 1 : 0);

        arma::mat virtualCoords(nDp, nTotalVars, arma::fill::zeros);
        // 填充默认值
        for (int i = 0; i < nDp; i++)
        {
            for (int j = 0; j < nTotalVars; j++)
            {
                virtualCoords(i, j) = 0;
            }
        }
        mAlgorithm.setCoords(virtualCoords);
        emit message(tr("Virtual coordinates set: %1 rows x %2 cols (matching weighting variables)").arg(nDp).arg(nWeightingVars));
    }
    if (!checkCanceled())
    {
        emit message(tr("Extracting data and coordinates."));
        mAlgorithm.setCoords(initPoints(mLayer));
        initXY(mX, mY, mDepVar, mIndepVars);
        mAlgorithm.setIndependentVariables(mX);
        mAlgorithm.setDependentVariable(mY);
        emit message(tr("variables set"));

        //set parameters for OneDimDistance: Each dimension has its own dependent variable column
        const auto& sws = mAlgorithm.spatialWeights();
        if (sws.size() == 0) {
            emit error(tr("GTDR invalid: spatialWeights empty before parameterization."));
            return;
        }
        if (sws.size() != mWeightingData.n_cols)
        {
            emit error(tr("GTDR invalid: spatialWeights size (%1) != weighting variables count (%2)")
                      .arg(sws.size()).arg(mWeightingData.n_cols));
            return;
        }

        emit message(tr("Setting distance parameters using weighting variables..."));
        bool hasTimeStamp = !mMeta.timeStampVariable.name.isEmpty() && mMeta.timeStampVariable.index >= 0;
        int nWeightingVars = mMeta.weightingVariables.size();
        for (arma::uword k = 0; k < nWeightingVars && k < mWeightingData.n_cols; ++k)
        {
            auto* od = sws[k].distance<gwm::OneDimDistance>();
            if (!od) {
                emit error(tr("GTDR invalid: spatialWeights[%1] is not OneDimDistance.").arg(int(k)));
                return;
            }
            // 使用权重变量的第k列来设置距离参数
            arma::vec col = mWeightingData.col(k);
            od->makeParameter({ col, col });
            emit message(tr("Distance parameter set for weighting variable: %1")
                        .arg(mMeta.weightingVariables[k].name));
        }

        // 设置时间戳的距离参数（如果存在）
        if (hasTimeStamp && sws.size() > nWeightingVars && mWeightingData.n_cols > nWeightingVars)
        {
            arma::uword timeStampIndex = nWeightingVars;
            auto* od = sws[timeStampIndex].distance<gwm::OneDimDistance>();
            if (!od) {
                emit error(tr("GTDR invalid: spatialWeights[%1] (timestamp) is not OneDimDistance.").arg(int(timeStampIndex)));
                return;
            }
            arma::vec col = mWeightingData.col(timeStampIndex);
            od->makeParameter({ col, col });
            emit message(tr("Distance parameter set for timestamp variable: %1")
                        .arg(mMeta.timeStampVariable.name));
        }
    }

    mAlgorithm.setTelegram(std::make_unique<GwmTaskThreadTelegram>(this));
    if(!checkCanceled() && mMeta.bandwidthAuto)
    {
        // Bandwidth size selection
        emit message(tr("Automatically selecting bandwidth..."));
        vector<gwm::BandwidthWeight*> vecBandwidthWeight0 ;
        mAlgorithm.setBandwidthCriterionType(mMeta.bandwidthCriterionType);

        // 4. 收集所有维度的带宽权重指针
        std::vector<gwm::BandwidthWeight*> bandwidths;
        const auto& sws = mAlgorithm.spatialWeights();
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

        double eps = std::max(1e-4, mMeta.bandwidthOptimizeEps);  // 最小 1e-4
        double step = (mMeta.bandwidthOptimizeStep > 0 && mMeta.bandwidthOptimizeStep <= 0.5)
                          ? mMeta.bandwidthOptimizeStep
                          : 0.1;
        size_t maxIter = (mMeta.bandwidthOptimizeMaxIter > 0 && mMeta.bandwidthOptimizeMaxIter <= 1000)
                             ? mMeta.bandwidthOptimizeMaxIter
                             : 500;
        emit message(tr("Optimization parameters: eps=%1, step=%2, maxIter=%3")
                         .arg(eps, 0, 'g', 6).arg(step).arg(maxIter));
        // 优化
        gwm::GTDRBandwidthOptimizer optimizer(bandwidths);
        QElapsedTimer timer;
        timer.start();

        try
        {
            int resultCode = optimizer.optimize(
                &mAlgorithm,      // GTDR 实例
                mX.n_rows,            // featureCount
                maxIter,              // maxIter
                eps,                  // eps
                step                  // step
                );
            qint64 elapsed = timer.elapsed();

            if (resultCode == 0)  // GSL_SUCCESS
            {
                isOptSuccess = true;
                emit message(tr("Bandwidth optimization completed successfully in %1 ms").arg(elapsed));
                const auto& sws = mAlgorithm.spatialWeights();
                for (size_t i = 0; i < sws.size(); ++i)
                {
                    auto* bw = sws[i].weight<gwm::BandwidthWeight>();
                    if (bw)
                    {
                        QString varName = i < mMeta.weightingVariables.size()
                        ? mMeta.weightingVariables[i].name
                        : QString("Dimension_%1").arg(i);

                        emit message(tr("Dimension %1 (%2): optimized bandwidth = %3")
                                         .arg(i).arg(varName).arg(bw->bandwidth()));
                    }
                }
            }
            else
            {
                isOptSuccess = false;
                emit error(tr("Bandwidth optimization failed with code: %1").arg(resultCode));
                const auto& sws = mAlgorithm.spatialWeights();
                for (size_t i = 0; i < sws.size(); ++i)
                {
                    auto* bw = sws[i].weight<gwm::BandwidthWeight>();
                    if (bw)
                    {
                        //bw->setBandwidth(100);// 继续使用初始带宽值(这里需要实时更新)

                        // 直接使用 bandwidths 中的当前值（优化器最后一次尝试的值）
                        double currentBw = bandwidths[i]->bandwidth();

                        // 验证值的有效性
                        double lower = bw->adaptive() ? (mMeta.weightingVariables.size() + 1) : 0.0;
                        double upper = bw->adaptive() ? mX.n_rows : sws[i].distance()->maxDistance();

                        if (currentBw <= lower || currentBw >= upper || !isfinite(currentBw))
                        {
                            // 如果值无效，使用合理的默认值
                            currentBw = bw->adaptive()
                                            ? std::round(std::max(20.0, upper * 0.618))
                                            : upper * 0.618;
                            bw->setBandwidth(currentBw);
                        }
                        // 如果值有效，不需要设置（已经是当前值）

                        QString varName = i < mMeta.weightingVariables.size()
                                              ? mMeta.weightingVariables[i].name
                                              : QString("Dimension_%1").arg(i);

                        emit message(tr("Dimension %1 (%2): using initial bandwidth: %3 (optimization failed)")
                                         .arg(i).arg(varName).arg(bw->bandwidth()));
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            emit error(tr("Bandwidth optimization exception: %1").arg(e.what()));
        }

        // 7. 关闭库内部优化
        mAlgorithm.setEnableBandwidthOptimize(false);

    }
    else
    {
        emit message(tr("Bandwidth size defined."));
        mAlgorithm.setEnableBandwidthOptimize(false);

    }

    // Run algorithm;
    if (checkCanceled()) return;   
    try
    { 
        mAlgorithm.fit();
        emit message(tr("fit."));
        
        mDiagnostic=mAlgorithm.diagnostic();
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
    //emit message(tr("10"));
    int nDp = mLayer->featureCount(), nVar = indepVars.size() + 1;
    // Data layer and X,Y
    //emit message(tr("11"));
    x = mat(nDp, nVar, fill::zeros);
    y = vec(nDp, fill::zeros);
    //emit message(tr("11"));
    QgsFeatureIterator iterator = mLayer->getFeatures();
    QgsFeature f;
    bool ok = false;
    //emit message(tr("12"));
    for (int i = 0; iterator.nextFeature(f); i++)
    {
    //emit message(tr("13"));
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
    //emit message(tr("13"));

    
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
    // emit message(tr("[GTDR] result fields=%1, features=%2")
    // .arg(mResultLayer->fields().count())
    // .arg(int(mResultLayer->featureCount())));
    // // 打印前两条要素的前5个属性，避免UI阻塞
    // int cnt = 0;
    // for (auto it = mResultLayer->getFeatures(); cnt < 2 && it.nextFeature(f); ++cnt) {
    //     QStringList vals;
    //     for (int k = 0; k < std::min(5, mResultLayer->fields().count()); ++k)
    //         vals << f.attribute(k).toString();
    //     emit message(tr("[GTDR] feat%1 attrs: %2").arg(cnt).arg(vals.join(",")));
    // }
}

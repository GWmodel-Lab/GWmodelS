#include "gwmgwdataskthread.h"
#include <exception>
#include <gwmodel.h>
#include <set>
#include "SpatialWeight/gwmcrsdistance.h"
#ifdef ENABLE_OpenMP
#include <omp.h>
#endif

using namespace std;
using namespace gwm;

int GwmGWDATaskThread::treeChildCount = 0;

GwmGWDATaskThread::GwmGWDATaskThread() : GwmSpatialMonoscaleAlgorithm()
{
    // 初始化代码
}

void GwmGWDATaskThread::setDataLayer(QgsVectorLayer* layer)
{
    mLayer = layer;
    GwmSpatialAlgorithm::setDataLayer(layer);
}

void GwmGWDATaskThread::setGroupVariable(const GwmVariable& groupVar)
{
    mGroupVariable = groupVar;
}

void GwmGWDATaskThread::setIndependentVariables(const QList<GwmVariable>& indepVars)
{
    mVariables = indepVars;
}

void GwmGWDATaskThread::setSpatialWeight(const GwmSpatialWeight& spatialWeight)
{
    GwmSpatialMonoscaleAlgorithm::setSpatialWeight(spatialWeight);
}

void GwmGWDATaskThread::setIsWqda(bool isWqda)
{
    mAlgorithm.setIsWqda(isWqda);
}

void GwmGWDATaskThread::setHascov(bool hasCov)
{
    mAlgorithm.setHascov(hasCov);
}

void GwmGWDATaskThread::setHasmean(bool hasMean)
{
    mAlgorithm.setHasmean(hasMean);
}

void GwmGWDATaskThread::setHasprior(bool hasPrior)
{
    mAlgorithm.setHasprior(hasPrior);
}

void GwmGWDATaskThread::run()
{
    emit tick(0, 0);
    if (!checkCanceled())
    {
        emit message(tr("Extracting data and coordinates."));

        // 1. 提取坐标
        int nDp = mLayer->featureCount();
        arma::mat coords(nDp, 2, arma::fill::zeros);
        QgsFeatureIterator iterator = mLayer->getFeatures();
        QgsFeature f;
        int i = 0;
        while (iterator.nextFeature(f))
        {
            QgsPointXY point = f.geometry().asPoint();
            coords(i, 0) = point.x();
            coords(i, 1) = point.y();
            i++;
        }
        mAlgorithm.setCoords(coords);
        emit message(tr("Coordination extracting completed."));

        // 2. 提取自变量矩阵
        arma::mat x(nDp, mVariables.size(), arma::fill::zeros);
        iterator = mLayer->getFeatures();
        i = 0;
        while (iterator.nextFeature(f))
        {
            for (int k = 0; k < mVariables.size(); k++)
            {
                bool ok = false;
                double vX = f.attribute(mVariables[k].name).toDouble(&ok);
                if (ok)
                {
                    x(i, k) = vX;
                }
            }
            i++;
        }
        mAlgorithm.setVariables(x);
        emit message(tr("X matrix extracting completed"));

        // 3. 提取分组变量（转换为字符串）
        std::vector<std::string> y(nDp);
        iterator = mLayer->getFeatures();
        i = 0;
        while (iterator.nextFeature(f))
        {
            QVariant value = f.attribute(mGroupVariable.name);
            // 将数值转换为字符串（GWDA 需要字符串类型的分组变量）
            y[i] = value.toString().toStdString();
            i++;
        }
        mAlgorithm.setGroup(y);
        emit message(tr("Y extracting completed"));

        // 4. 设置空间权重
        GwmSpatialWeight gwmSpatialWeight = this->spatialWeight();
        // 从 GwmSpatialWeight 中提取 BandwidthWeight
        GwmBandwidthWeight* gwmBwWeight = gwmSpatialWeight.weight<GwmBandwidthWeight>();
        if (!gwmBwWeight)
        {
            emit error(tr("Invalid bandwidth weight type."));
            return;
        }

        // 创建 gwm::BandwidthWeight
        gwm::BandwidthWeight::KernelFunctionType kernelType =
            static_cast<gwm::BandwidthWeight::KernelFunctionType>(gwmBwWeight->kernel());
        gwm::BandwidthWeight bandwidthWeight(
            gwmBwWeight->bandwidth(),
            gwmBwWeight->adaptive(),
            kernelType
            );

        // 从 GwmSpatialWeight 中提取 Distance 并创建对应的 gwm::Distance
        gwm::Distance* distance = nullptr;
        GwmDistance* gwmDist = gwmSpatialWeight.distance();
        if (!gwmDist)
        {
            emit error(tr("Invalid distance type."));
            return;
        }

        switch (gwmDist->type())
        {
        case GwmDistance::CRSDistance:
        {
            GwmCRSDistance* crsDist = gwmSpatialWeight.distance<GwmCRSDistance>();
            distance = new gwm::CRSDistance(crsDist->geographic());
            break;
        }
        case GwmDistance::MinkwoskiDistance:
        {
            GwmMinkwoskiDistance* minkDist = gwmSpatialWeight.distance<GwmMinkwoskiDistance>();
            distance = new gwm::MinkwoskiDistance(minkDist->poly(), minkDist->theta());
            break;
        }
        case GwmDistance::DMatDistance:
        {
            // 注意：DMatDistance 需要文件名，需要从 GwmDMatDistance 中获取
            GwmDMatDistance* dmatDist = gwmSpatialWeight.distance<GwmDMatDistance>();
            // 需要获取文件名，可能需要添加 getter 方法
            // distance = new gwm::DMatDistance(dmatDist->filename());
            emit error(tr("DMatDistance conversion not implemented yet."));
            return;
        }
        default:
            emit error(tr("Unsupported distance type."));
            return;
        }

        // 创建 gwm::SpatialWeight 并设置到算法
        gwm::SpatialWeight spatialWeight(&bandwidthWeight, distance);
        mAlgorithm.setSpatialWeight(spatialWeight);


        // 将 GwmSpatialWeight 转换为 gwm::SpatialWeight
        // 这里需要根据实际的转换方法实现
        // mAlgorithm.setSpatialWeight(...);
        emit message(tr("Spatial weight setting completed"));
    }

    // 运行算法
    if (checkCanceled()) return;
    try
    {
        mAlgorithm.setTelegram(make_unique<GwmTaskThreadTelegram>(this));

        emit message(tr("run start"));
        mAlgorithm.run();
        emit message(tr("run completed"));

        if(!checkCanceled())
        {
            emit message(tr("Creating result layer..."));

            // 获取结果数据
            const std::vector<std::string>& groups = mAlgorithm.group();
            const arma::mat& probs = mAlgorithm.probs();
            const arma::vec& pmax = mAlgorithm.pmax();
            const arma::vec& entropy = mAlgorithm.entropy();

            // 获取类别数量
            int nClasses = probs.n_cols;
            int nDp = groups.size();

            // 获取类别名称（用于字段命名）
            std::vector<std::string> classNames;
            std::set<std::string> uniqueClasses(groups.begin(), groups.end());
            classNames.assign(uniqueClasses.begin(), uniqueClasses.end());

            // 创建结果图层
            QString layerFileName = QgsWkbTypes::displayString(mLayer->wkbType()) + QStringLiteral("?");
            QString layerName = mLayer->name() + QStringLiteral("_GWDA");

            // 避免图层名重复
            if(treeChildCount > 0)
            {
                layerName = layerName + "(" + QString::number(treeChildCount) + ")";
            }
            // 节点记录标签
            treeChildCount++;

            mResultLayer = new QgsVectorLayer(layerFileName, layerName, QStringLiteral("memory"));
            mResultLayer->setCrs(mLayer->crs());

            // 设置字段
            QgsFields fields;

            // 1. group_pred (字符串类型)
            fields.append(QgsField("group_pred", QVariant::String, QStringLiteral("varchar"), 50));

            // 2. entropy (double)
            fields.append(QgsField("entropy", QVariant::Double, QStringLiteral("double")));

            // 3. pmax (double)
            fields.append(QgsField("pmax", QVariant::Double, QStringLiteral("double")));

            // 4. logp (每个类别一个字段)
            for (int i = 0; i < nClasses; i++)
            {
                QString fieldName = QString("logp_%1").arg(QString::fromStdString(classNames[i]));
                fields.append(QgsField(fieldName, QVariant::Double, QStringLiteral("double")));
            }

            // 5. probs (每个类别一个字段)
            for (int i = 0; i < nClasses; i++)
            {
                QString fieldName = QString("probs_%1").arg(QString::fromStdString(classNames[i]));
                fields.append(QgsField(fieldName, QVariant::Double, QStringLiteral("double")));
            }

            mResultLayer->dataProvider()->addAttributes(fields.toList());
            mResultLayer->updateFields();

            // 写入数据
            mResultLayer->startEditing();
            QgsFeatureIterator iterator = mLayer->getFeatures();
            QgsFeature f;
            int i = 0;
            while (iterator.nextFeature(f) && i < nDp)
            {
                QgsFeature feature(fields);
                feature.setGeometry(f.geometry());

                int fieldIdx = 0;

                // group_pred
                feature.setAttribute(fieldIdx++, QString::fromStdString(groups[i]));

                // entropy
                feature.setAttribute(fieldIdx++, entropy(i));

                // pmax
                feature.setAttribute(fieldIdx++, pmax(i));

                // logp (需要获取 mRes，如果 GWDA 类没有 res() 方法，需要添加)
                // 注意：这里假设有 res() 方法，如果没有需要先添加
                const arma::mat& logp = mAlgorithm.res();
                for (int j = 0; j < nClasses; j++)
                {
                    feature.setAttribute(fieldIdx++, logp(i, j));
                }

                // probs
                for (int j = 0; j < nClasses; j++)
                {
                    feature.setAttribute(fieldIdx++, probs(i, j));
                }

                mResultLayer->addFeature(feature);
                i++;
            }
            mResultLayer->commitChanges();

            emit message(tr("Result layer created successfully."));
        }

        // 打印结果到控制台
        /*if(!checkCanceled())
        {
            // 1. 打印预测正确率
            double correctRate = mAlgorithm.correctRate();
            qDebug() << "=== GWDA Results ===";
            qDebug() << "Prediction Accuracy (Correct Rate):" << correctRate
                     << QString("(%1%)").arg(correctRate * 100.0, 0, 'f', 2);

            // 2. 打印分类结果（前10个点）
            const std::vector<std::string>& groups = mAlgorithm.group();
            qDebug() << "\nClassification Results (first 10 points):";
            int printCount = std::min(10, (int)groups.size());
            for (int i = 0; i < printCount; i++)
            {
                qDebug() << QString("Point %1: %2").arg(i).arg(QString::fromStdString(groups[i]));
            }
            if (groups.size() > 10)
            {
                qDebug() << QString("... (total %1 points)").arg(groups.size());
            }

            // 3. 打印概率统计信息
            const arma::mat& probs = mAlgorithm.probs();
            const arma::mat& pmax = mAlgorithm.pmax();
            const arma::mat& entropy = mAlgorithm.entropy();


            qDebug() << "\nProbability Statistics:";
            qDebug() << "Number of points:" << probs.n_rows;
            qDebug() << "Number of classes:" << probs.n_cols;
            // qDebug() << "Average max probability:" << arma::mean(pmax);
            // qDebug() << "Min max probability:" << arma::min(pmax);
            // qDebug() << "Max max probability:" << arma::max(pmax);
            // qDebug() << "Average entropy:" << arma::mean(entropy);
            // qDebug() << "Min entropy:" << arma::min(entropy);
            // qDebug() << "Max entropy:" << arma::max(entropy);

            // 4. 打印前5个点的详细概率信息
            qDebug() << "\nDetailed Probability (first 5 points):";
            int detailCount = std::min(5, (int)probs.n_rows);
            for (int i = 0; i < detailCount; i++)
            {
                QString probStr = QString("Point %1: ").arg(i);
                for (arma::uword j = 0; j < probs.n_cols; j++)
                {
                    probStr += QString("Class%1=%2 ").arg(j).arg(probs(i, j), 0, 'f', 4);
                }
                probStr += QString("(max=%1, entropy=%2)")
                               .arg(pmax(i), 0, 'f', 4)
                               .arg(entropy(i), 0, 'f', 4);
                qDebug() << probStr;
            }

            // 5. 打印类别分布
            std::map<std::string, int> classCount;
            for (const std::string& g : groups)
            {
                classCount[g]++;
            }
            qDebug() << "\nClass Distribution:";
            for (const auto& pair : classCount)
            {
                qDebug() << QString("Class %1: %2 points (%3%)")
                .arg(QString::fromStdString(pair.first))
                    .arg(pair.second)
                    .arg((double)pair.second / groups.size() * 100.0, 0, 'f', 2);
            }

            qDebug() << "=== End of GWDA Results ===";

            emit success();
            emit tick(100, 100);
        }
        */

        // 处理结果...
        if(!checkCanceled())
        {
            emit success();
            emit tick(100, 100);
        }
    }
    catch(const std::exception& e)
    {
        emit error(QString(e.what()));
    }
}

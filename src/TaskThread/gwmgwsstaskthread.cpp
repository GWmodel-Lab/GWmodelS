//#include "gwmgwsstaskthread.h"
#include "gwmgwsstaskthread.h"
#include <exception>
#include <gwmodel.h>
#include "SpatialWeight/gwmcrsdistance.h"
#ifdef ENABLE_OpenMP
#include <omp.h>
#endif

using namespace std;
using namespace gwm;

int GwmGWSSTaskThread::treeChildCount = 0;

GwmGWSSTaskThread::GwmGWSSTaskThread() : GwmSpatialMonoscaleAlgorithm()
{

}

GwmGWSSTaskThread::GwmGWSSTaskThread(const GwmAlgorithmMetaGWSS& meta) : mMeta(meta)
{
    // Check parameter
    QString metaError;
    if (!meta.validate(metaError))
    {
        throw std::bad_alloc();
    }

    mLayer = meta.layer;
    mVariables = meta.variables;

    // Spatial Weight
    BandwidthWeight weight(meta.weightBandwidthSize, meta.weightBandwidthAdaptive, meta.weightBandwidthKernel);
    Distance* distance;
    switch (meta.distanceType)
    {
    case Distance::DistanceType::CRSDistance:
        distance = new CRSDistance(meta.distanceCrsGeographic);
        break;
    case Distance::DistanceType::MinkwoskiDistance:
        distance = new MinkwoskiDistance(meta.distanceMinkowskiPower, meta.distanceMinkowskiTheta);
        break;
    default:
        distance = new CRSDistance(false);
        break;
    }
    SpatialWeight spatialWeight(&weight, distance);

    std::vector<SpatialWeight> spatialWeights;
    
    // 设置两个算法的空间权重
    mAlgorithmAverage.setSpatialWeight(spatialWeight);
    mAlgorithmCorrelation.setSpatialWeights(spatialWeights);

    // Parallel
    mAlgorithmAverage.setParallelType(meta.parallelType);
    mAlgorithmCorrelation.setParallelType(meta.parallelType);
    switch (meta.parallelType)
    {
    case gwm::ParallelType::OpenMP:
        mAlgorithmAverage.setOmpThreadNum(meta.parallelOmpThreads);
        mAlgorithmCorrelation.setOmpThreadNum(meta.parallelOmpThreads);
        break;
    default:
        break;
    }
    // Others
    mAlgorithmAverage.setQuantile(meta.quantile);
    //mAlgorithmCorrelation.setQuantile(meta.quantile);
    delete distance;
}

void GwmGWSSTaskThread::run()
{
    emit tick(0, 0);
    if (!checkCanceled())
    {
        emit message(tr("Extracting data and coordinates."));
        mat coords = initPoints(mLayer);
        mat variables = initXY(mLayer, mVariables);
        
        // 设置两个算法的坐标和变量
        mAlgorithmAverage.setCoords(coords);
        mAlgorithmAverage.setVariables(variables);
        mAlgorithmCorrelation.setCoords(coords);
        mAlgorithmCorrelation.setVariables1(variables);
        mAlgorithmCorrelation.setVariables2(variables);
    }

    // Run algorithm;
    if (checkCanceled()) return;
    try
    {
        // 运行两个算法
        mAlgorithmAverage.setTelegram(make_unique<GwmTaskThreadTelegram>(this));
        mAlgorithmAverage.run();
        
        if (checkCanceled()) return;
        
        mAlgorithmCorrelation.setTelegram(make_unique<GwmTaskThreadTelegram>(this));
        mAlgorithmCorrelation.run();
        
        if(!checkCanceled())
        {
            mResultList.push_back(qMakePair(QString("LM"), localmean()));
            mResultList.push_back(qMakePair(QString("LSD"), standarddev()));
            mResultList.push_back(qMakePair(QString("LVar"), lvar()));
            mResultList.push_back(qMakePair(QString("LSke"), localskewness()));
            mResultList.push_back(qMakePair(QString("LCV"), lcv()));
        }
        if(mAlgorithmAverage.quantile())
        {
            mResultList.push_back(qMakePair(QString("Median"), localmedian()));
            mResultList.push_back(qMakePair(QString("IQR"), iqr()));
            mResultList.push_back(qMakePair(QString("QI"), qi()));
        }
        if(mAlgorithmAverage.variables().n_cols >= 2)
        {
            mResultList.push_back(qMakePair(QString("Cov"), covmat()));
            mResultList.push_back(qMakePair(QString("Corr"), corrmat()));
            mResultList.push_back(qMakePair(QString("Spearman_rho"), scorrmat()));
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

mat GwmGWSSTaskThread::initPoints(QgsVectorLayer* layer)
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

mat GwmGWSSTaskThread::initXY(QgsVectorLayer* layer, const QList<GwmVariable> &indepVars)
{
    int nDp = layer->featureCount(), nVar = indepVars.size();
    // Data layer and X,Y
    mat x(nDp, nVar, fill::zeros);
    QgsFeatureIterator iterator = layer->getFeatures();
    QgsFeature f;
    bool ok = false;
    for (int i = 0; iterator.nextFeature(f); i++)
    {
        for (int k = 0; k < indepVars.size(); k++)
        {
            double vX = f.attribute(indepVars[k].name).toDouble(&ok);
            if (ok) x(i, k ) = vX;
            else emit error(tr("variable value cannot convert to a number. Set to 0."));
        }
    }
    return x;
}

void GwmGWSSTaskThread::createResultLayer(CreateResultLayerData data)
{
    QgsVectorLayer* srcLayer = mLayer;
    int nVar = mVariables.size();
    QString layerFileName = QgsWkbTypes::displayString(srcLayer->wkbType()) + QStringLiteral("?");
    QString layerName = srcLayer->name();
    //避免图层名重复
    if(treeChildCount > 0)
    {
        layerName += QStringLiteral("_GWSS") + "(" + QString::number(treeChildCount) + ")";
    } else
    {
        layerName += QStringLiteral("_GWSS");
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
        if (value.n_cols > nVar)
        {
            for(uword j = 0; j < nVar-1; j++)
            {
                for (uword k = j+1; k < nVar; k++)
                {
                    QString variableName = title + "_" + mVariables[j].name + "." + mVariables[k].name;
//                    QString fieldName = title.arg(variableName);
                    fields.append(QgsField(variableName, QVariant::Double, QStringLiteral("double")));
                }
            }
        }
        else
        {
            for (uword k = 0; k < value.n_cols; k++)
            {
                QString variableName = mVariables[k].name + "_" + title;
//                QString fieldName = title.arg(variableName);
                fields.append(QgsField(variableName, QVariant::Double, QStringLiteral("double")));
            }
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

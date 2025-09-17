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

}

GwmGTDRTaskThread::GwmGTDRTaskThread(const GwmAlgorithmMetaGTDR& meta) : mMeta(meta)
{
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
    vector<SpatialWeight> spatials;
    for (size_t i = 0; i < nDim; i++)
    {
        OneDimDistance distance;
        BandwidthWeight bandwidth(meta.weightBandwidthSize, meta.weightBandwidthAdaptive, meta.weightBandwidthKernel);
        spatials.push_back(SpatialWeight(&bandwidth, &distance));
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
        mAlgorithm.setIndependentVariables(mY);
        mAlgorithm.setDependentVariable(mX);
        emit message(tr("variable set"));
    }

    // Run algorithm;
    if (checkCanceled()) return;
    try
    {
        mAlgorithm.setTelegram(make_unique<GwmTaskThreadTelegram>(this));
        mAlgorithm.fit();
        mBetas = mAlgorithm.betas();
        mDiagnostic=mAlgorithm.diagnostic();
        if(!checkCanceled())
        {   
            mResultList.push_back(qMakePair(QString("%1"), mBetas));
            mResultList.push_back(qMakePair(QString("y"), mY));
            mResultList.push_back(qMakePair(QString("yhat"), mAlgorithm.Fitted(mX, mBetas)));
            mResultList.push_back(qMakePair(QString("%1_SE"), mAlgorithm.betasSE()));
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
    int nDp = mDataLayer->featureCount(), nVar = indepVars.size() + 1;
    // Data layer and X,Y
    emit message(tr("11"));
    x = mat(nDp, nVar, fill::zeros);
    y = vec(nDp, fill::zeros);
    emit message(tr("11"));
    QgsFeatureIterator iterator = mDataLayer->getFeatures();
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

}
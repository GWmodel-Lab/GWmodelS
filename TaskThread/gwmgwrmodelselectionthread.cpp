#include "gwmgwrmodelselectionthread.h"
#include "GWmodel/GWmodel.h"
#include <QDebug>

GwmGWRModelSelectionThread::GwmGWRModelSelectionThread(QgsVectorLayer* layer, GwmLayerAttributeItem* depVar, QList<GwmLayerAttributeItem*> indepVars)
    : GwmGWRTaskThread(layer,depVar,indepVars)
    , mLayer(layer)
    , mDepVar(depVar)
    , mIndepVars(indepVars)
{
    QgsFeatureIterator it = mLayer->getFeatures();
    QgsFeature feature;
    while (it.nextFeature(feature))
    {
        mFeatureIds.append(feature.id());
    }

    // 计算所需的矩阵
    mBetas = mat(indepVars.size() + 1, mFeatureIds.size());
    mDataPoints = mat(mFeatureIds.size(), 2);

    // 获取自变量和因变量所属的属性列索引
    mDepVarIndex = mDepVar->attributeIndex();
    for (GwmLayerAttributeItem* item : mIndepVars)
    {
        int iIndepVar = item->attributeIndex();
        mIndepVarsIndex.append(iIndepVar);
    }
}

void GwmGWRModelSelectionThread::run()
{
    int process = 0;
    int total = (mIndepVars.size()+1)*mIndepVars.size()/2;
    for(int i = 0; i < mIndepVars.size(); i++){
        QList<int> inDepVarsIndex = QList<int>();
        vec AICcs = vec(mIndepVars.size() - i);
        for(int j = 0; j < mIndepVars.size() - i; j++)
        {
            inDepVarsIndex.append(mIndepVarsIndex[j]);
            QList<mat> XY = setXY(mDepVarIndex,inDepVarsIndex);
            if(XY.size() == 0){
                return;
            }
            mX = XY[0];
            mY = XY[1];
            mBetas = gw_reg_all(mX,mY);
            vec s_hat = vec(mBetas.n_rows,2);
            vec aic_rss = AICcRss(mY,mX,mBetas,s_hat);
            AICcs[j] = aic_rss[2];
            QStringList inDepVarsName;
            for(int inDepVarIndex : inDepVarsIndex){
               inDepVarsName.append( mLayer->fields().field(inDepVarIndex).name());
            }
            qDebug() << inDepVarsName;
            qDebug() << AICcs[j];
            mModelInDepVars.append(inDepVarsName);
            mModelAICcs.append(AICcs[j]);
            inDepVarsIndex.removeOne(mIndepVarsIndex[j]);
            emit tick(process,total);
            process++;
        }
        int index = AICcs.index_min();
        inDepVarsIndex.append(mIndepVarsIndex[index]);
        mIndepVarsIndex.removeOne(mIndepVarsIndex[index]);
    }
    mModelInDepVars = modelSort(mModelInDepVars,mModelAICcs);
    modelSelection();
    emit success();
}

QList<mat> GwmGWRModelSelectionThread::setXY(int depVarIndex,QList<int> inDepVarsIndex)
{
    mat X = mat(mFeatureIds.size(), inDepVarsIndex.size() + 1);
    mat Y = vec(mFeatureIds.size());
    QgsField depField = mLayer->fields()[depVarIndex];
    if (!isNumeric(depField.type()))
    {
        emit error(tr("Dependent variable is not numeric."));
        return QList<mat> ();
    }
    for (int iIndepVar : inDepVarsIndex)
    {
        QgsField indepField = mLayer->fields()[iIndepVar];
        if (!isNumeric(indepField.type()))
        {
            emit error(tr("Independent variable \"") + indepField.name() + tr("\" is not numeric."));
            return QList<mat> ();
        }
    }

    int row = 0;
    bool* ok = nullptr;
    for (QgsFeatureId featureId : mFeatureIds)
    {
        QgsFeature feature = mLayer->getFeature(featureId);
        double vY = feature.attribute(depVarIndex).toDouble(ok);
        if (ok)
        {
            Y.at(row, 0) = vY;
            delete ok;
            ok = nullptr;

            // 设置 X 矩阵
            for (int index : inDepVarsIndex)
            {
                double vX = feature.attribute(index).toDouble(ok);
                if (ok)
                {
                    X.at(row, index + 1) = vX;
                    delete ok;
                    ok = nullptr;
                }
                else
                {
                    emit error(tr("Independent variable value cannot convert to a number. Set to 0."));
                }
            }
            // 设置坐标
            QgsPointXY centroPoint = feature.geometry().centroid().asPoint();
            mDataPoints.at(row, 0) = centroPoint.x();
            mDataPoints.at(row, 1) = centroPoint.y();
        }
        else
        {
            emit error(tr("Dependent variable value cannot convert to a number. Set to 0."));
        }
    }
    // 坐标旋转
    if (mDistSrcType == DistanceSourceType::Minkowski)
    {
        QMap<QString, QVariant> parameters = mDistSrcParameters.toMap();
        double theta = parameters["theta"].toDouble();
        mDataPoints = coordinateRotate(mDataPoints, theta);
    }
    return QList<mat>() << X << Y;
}

bool GwmGWRModelSelectionThread::isNumeric(QVariant::Type type)
{
    switch (type)
    {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::UInt:
    case QVariant::Double:
        return true;
    default:
        return false;
    }
}

bool GwmGWRModelSelectionThread::calDmat(){
    mDmat = mat(mFeatureIds.size(),mFeatureIds.size());
    for (int i = 0; i < mFeatureIds.size(); i++){
        QgsFeatureId id = mFeatureIds[i];
        mat dist = distance(id);
        mDmat.col(i) = dist;
//        double max = dist.max();
//        if(mBandwidthSize < max)
//        {
//            mBandwidthSize = max;
//        }
    }
    return true;
}

mat GwmGWRModelSelectionThread::gw_reg_all(mat X, mat Y)
{
    mat Betas = mat(X.n_cols, mFeatureIds.size());
//    calDmat();
    if(mBandwidthType == BandwidthType::Adaptive){
        mBandwidthSize = mFeatureIds.size();
    }
    else{
        double height = mLayer->extent().height();
        double width = mLayer->extent().width();
        mBandwidthSize = sqrt(height*height+width*width);
    }
    for (int i = 0; i < mFeatureIds.size(); i++)
    {
        QgsFeatureId id = mFeatureIds[i];
        mat dist = distance(id);
        mat weight = gwWeight(dist, mBandwidthSize, mBandwidthKernelFunction, mBandwidthType == BandwidthType::Adaptive);
        auto result = gwReg(X, Y, weight, false, i);
        Betas.col(i) = result[RegressionResult::Beta];
//        emit tick(i, mFeatureIds.size());
    }
    return Betas;
}

vec GwmGWRModelSelectionThread::distance(const QgsFeatureId &id)
{
    switch (mDistSrcType)
    {
    case DistanceSourceType::Minkowski:
        return distanceMinkowski(id);
    default:
        return distanceCRS(id);
    }
}

vec GwmGWRModelSelectionThread::distanceCRS(const QgsFeatureId &id)
{
    QgsFeature fSrc = mLayer->getFeature(id);
    QgsGeometry gSrc = fSrc.geometry();
    vec dist(mFeatureIds.size(), fill::zeros);
    for (int i = 0; i < mFeatureIds.size(); i++)
    {
        QgsFeature fDes = mLayer->getFeature(mFeatureIds[i]);
        dist.at(i) = fDes.geometry().distance(gSrc);
    }
    return dist;
}

vec GwmGWRModelSelectionThread::distanceMinkowski(const QgsFeatureId &id)
{
    QMap<QString, QVariant> parameters = mDistSrcParameters.toMap();
    double p = parameters["p"].toDouble();
    QgsFeature fSrc = mLayer->getFeature(id);
    QgsPointXY gSrc = fSrc.geometry().centroid().asPoint();
    vec srcLoc(gSrc.x(), gSrc.y());
    return mkDistVec(mDataPoints, srcLoc, p);
}

QList<QStringList> GwmGWRModelSelectionThread::modelSort(QList<QStringList> modelList, QList<double> modelAICcs)
{
    int tag = 0;
    QList<int> sortIndex;
    for(int i = mIndepVars.size(); i > 0; i--){
        vec tmpList = vec(i);
        for(int j = tag; j < tag + i; j++){
            tmpList.row(j - tag) = modelAICcs[j];
        }
        for(int j = 0; j < i; j++){
            int index = tmpList.index_max();
            sortIndex.append(index + tag);
            tmpList.row(index) = 0;
        }
    }
    QList<QStringList> res;
    mModelAICcs.clear();
    for(int i = 0; i < modelList.size(); i++){
        res.append(modelList[i]);
        mModelAICcs.append(modelAICcs[i]);
        for(QString inDepVar:modelList[i]){
            emit message(inDepVar);
        }
        emit message(QString::number(modelAICcs[i],10,5));
        qDebug() << modelList[i];
        qDebug() << modelAICcs[i];
    }
    return res;
}

QMap<QStringList,double> GwmGWRModelSelectionThread::modelSelection(){
    for(int i = mModelInDepVars.size() - 1; i >= 0; i--){
        if(mModelAICcs[i-1] - mModelAICcs[i] >= 3){
            QMap<QStringList,double> res;
            res.insert(mModelInDepVars[i],mModelAICcs[i]);
            for(QString inDepVar:mModelInDepVars[i]){
                emit message(inDepVar);
            }
            emit message(QString::number(mModelAICcs[i],10,5));
            qDebug() << res;
            return res;
        }
    }
    return QMap<QStringList,double>();
}




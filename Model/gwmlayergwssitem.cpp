#include "gwmlayergwssitem.h"
#include "gwmlayergroupitem.h"


GwmLayerGWSSItem::GwmLayerGWSSItem(GwmLayerItem* parentItem, QgsVectorLayer* vector, const GwmGWSSTaskThread* taskThread)
    : GwmLayerVectorItem(parentItem, vector)
{
    if (taskThread)
    {
        mDataPointsSize = taskThread->dataPointsSize();
        mVariables = taskThread->variables();
        mBandwidth = taskThread->bandwidth();
        mQuantile = taskThread->quantile();
        mResultList = taskThread->resultlist();

        mLocalMean = taskThread->localmean();
        mStandardDev = taskThread->standarddev();
        mLocalSkewness = taskThread->localskewness();
        mLCV = taskThread->lcv();
        mLVar = taskThread->lvar();

        if(mQuantile){
            mLocalMedian = taskThread->localmedian();
            mIQR = taskThread->iqr();
            mQI = taskThread->qi();
        }

        if(mVariables.size() >= 2){
            mCovmat = taskThread->covmat();
            mCorrmat = taskThread->corrmat();
            mSCorrmat = taskThread->scorrmat();
        }
    }
    else
    {
        //todo:修改回来
//        mBandwidth = new GwmBandwidthWeight();
    }
}

GwmLayerGWSSItem::GwmLayerGWSSItem(GwmLayerItem* parentItem, QgsVectorLayer* vector, const GwmGWcorrelationTaskThread* taskThread)
    : GwmLayerVectorItem(parentItem, vector)
{
    if (taskThread)
    {
        mDataPointsSize = taskThread->dataPointsSize();
        mVariables = taskThread->variables();
        //todo:修改回来
//        mBandwidth = taskThread->bandwidth();
        mQuantile = taskThread->quantile();
        mResultList = taskThread->resultlist();

        if(mVariables.size() >= 2){
            mCovmat = taskThread->covmat();
            mCorrmat = taskThread->corrmat();
            mSCorrmat = taskThread->scorrmat();
        }
    }
    else
    {
        mBandwidth = new GwmBandwidthWeight();
    }
}

GwmLayerGWSSItem::GwmLayerGWSSItem(GwmLayerItem* parentItem, QgsVectorLayer* vector, const GwmGWaverageTaskThread* taskThread)
    : GwmLayerVectorItem(parentItem, vector)
{
    if (taskThread)
    {
        mDataPointsSize = taskThread->dataPointsSize();
        mVariables = taskThread->variables();
        mBandwidth = taskThread->bandwidth();
        mQuantile = taskThread->quantile();
        mResultList = taskThread->resultlist();

        mLocalMean = taskThread->localmean();
        mStandardDev = taskThread->standarddev();
        mLocalSkewness = taskThread->localskewness();
        mLCV = taskThread->lcv();
        mLVar = taskThread->lvar();

        if(mQuantile){
            mLocalMedian = taskThread->localmedian();
            mIQR = taskThread->iqr();
            mQI = taskThread->qi();
        }

        if(mVariables.size() >= 2){
            mCovmat = taskThread->covmat();
            mCorrmat = taskThread->corrmat();
            mSCorrmat = taskThread->scorrmat();
        }
    }
    else
    {
        mBandwidth = new GwmBandwidthWeight();
    }
}

GwmLayerGWSSItem::~GwmLayerGWSSItem()
{
    if (mBandwidth)
        delete mBandwidth;
}

int GwmLayerGWSSItem::childNumber()
{
    if (mParentItem)
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this) + 1;
    return 0;
}

bool GwmLayerGWSSItem::readXml(QDomNode &node)
{
    if (GwmLayerVectorItem::readXml(node))
    {
        mDataPointsSize = mLayer->featureCount();

        QDomElement analyse = node.toElement();
        mQuantile = analyse.attribute("quantile").toInt();

        QDomElement nodeIndepVarList = analyse.firstChildElement("variableList");
        if (!nodeIndepVarList.isNull())
        {
            QDomElement nodeVariable = nodeIndepVarList.firstChildElement("variable");
            while (!nodeVariable.isNull())
            {
                if (nodeVariable.hasAttribute("name") && nodeVariable.hasAttribute("index")
                        && nodeVariable.hasAttribute("isNumeric")
                        && nodeVariable.hasAttribute("type"))
                {
                    GwmVariable variable;
                    variable.name = nodeVariable.attribute("name");
                    variable.index = nodeVariable.attribute("index").toInt();
                    variable.isNumeric = nodeVariable.attribute("isNumeric").toInt();
                    variable.type = QVariant::Type(nodeVariable.attribute("type").toInt());
                    mVariables.append(variable);
                }
                nodeVariable = nodeVariable.nextSiblingElement("variable");
            }
        }
        else return false;

        QDomElement weightNode = analyse.firstChildElement("weight");
        if (weightNode.hasAttribute("bandwidth") && weightNode.hasAttribute("kernel")
                && weightNode.hasAttribute("adaptive"))
        {
            double bandwidth = weightNode.attribute("bandwidth").toDouble();
            bool adaptive = weightNode.attribute("adaptive").toInt();
            GwmBandwidthWeight::KernelFunctionType kernel = GwmBandwidthWeight::KernelFunctionTypeNameMapper.value(weightNode.attribute("kernel"));
            mBandwidth = new GwmBandwidthWeight(bandwidth, adaptive, kernel);
        }
        else return false;

        // 提取平均数
        if (mLayer)
        {
            int nVar = mVariables.size(), nVar2 = (nVar + 1) * nVar / 2 - nVar;
            mLocalMean = mat(mDataPointsSize, nVar, fill::zeros);
            mStandardDev = mat(mDataPointsSize, nVar, fill::zeros);
            mLVar = mat(mDataPointsSize, nVar, fill::zeros);
            mLocalSkewness = mat(mDataPointsSize, nVar, fill::zeros);
            mLCV = mat(mDataPointsSize, nVar, fill::zeros);
            if (mQuantile)
            {
                mLocalMedian = mat(mDataPointsSize, nVar, fill::zeros);
                mIQR = mat(mDataPointsSize, nVar, fill::zeros);
                mQI = mat(mDataPointsSize, nVar, fill::zeros);
            }
            if (nVar >= 2)
            {
                mCovmat = mat(mDataPointsSize, nVar2, fill::zeros);
                mCorrmat = mat(mDataPointsSize, nVar2, fill::zeros);
                mSCorrmat = mat(mDataPointsSize, nVar2, fill::zeros);
            }

            QgsFeatureIterator iterator = mLayer->getFeatures();
            QgsFeature f;
            for (int i = 0; i < mDataPointsSize && iterator.nextFeature(f); i++)
            {
                for (int k = 0; k < nVar; k++)
                {
                    QString nameLMean = QString("%1_LM").arg(mVariables[k].name);
                    QString nameLSD = QString("%1_LSD").arg(mVariables[k].name);
                    QString nameLVar = QString("%1_LVar").arg(mVariables[k].name);
                    QString nameLSke = QString("%1_LSke").arg(mVariables[k].name);
                    QString nameLCV = QString("%1_LCV").arg(mVariables[k].name);
                    mLocalMean(i, k) = f.attribute(nameLMean).toDouble();
                    mStandardDev(i, k) = f.attribute(nameLSD).toDouble();
                    mLVar(i, k) = f.attribute(nameLVar).toDouble();
                    mLocalSkewness(i, k) = f.attribute(nameLSke).toDouble();
                    mLCV(i, k) = f.attribute(nameLCV).toDouble();
                    if (mQuantile)
                    {
                        QString nameLMedian = QString("%1_Median").arg(mVariables[k].name);
                        QString nameIQR = QString("%1_IQR").arg(mVariables[k].name);
                        QString nameQI = QString("%1_QI").arg(mVariables[k].name);
                        mLocalMean(i, k) = f.attribute(nameLMedian).toDouble();
                        mIQR(i, k) = f.attribute(nameIQR).toDouble();
                        mQI(i, k) = f.attribute(nameQI).toDouble();
                    }
                }
                if (nVar >= 2)
                {
                    int tag = 0;
                    for (int j = 0; j < nVar - 1; j++)
                    {
                        for (int k = j + 1; k < nVar; k++)
                        {
                            QString nameCov = QString("Cov_%1.%2").arg(mVariables[k].name);
                            QString nameCorr = QString("Corr_%1.%2").arg(mVariables[k].name);
                            QString nameSCorr = QString("Spearman_rho_%1.%2").arg(mVariables[k].name);
                            mCovmat(i, tag) = f.attribute(nameCov).toDouble();
                            mCorrmat(i, tag) = f.attribute(nameCorr).toDouble();
                            mSCorrmat(i, tag) = f.attribute(nameSCorr).toDouble();
                        }
                    }
                }
            }

            GwmGWSSTaskThread::CreateResultLayerData resultLayerData;
            resultLayerData.push_back(qMakePair(QString("LM"), mLocalMean));
            resultLayerData.push_back(qMakePair(QString("LSD"), mStandardDev));
            resultLayerData.push_back(qMakePair(QString("LVar"), mLVar));
            resultLayerData.push_back(qMakePair(QString("LSke"), mLocalSkewness));
            resultLayerData.push_back(qMakePair(QString("LCV"), mLCV));
            if(mQuantile){
                resultLayerData.push_back(qMakePair(QString("Median"), mLocalMedian));
                resultLayerData.push_back(qMakePair(QString("IQR"), mIQR));
                resultLayerData.push_back(qMakePair(QString("QI"), mQI));
            }
            if(nVar >= 2){
                resultLayerData.push_back(qMakePair(QString("Cov"), mCovmat));
                resultLayerData.push_back(qMakePair(QString("Corr"), mCorrmat));
                resultLayerData.push_back(qMakePair(QString("Spearman_rho"), mSCorrmat));
            }
            mResultList = resultLayerData;
        }
    }
    else return false;
}

bool GwmLayerGWSSItem::writeXml(QDomNode &node, QDomDocument &doc)
{
    if (GwmLayerVectorItem::writeXml(node, doc))
    {
        QDomElement nodeAnalyse = node.toElement();
        nodeAnalyse.setAttribute("quantile", mQuantile);
        nodeAnalyse.setAttribute("dataPointSize", mDataPointsSize);

        QDomElement nodeVariableList = doc.createElement("variableList");
        for (auto v : mVariables)
        {
            QDomElement nodeVariable = doc.createElement("variable");
            nodeVariable.setAttribute("index", v.index);
            nodeVariable.setAttribute("isNumeric", v.isNumeric);
            nodeVariable.setAttribute("name", v.name);
            nodeVariable.setAttribute("type", int(v.type));
            nodeVariableList.appendChild(nodeVariable);
        }
        nodeAnalyse.appendChild(nodeVariableList);

        QDomElement nodeBandwidth = doc.createElement("weight");
        nodeBandwidth.setAttribute("kernel", GwmBandwidthWeight::KernelFunctionTypeNameMapper.name(mBandwidth->kernel()));
        nodeBandwidth.setAttribute("bandwidth", mBandwidth->bandwidth());
        nodeBandwidth.setAttribute("adaptive", mBandwidth->adaptive());
        nodeAnalyse.appendChild(nodeBandwidth);
    }
    else return false;
}

int GwmLayerGWSSItem::dataPointsSize() const
{
    return mDataPointsSize;
}

#include "gwmlayergwdaitem.h"
#include "gwmlayergroupitem.h"
#include <armadillo>

using namespace arma;

GwmLayerGWDAItem::GwmLayerGWDAItem(GwmLayerItem* parentItem, QgsVectorLayer* vector, const GwmGWDATaskThread* taskThread)
    : GwmLayerVectorItem(parentItem, vector)
    , mDataPointsSize(0)
    , mCorrectRate(0.0)
    , mIsWqda(false)
    , mHasCov(false)
    , mHasMean(false)
    , mHasPrior(false)
    , mBandwidth(nullptr)
{
    if (taskThread)
    {
        // 获取空间权重
        GwmSpatialWeight spatialWeight = taskThread->spatialWeight();
        GwmBandwidthWeight* gwmBwWeight = spatialWeight.weight<GwmBandwidthWeight>();
        if (gwmBwWeight)
        {
            mWeight = *gwmBwWeight;
            mBandwidth = new GwmBandwidthWeight(*gwmBwWeight);
        }
        else
        {
            mBandwidth = new GwmBandwidthWeight();
        }

        // 获取数据点数量
        if (mLayer)
        {
            mDataPointsSize = mLayer->featureCount();
        }
        else if (taskThread->dataLayer())
        {
            mDataPointsSize = taskThread->dataLayer()->featureCount();
        }

        // 获取变量信息
        mGroupVariable = taskThread->groupVariable();
        // 注意：需要从 taskThread 获取分组变量和独立变量
        // 如果 taskThread 没有提供这些方法，可能需要添加 getter 方法
        mIndependentVariables = taskThread->independentVariables();

        // 获取算法参数（需要添加 getter 方法到 GwmGWDATaskThread）
        mIsWqda = taskThread->isWqda();
        mHasCov = taskThread->hasCov();
        mHasMean = taskThread->hasMean();
        mHasPrior = taskThread->hasPrior();

        // 获取正确率（需要添加 getter 方法到 GwmGWDATaskThread）
        mCorrectRate = taskThread->correctRate();
    }
    else
    {
        mBandwidth = new GwmBandwidthWeight();
    }
}

GwmLayerGWDAItem::~GwmLayerGWDAItem()
{
    if (mBandwidth)
        delete mBandwidth;
}

int GwmLayerGWDAItem::childNumber()
{
    if (mParentItem)
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this) + 1;
    return 0;
}

bool GwmLayerGWDAItem::readXml(QDomNode &node)
{
    if (GwmLayerVectorItem::readXml(node))
    {
        mDataPointsSize = mLayer ? mLayer->featureCount() : 0;

        QDomElement analyse = node.toElement();
        mIsWqda = analyse.attribute("isWqda").toInt();
        mHasCov = analyse.attribute("hasCov").toInt();
        mHasMean = analyse.attribute("hasMean").toInt();
        mHasPrior = analyse.attribute("hasPrior").toInt();
        mCorrectRate = analyse.attribute("correctRate").toDouble();

        // 读取分组变量
        QDomElement nodeGroupVar = analyse.firstChildElement("groupVar");
        if (!nodeGroupVar.isNull())
        {
            if (nodeGroupVar.hasAttribute("name") && nodeGroupVar.hasAttribute("index")
                && nodeGroupVar.hasAttribute("isNumeric")
                && nodeGroupVar.hasAttribute("type"))
            {
                mGroupVariable.name = nodeGroupVar.attribute("name");
                mGroupVariable.index = nodeGroupVar.attribute("index").toInt();
                mGroupVariable.isNumeric = nodeGroupVar.attribute("isNumeric").toInt();
                mGroupVariable.type = QVariant::Type(nodeGroupVar.attribute("type").toInt());
            }
        }

        // 读取独立变量列表
        QDomElement indepVarList = analyse.firstChildElement("indepVarList");
        if (!indepVarList.isNull())
        {
            QDomElement indepVarNode = indepVarList.firstChildElement("indepVar");
            while (!indepVarNode.isNull())
            {
                if (indepVarNode.hasAttribute("name") && indepVarNode.hasAttribute("index")
                    && indepVarNode.hasAttribute("isNumeric")
                    && indepVarNode.hasAttribute("type"))
                {
                    GwmVariable indepVar;
                    indepVar.name = indepVarNode.attribute("name");
                    indepVar.index = indepVarNode.attribute("index").toInt();
                    indepVar.isNumeric = indepVarNode.attribute("isNumeric").toInt();
                    indepVar.type = QVariant::Type(indepVarNode.attribute("type").toInt());
                    mIndependentVariables.append(indepVar);
                }
                indepVarNode = indepVarNode.nextSiblingElement("indepVar");
            }
        }

        // 读取空间权重
        QDomElement weightNode = analyse.firstChildElement("weight");
        if (weightNode.hasAttribute("bandwidth") && weightNode.hasAttribute("kernel")
            && weightNode.hasAttribute("adaptive"))
        {
            double bandwidth = weightNode.attribute("bandwidth").toDouble();
            bool adaptive = weightNode.attribute("adaptive").toInt();
            GwmBandwidthWeight::KernelFunctionType kernel = GwmBandwidthWeight::KernelFunctionTypeNameMapper.value(weightNode.attribute("kernel"));
            mWeight = GwmBandwidthWeight(bandwidth, adaptive, kernel);
            if (mBandwidth)
                delete mBandwidth;
            mBandwidth = new GwmBandwidthWeight(bandwidth, adaptive, kernel);
        }

        return true;
    }
    return false;
}

bool GwmLayerGWDAItem::writeXml(QDomNode &node, QDomDocument &doc)
{
    if (GwmLayerVectorItem::writeXml(node, doc))
    {
        QDomElement nodeAnalyse = node.toElement();
        nodeAnalyse.setAttribute("dataPointSize", mDataPointsSize);
        nodeAnalyse.setAttribute("isWqda", mIsWqda);
        nodeAnalyse.setAttribute("hasCov", mHasCov);
        nodeAnalyse.setAttribute("hasMean", mHasMean);
        nodeAnalyse.setAttribute("hasPrior", mHasPrior);
        nodeAnalyse.setAttribute("correctRate", mCorrectRate);

        // 写入分组变量
        QDomElement nodeGroupVar = doc.createElement("groupVar");
        nodeGroupVar.setAttribute("index", mGroupVariable.index);
        nodeGroupVar.setAttribute("isNumeric", mGroupVariable.isNumeric);
        nodeGroupVar.setAttribute("name", mGroupVariable.name);
        nodeGroupVar.setAttribute("type", int(mGroupVariable.type));
        nodeAnalyse.appendChild(nodeGroupVar);

        // 写入独立变量列表
        QDomElement nodeIndepVarList = doc.createElement("indepVarList");
        for (auto indepVar : mIndependentVariables)
        {
            QDomElement nodeIndep = doc.createElement("indepVar");
            nodeIndep.setAttribute("index", indepVar.index);
            nodeIndep.setAttribute("isNumeric", indepVar.isNumeric);
            nodeIndep.setAttribute("name", indepVar.name);
            nodeIndep.setAttribute("type", int(indepVar.type));
            nodeIndepVarList.appendChild(nodeIndep);
        }
        nodeAnalyse.appendChild(nodeIndepVarList);

        // 写入空间权重
        QDomElement nodeBandwidth = doc.createElement("weight");
        nodeBandwidth.setAttribute("kernel", GwmBandwidthWeight::KernelFunctionTypeNameMapper.name(mWeight.kernel()));
        nodeBandwidth.setAttribute("bandwidth", mWeight.bandwidth());
        nodeBandwidth.setAttribute("adaptive", mWeight.adaptive());
        nodeAnalyse.appendChild(nodeBandwidth);

        return true;
    }
    return false;
}

int GwmLayerGWDAItem::dataPointsSize() const
{
    return mDataPointsSize;
}

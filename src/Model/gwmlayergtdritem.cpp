#include "gwmlayergtdritem.h"
#include "gwmlayergroupitem.h"


GwmLayerGTDRItem::GwmLayerGTDRItem(GwmLayerItem* parentItem, QgsVectorLayer* vector, const GwmGTDRTaskThread* taskThread)
    : GwmLayerVectorItem(parentItem, vector)
{
    if (taskThread)
    {
        auto taskMeta = taskThread->meta();
        mBandwidth = new GwmBandwidthWeight(taskMeta.weightBandwidthSize, taskMeta.weightBandwidthAdaptive, GwmBandwidthWeight::KernelFunctionType(taskMeta.weightBandwidthKernel));

        const auto& sws = taskThread->algorithm().spatialWeights();
        mBandwidths.clear();
        mBandwidths.reserve(sws.size());
        for(int i = 0; i < sws.size(); ++i){
            // 如果无法获取，使用 taskMeta 的默认值
            // auto* appBw = new GwmBandwidthWeight(
            //     taskMeta.weightBandwidthSize,
            //     taskMeta.weightBandwidthAdaptive,
            //     static_cast<GwmBandwidthWeight::KernelFunctionType>(taskMeta.weightBandwidthKernel)
            //     );
            // mBandwidths.append(appBw);
            double metaBwSize = (i < taskMeta.weightBandwidthSizes.size())
                                    ? taskMeta.weightBandwidthSizes[i]
                                    : taskMeta.weightBandwidthSize;
            gwm::BandwidthWeight::KernelFunctionType metaKernel = (i < taskMeta.weightBandwidthKernels.size())
                                                                      ? taskMeta.weightBandwidthKernels[i]
                                                                      : taskMeta.weightBandwidthKernel;

            auto* appBw = new GwmBandwidthWeight(
                metaBwSize,  // 使用每个维度对应的带宽值
                taskMeta.weightBandwidthAdaptive,
                static_cast<GwmBandwidthWeight::KernelFunctionType>(metaKernel)  // 使用每个维度对应的核函数
                );
            mBandwidths.append(appBw);
        };

        // 获取优化后的带宽值（如果有优化）
        if (taskMeta.bandwidthAuto)
        {
            // update mBandwidth
            if (!sws.empty())
            {
                auto* optimizedBw = sws[0].weight<gwm::BandwidthWeight>();
                if (optimizedBw)
                {
                    // 更新为优化后的带宽值
                    mBandwidth->setBandwidth(optimizedBw->bandwidth());
                    mBandwidth->setAdaptive(optimizedBw->adaptive());
                    mBandwidth->setKernel(static_cast<GwmBandwidthWeight::KernelFunctionType>(optimizedBw->kernel()));
                    isBandwidthOptimized = true;
                }
            }
            // update mBandwidths
            mBandwidths.clear();
            mBandwidths.reserve(sws.size());
            for(int i = 0; i < sws.size(); ++i){
                // 从 taskMeta 获取初始值，或从优化后的 spatialWeight 获取
                auto* libBw = sws[i].weight<gwm::BandwidthWeight>();
                if (libBw)
                {
                    // 创建应用层的带宽权重对象
                    auto* appBw = new GwmBandwidthWeight(
                        libBw->bandwidth(),
                        libBw->adaptive(),
                        static_cast<GwmBandwidthWeight::KernelFunctionType>(libBw->kernel())
                        );
                    mBandwidths.append(appBw);
                }
                else
                {
                    // 如果无法获取，使用 taskMeta 的默认值
                    auto* appBw = new GwmBandwidthWeight(
                        taskMeta.weightBandwidthSize,
                        taskMeta.weightBandwidthAdaptive,
                        static_cast<GwmBandwidthWeight::KernelFunctionType>(taskMeta.weightBandwidthKernel)
                        );
                    mBandwidths.append(appBw);
                }
            }
        }

        mDataPointsSize = taskMeta.layer->featureCount();
        mDepVar = taskMeta.dependentVariable;
        mIndepVars = taskMeta.independentVariables;
        hasHatmatrix = taskThread->hasHatMatrix();
        mBetas = mat(taskThread->betas());
        mDiagnostic = taskThread->diagnostic();
        // isBandwidthOptimized = taskThread->isAutoselectBandwidth();
        // mBandwidthSelScores = taskThread->bandwidthSelectorCriterions();
        // isRegressionPointGiven = !(taskThread->regressionLayer() == nullptr);
    }
    else
    {
        mBandwidth = new GwmBandwidthWeight();
    }
}

GwmLayerGTDRItem::~GwmLayerGTDRItem()
{
    if (mBandwidth)
        delete mBandwidth;

    // 释放 mBandwidths 中的所有对象
    for (auto* bw : mBandwidths)
    {
        if (bw)
            delete bw;
    }
    mBandwidths.clear();
}

int GwmLayerGTDRItem::childNumber()
{
    if (mParentItem)
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this) + 1;
    return 0;
}

bool GwmLayerGTDRItem::readXml(QDomNode &node)
{
    if (GwmLayerVectorItem::readXml(node))
    {
        mDataPointsSize = mLayer->featureCount();

        QDomElement analyse = node.toElement();
        hasHatmatrix = analyse.attribute("hasHatmatrix").toInt();
        isRegressionPointGiven = analyse.attribute("isRegressionPointGiven").toInt();
        isBandwidthOptimized = analyse.attribute("isBandwidthOptimized").toInt();

        QDomElement nodeDepVar = analyse.firstChildElement("depVar");
        if (nodeDepVar.isNull())
            return false;
        if (nodeDepVar.hasAttribute("name") && nodeDepVar.hasAttribute("index")
                && nodeDepVar.hasAttribute("isNumeric")
                && nodeDepVar.hasAttribute("type"))
        {
            mDepVar.name = nodeDepVar.attribute("name");
            mDepVar.index = nodeDepVar.attribute("index").toInt();
            mDepVar.isNumeric = nodeDepVar.attribute("isNumeric").toInt();
            mDepVar.type = QVariant::Type(nodeDepVar.attribute("type").toInt());
        }
        else return false;

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
                    mIndepVars.append(indepVar);
                }
                indepVarNode = indepVarNode.nextSiblingElement("indepVar");
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
            mWeight = GwmBandwidthWeight(bandwidth, adaptive, kernel);
        }
        else return false;

        // 读取回归系数表
        if (mLayer)
        {
            mBetas = mat(mDataPointsSize, mIndepVars.size() + 1, fill::zeros);
            QgsFeatureIterator iterator = mLayer->getFeatures();
            QgsFeature f;
            for (int i = 0; i < mDataPointsSize && iterator.nextFeature(f); i++)
            {
                mBetas(i, 0) = f.attribute("Intercept").toDouble();
                for (int k = 0; k < mIndepVars.size(); k++)
                {
                    int c = k + 1;
                    mBetas(i, c) = f.attribute(mIndepVars[k].name).toDouble();
                }
            }
        }

        if (hasHatmatrix)
        {
            QDomElement diagnosticNode = analyse.firstChildElement("diagnostic");
            if (!diagnosticNode.isNull())
            {
                mDiagnostic.RSS = diagnosticNode.attribute("RSS").toDouble();
                mDiagnostic.RSquareAdjust = diagnosticNode.attribute("RSquareAdjust").toDouble();
                mDiagnostic.ENP = diagnosticNode.attribute("ENP").toDouble();
                mDiagnostic.EDF = diagnosticNode.attribute("EDF").toDouble();
                mDiagnostic.AIC = diagnosticNode.attribute("AIC").toDouble();
                mDiagnostic.AICc = diagnosticNode.attribute("AICc").toDouble();
                mDiagnostic.RSquare = diagnosticNode.attribute("RSquare").toDouble();
            }
            else return false;
        }

        if (isBandwidthOptimized)
        {
            QDomElement bandwidthCriterionsNode = node.firstChildElement("bandwidthCriterions");
            if (!bandwidthCriterionsNode.isNull())
            {
                QDomElement bandwidthNode = bandwidthCriterionsNode.firstChildElement("bandwidth");
                while (!bandwidthNode.isNull())
                {
                    if (bandwidthNode.hasAttribute("size") && bandwidthNode.hasAttribute("criterion"))
                    {
                        double size = bandwidthNode.attribute("size").toDouble();
                        double criterion = bandwidthNode.attribute("criterion").toDouble();
                        mBandwidthSelScores.append(qMakePair(size, criterion));
                    }
                    bandwidthNode = bandwidthNode.nextSiblingElement("bandwidth");
                }
            }
            else
            {
                isBandwidthOptimized = false;
            }
        }

        return true;
    }
    else return false;
}

bool GwmLayerGTDRItem::writeXml(QDomNode &node, QDomDocument &doc)
{
    if (GwmLayerVectorItem::writeXml(node, doc))
    {
        QDomElement nodeAnalyse = node.toElement();
        nodeAnalyse.setAttribute("dataPointSize", mDataPointsSize);
        nodeAnalyse.setAttribute("isBandwidthOptimized", isBandwidthOptimized);
        nodeAnalyse.setAttribute("hasHatmatrix", hasHatmatrix);
        nodeAnalyse.setAttribute("isRegressionPointGiven", isRegressionPointGiven);

        QDomElement nodeDepVar = doc.createElement("depVar");
        nodeDepVar.setAttribute("index", mDepVar.index);
        nodeDepVar.setAttribute("isNumeric", mDepVar.isNumeric);
        nodeDepVar.setAttribute("name", mDepVar.name);
        nodeDepVar.setAttribute("type", int(mDepVar.type));
        nodeAnalyse.appendChild(nodeDepVar);

        QDomElement nodeIndepVarList = doc.createElement("indepVarList");
        for (auto indepVar : mIndepVars)
        {
            QDomElement nodeIndep = doc.createElement("indepVar");
            nodeIndep.setAttribute("index", indepVar.index);
            nodeIndep.setAttribute("isNumeric", indepVar.isNumeric);
            nodeIndep.setAttribute("name", indepVar.name);
            nodeIndep.setAttribute("type", int(indepVar.type));
            nodeIndepVarList.appendChild(nodeIndep);
        }
        nodeAnalyse.appendChild(nodeIndepVarList);

        QDomElement nodeBandwidth = doc.createElement("weight");
        nodeBandwidth.setAttribute("kernel", GwmBandwidthWeight::KernelFunctionTypeNameMapper.name(mWeight.kernel()));
        nodeBandwidth.setAttribute("bandwidth", mWeight.bandwidth());
        nodeBandwidth.setAttribute("adaptive", mWeight.adaptive());
        nodeAnalyse.appendChild(nodeBandwidth);

        if (hasHatmatrix)
        {
            QDomElement nodeDiagnostic = doc.createElement("diagnostic");
            nodeDiagnostic.setAttribute("RSS", mDiagnostic.RSS);
            nodeDiagnostic.setAttribute("AIC", mDiagnostic.AIC);
            nodeDiagnostic.setAttribute("AICc", mDiagnostic.AICc);
            nodeDiagnostic.setAttribute("ENP", mDiagnostic.ENP);
            nodeDiagnostic.setAttribute("EDF", mDiagnostic.EDF);
            nodeDiagnostic.setAttribute("RSquare", mDiagnostic.RSquare);
            nodeDiagnostic.setAttribute("RSquareAdjust", mDiagnostic.RSquareAdjust);
            nodeAnalyse.appendChild(nodeDiagnostic);
        }

        if (isBandwidthOptimized)
        {
            QDomElement nodeBandwidthCriterion = doc.createElement("bandwidthCriterions");
            for (auto bandwidth : mBandwidthSelScores)
            {
                QDomElement nodeBandwidth = doc.createElement("bandwidth");
                nodeBandwidth.setAttribute("size", bandwidth.first);
                nodeBandwidth.setAttribute("criterion", bandwidth.second);
                nodeBandwidthCriterion.appendChild(nodeBandwidth);
            }
            nodeAnalyse.appendChild(nodeBandwidthCriterion);
        }

        return true;
    }
    return false;
}

int GwmLayerGTDRItem::dataPointsSize() const
{
    return mDataPointsSize;
}

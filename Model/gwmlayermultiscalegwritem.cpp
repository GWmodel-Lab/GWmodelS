#include "gwmlayermultiscalegwritem.h"
#include "gwmlayergroupitem.h"

GwmLayerMultiscaleGWRItem::GwmLayerMultiscaleGWRItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmMultiscaleGWRAlgorithm* taskThread)
    : GwmLayerVectorItem(parent, vector)
{
    if (taskThread)
    {
        mDataPointsSize = taskThread->dataLayer()->featureCount();
        mDepVar = taskThread->dependentVariable();
        mIndepVars = taskThread->independentVariables();
        for (const GwmSpatialWeight& sp : taskThread->spatialWeights())
        {
            GwmBandwidthWeight* pBw = static_cast<GwmBandwidthWeight*>(sp.weight()->clone());
            GwmBandwidthWeight bw(pBw);
            mBandwidthWeights.append(bw);
            mDistaneTypes.append(sp.distance()->type());
        }
        mBandwidthInitilize = taskThread->bandwidthInitilize();
        mBandwidthSelectionApproach = taskThread->bandwidthSelectionApproach();
        mPreditorCentered = taskThread->preditorCentered();
        mBandwidthSelectThreshold = taskThread->bandwidthSelectThreshold();
        mCriterionType = taskThread->criterionType();
        mDiagnostic = taskThread->diagnostic();
        mBetas = mat(taskThread->betas());
        mHasHatmatrix = taskThread->hasHatMatrix();
    }
}

int GwmLayerMultiscaleGWRItem::childNumber()
{
    if (mParentItem)
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this) + 1;
    return 0;
}

bool GwmLayerMultiscaleGWRItem::readXml(QDomNode &node)
{
    if (GwmLayerVectorItem::readXml(node))
    {
        mDataPointsSize = mLayer->featureCount();

        QDomElement analyse = node.toElement();
        mHasHatmatrix = analyse.attribute("hasHatmatrix").toInt();
        mCriterionType = GwmMultiscaleGWRAlgorithm::BackFittingCriterionType(analyse.attribute("criterionType").toInt());

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

        if (mHasHatmatrix)
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

        QDomElement nodeBandwidthList = analyse.firstChildElement("bandwidthList");
        if (!nodeBandwidthList.isNull())
        {
            int bandwidthCount = 0;
            QDomElement nodeBandwidth = indepVarList.firstChildElement("bandwidth");
            while (!nodeBandwidth.isNull())
            {
                if (nodeBandwidth.hasAttribute("bandwidth") && nodeBandwidth.hasAttribute("kernel")
                        && nodeBandwidth.hasAttribute("distanceType")
                        && nodeBandwidth.hasAttribute("initilizeType")
                        && nodeBandwidth.hasAttribute("selectionApproach")
                        && nodeBandwidth.hasAttribute("preditorCentered")
                        && nodeBandwidth.hasAttribute("selectThreshold"))
                {
                    bandwidthCount += 1;
                    double bandwidth = nodeBandwidth.attribute("bandwidth").toDouble();
                    bool adaptive = nodeBandwidth.attribute("adaptive").toInt();
                    GwmBandwidthWeight::KernelFunctionType kernel = GwmBandwidthWeight::KernelFunctionTypeNameMapper.value(nodeBandwidth.attribute("kernel"));
                    GwmBandwidthWeight weights(bandwidth, adaptive, kernel);
                    GwmDistance::DistanceType distanceType = GwmDistance::DistanceType(nodeBandwidth.attribute("distanceType").toUInt());
                    GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType initilizeType = GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType(nodeBandwidth.attribute("initilizeType").toUInt());
                    GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType selectionApproach = GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType(nodeBandwidth.attribute("selectionApproach").toUInt());
                    bool preditorCentered = nodeBandwidth.attribute("preditorCentered").toUInt();
                    double selectThreshold = nodeBandwidth.attribute("selectThreshold").toDouble();
                    mBandwidthWeights.append(weights);
                    mDistaneTypes.append(distanceType);
                    mBandwidthInitilize.append(initilizeType);
                    mBandwidthSelectionApproach.append(selectionApproach);
                    mPreditorCentered.append(preditorCentered);
                    mBandwidthSelectThreshold.append(selectThreshold);
                }
            }
            if (bandwidthCount != (mIndepVars.size() + 1))
            {
                return false;
            }
        }

        return true;
    }
    else return false;
}

bool GwmLayerMultiscaleGWRItem::writeXml(QDomNode &node, QDomDocument &doc)
{
    if (GwmLayerVectorItem::writeXml(node, doc))
    {
        QDomElement nodeAnalyse = node.toElement();
        nodeAnalyse.setAttribute("dataPointSize", mDataPointsSize);
        nodeAnalyse.setAttribute("criterionType", mCriterionType);

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

        QDomElement nodeBandwidthList = doc.createElement("bandwidthList");
        if ((mIndepVars.size() + 1) == mBandwidthWeights.size()
                == mDistaneTypes.size()
                == mBandwidthInitilize.size()
                == mBandwidthSelectionApproach.size()
                == mPreditorCentered.size()
                == mBandwidthSelectThreshold.size())
        {
            for (int i = 0; i < mBandwidthWeights.size(); i++)
            {
                QDomElement nodeBandwidth = doc.createElement("bandwidth");
                nodeBandwidth.setAttribute("kernel", GwmBandwidthWeight::KernelFunctionTypeNameMapper.name(mBandwidthWeights[i].kernel()));
                nodeBandwidth.setAttribute("bandwidth", mBandwidthWeights[i].bandwidth());
                nodeBandwidth.setAttribute("adaptive", mBandwidthWeights[i].adaptive());
                nodeBandwidth.setAttribute("distanceType", mDistaneTypes[i]);
                nodeBandwidth.setAttribute("initilizeType", mBandwidthInitilize[i]);
                nodeBandwidth.setAttribute("selectionApproach", mBandwidthSelectionApproach[i]);
                nodeBandwidth.setAttribute("preditorCentered", mPreditorCentered[i]);
                nodeBandwidth.setAttribute("selectThreshold", mBandwidthSelectThreshold[i]);
                nodeAnalyse.appendChild(nodeBandwidth);
            }
        }
        else return false;

        if (mHasHatmatrix)
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

        return  true;
    }
    else return false;
}

int GwmLayerMultiscaleGWRItem::dataPointsSize() const
{
    return mDataPointsSize;
}

QList<bool> GwmLayerMultiscaleGWRItem::preditorCentered() const
{
    return mPreditorCentered;
}

GwmDiagnostic GwmLayerMultiscaleGWRItem::diagnostic() const
{
    return mDiagnostic;
}

arma::mat GwmLayerMultiscaleGWRItem::betas() const
{
    return mBetas;
}

bool GwmLayerMultiscaleGWRItem::hasHatmatrix() const
{
    return mHasHatmatrix;
}

QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> GwmLayerMultiscaleGWRItem::bandwidthInitilize() const
{
    return mBandwidthInitilize;
}

QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> GwmLayerMultiscaleGWRItem::bandwidthSelectionApproach() const
{
    return mBandwidthSelectionApproach;
}

QList<double> GwmLayerMultiscaleGWRItem::bandwidthSelectThreshold() const
{
    return mBandwidthSelectThreshold;
}

GwmMultiscaleGWRAlgorithm::BackFittingCriterionType GwmLayerMultiscaleGWRItem::criterionType() const
{
    return mCriterionType;
}

QList<GwmBandwidthWeight> GwmLayerMultiscaleGWRItem::bandwidthWeights() const
{
    return mBandwidthWeights;
}

QList<GwmDistance::DistanceType> GwmLayerMultiscaleGWRItem::distaneTypes() const
{
    return mDistaneTypes;
}

GwmVariable GwmLayerMultiscaleGWRItem::depVar() const
{
    return mDepVar;
}

QList<GwmVariable> GwmLayerMultiscaleGWRItem::indepVars() const
{
    return mIndepVars;
}

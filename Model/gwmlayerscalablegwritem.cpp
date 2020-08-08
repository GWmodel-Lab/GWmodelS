#include "gwmlayerscalablegwritem.h"

GwmLayerScalableGWRItem::GwmLayerScalableGWRItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmScalableGWRAlgorithm* taskThread)
    : GwmLayerVectorItem(parent, vector)
{
    if (taskThread)
    {
        mDataPointsSize = taskThread->dataLayer()->featureCount();
        mDepVar = taskThread->dependentVariable();
        mIndepVars = taskThread->independentVariables();
        mWeight = GwmBandwidthWeight(*static_cast<GwmBandwidthWeight*>(taskThread->spatialWeight().weight()));
        mDistanceType = taskThread->spatialWeight().distance()->type();
        mDiagnostic = taskThread->diagnostic();
        mBetas = mat(taskThread->betas());
        mPolynomial = taskThread->polynomial();
        mCV = taskThread->cv();
        mScale = taskThread->scale();
        mPenalty = taskThread->penalty();
        mParameterOptimizeCriterionType = taskThread->parameterOptimizeCriterion();
    }
}

bool GwmLayerScalableGWRItem::readXml(QDomNode &node)
{
    if (GwmLayerVectorItem::readXml(node))
    {
        QDomElement analyse = node.toElement();
        mDataPointsSize = analyse.attribute("dataPointSize").toDouble();
        if (mDataPointsSize != mLayer->featureCount())
            return false;
        mPolynomial = analyse.attribute("polynomial").toDouble();
        mCV = analyse.attribute("cv").toDouble();
        mScale = analyse.attribute("scale").toDouble();
        mPenalty = analyse.attribute("penalty").toDouble();
        mParameterOptimizeCriterionType = GwmScalableGWRAlgorithm::ParameterOptimizeCriterionType(analyse.attribute("parameterOptimizeCriterion").toInt());

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

        return true;
    }
    else return false;
}

bool GwmLayerScalableGWRItem::writeXml(QDomNode &node, QDomDocument &doc)
{
    if (GwmLayerVectorItem::writeXml(node, doc))
    {
        QDomElement nodeAnalyse = node.toElement();
        nodeAnalyse.setAttribute("dataPointSize", mDataPointsSize);
        nodeAnalyse.setAttribute("polynomial", mPolynomial);
        nodeAnalyse.setAttribute("cv", mCV);
        nodeAnalyse.setAttribute("scale", mScale);
        nodeAnalyse.setAttribute("penalty", mPenalty);
        nodeAnalyse.setAttribute("parameterOptimizeCriterion", mParameterOptimizeCriterionType);

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

        QDomElement nodeDiagnostic = doc.createElement("diagnostic");
        nodeDiagnostic.setAttribute("RSS", mDiagnostic.RSS);
        nodeDiagnostic.setAttribute("AIC", mDiagnostic.AIC);
        nodeDiagnostic.setAttribute("AICc", mDiagnostic.AICc);
        nodeDiagnostic.setAttribute("ENP", mDiagnostic.ENP);
        nodeDiagnostic.setAttribute("EDF", mDiagnostic.EDF);
        nodeDiagnostic.setAttribute("RSquare", mDiagnostic.RSquare);
        nodeDiagnostic.setAttribute("RSquareAdjust", mDiagnostic.RSquareAdjust);
        nodeAnalyse.appendChild(nodeDiagnostic);

        return true;
    }
    else return false;
}

double GwmLayerScalableGWRItem::cv() const
{
    return mCV;
}

double GwmLayerScalableGWRItem::scale() const
{
    return mScale;
}

double GwmLayerScalableGWRItem::penalty() const
{
    return mPenalty;
}

int GwmLayerScalableGWRItem::polynomial() const
{
    return mPolynomial;
}

int GwmLayerScalableGWRItem::dataPointsSize() const
{
    return mDataPointsSize;
}

GwmVariable GwmLayerScalableGWRItem::depVar() const
{
    return mDepVar;
}

QList<GwmVariable> GwmLayerScalableGWRItem::indepVars() const
{
    return mIndepVars;
}

GwmBandwidthWeight GwmLayerScalableGWRItem::weight() const
{
    return mWeight;
}

GwmDiagnostic GwmLayerScalableGWRItem::diagnostic() const
{
    return mDiagnostic;
}

arma::mat GwmLayerScalableGWRItem::betas() const
{
    return mBetas;
}

GwmDistance::DistanceType GwmLayerScalableGWRItem::distanceType() const
{
    return mDistanceType;
}

GwmScalableGWRAlgorithm::ParameterOptimizeCriterionType GwmLayerScalableGWRItem::parameterOptimizeCriterionType() const
{
    return mParameterOptimizeCriterionType;
}

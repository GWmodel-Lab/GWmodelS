#include "gwmlayerggwritem.h"

GwmLayerGGWRItem::GwmLayerGGWRItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmGeneralizedGWRAlgorithm* taskThread)
    :GwmLayerBasicGWRItem(parent,vector)
{
    if (taskThread)
    {
        mDataPointsSize = taskThread->dataLayer()->featureCount();
        mDepVar = taskThread->dependentVariable();
        mIndepVars = taskThread->independentVariables();
        mWeight = GwmBandwidthWeight(*static_cast<GwmBandwidthWeight*>(taskThread->spatialWeight().weight()));
        mBetas = mat(taskThread->betas());
        isBandwidthOptimized = taskThread->autoselectBandwidth();
        mBandwidthSelScores = taskThread->bandwidthSelectorCriterions();
        hasHatmatrix = taskThread->hasHatMatrix();
        isRegressionPointGiven = !(taskThread->regressionLayer() == nullptr);
        mDiagnostic = taskThread->getDiagnostic();
        mGLMDiagnostic = taskThread->getGLMDiagnostic();
    }
}

bool GwmLayerGGWRItem::readXml(QDomNode &node)
{
    if (GwmLayerBasicGWRItem::readXml(node))
    {
        QDomElement analyse = node.toElement();
        if (!analyse.hasAttribute("family"))
            return false;
        mFamily = GwmGeneralizedGWRAlgorithm::FamilyValueNameMapper.value(analyse.attribute("family"));

        if (hasHatmatrix)
        {
            QDomElement diagnosticNode = analyse.firstChildElement("diagnosticGGWR");
            if (!diagnosticNode.isNull())
            {
                mDiagnostic.RSS = diagnosticNode.attribute("RSS").toDouble();
                mDiagnostic.AIC = diagnosticNode.attribute("AIC").toDouble();
                mDiagnostic.AICc = diagnosticNode.attribute("AICc").toDouble();
                mDiagnostic.RSquare = diagnosticNode.attribute("RSquare").toDouble();
            }
            else
            {
                hasHatmatrix = false;
            }
        }

        if (hasHatmatrix)
        {
            QDomElement nodeDiagnosticGLM = analyse.firstChildElement("diagnosticGLM");
            if (!nodeDiagnosticGLM.isNull())
            {
                mGLMDiagnostic.NullDev = nodeDiagnosticGLM.attribute("NullDev").toDouble();
                mGLMDiagnostic.Dev = nodeDiagnosticGLM.attribute("Dev").toDouble();
                mGLMDiagnostic.AIC = nodeDiagnosticGLM.attribute("AIC").toDouble();
                mGLMDiagnostic.AICc = nodeDiagnosticGLM.attribute("AICc").toDouble();
                mGLMDiagnostic.RSquare = nodeDiagnosticGLM.attribute("RSquare").toDouble();
            }
            else
            {
                hasHatmatrix = false;
            }
        }

        return true;
    }
    else return false;
}

bool GwmLayerGGWRItem::writeXml(QDomNode &node, QDomDocument &doc)
{
    if (GwmLayerBasicGWRItem::writeXml(node, doc))
    {
        QDomElement nodeAnalyse = node.toElement();
        nodeAnalyse.setAttribute("family", GwmGeneralizedGWRAlgorithm::FamilyValueNameMapper.name(mFamily));

        if (hasHatmatrix)
        {
            QDomElement nodeDiagnosticGGWR = doc.createElement("diagnosticGGWR");
            nodeDiagnosticGGWR.setAttribute("RSS", mDiagnostic.RSS);
            nodeDiagnosticGGWR.setAttribute("AIC", mDiagnostic.AIC);
            nodeDiagnosticGGWR.setAttribute("AICc", mDiagnostic.AICc);
            nodeDiagnosticGGWR.setAttribute("RSquare", mDiagnostic.RSquare);
            nodeAnalyse.appendChild(nodeDiagnosticGGWR);

            QDomElement nodeDiagnosticGLM = doc.createElement("diagnosticGLM");
            nodeDiagnosticGLM.setAttribute("NullDev", mGLMDiagnostic.NullDev);
            nodeDiagnosticGLM.setAttribute("Dev", mGLMDiagnostic.Dev);
            nodeDiagnosticGLM.setAttribute("AIC", mGLMDiagnostic.AIC);
            nodeDiagnosticGLM.setAttribute("AICc", mGLMDiagnostic.AICc);
            nodeDiagnosticGLM.setAttribute("RSquare", mGLMDiagnostic.RSquare);
            nodeAnalyse.appendChild(nodeDiagnosticGLM);
        }

        return true;
    }
    else return false;
}

GwmGGWRDiagnostic GwmLayerGGWRItem::diagnostic() const
{
    return mDiagnostic;
}

GwmGLMDiagnostic GwmLayerGGWRItem::GLMdiagnostic() const
{
    return mGLMDiagnostic;
}

GwmGeneralizedGWRAlgorithm::Family GwmLayerGGWRItem::family() const
{
    return mFamily;
}

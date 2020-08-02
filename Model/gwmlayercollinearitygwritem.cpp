#include "gwmlayercollinearitygwritem.h"
#include "gwmlayergroupitem.h"

GwmLayerCollinearityGWRItem::GwmLayerCollinearityGWRItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmLocalCollinearityGWRAlgorithm* taskThread)
    : GwmLayerVectorItem(parent, vector)
{
    if (taskThread)
    {
        mDataPointsSize = taskThread->dataLayer()->featureCount();
        mDepVar = taskThread->dependentVariable();
        mIndepVars = taskThread->independentVariables();
        mWeight = GwmBandwidthWeight(*static_cast<GwmBandwidthWeight*>(taskThread->spatialWeight().weight()));
        mDiagnostic = taskThread->dialnostic();
        mBetas = mat(taskThread->betas());
        //mModelSelModels = taskThread->indepVarSelectorCriterions();
        isBandwidthOptimized = taskThread->isAutoselectBandwidth();
        //isModelOptimized = taskThread->autoselectIndepVars();
        mBandwidthSelScores = taskThread->bandwidthSelectorCriterions();
        //mFTestResults = taskThread->fTestResult();
        //hasHatmatrix = taskThread->getHasHatmatrix();
        //hasFTest = taskThread->hasFTest();
        //isRegressionPointGiven = !(taskThread->regressionLayer() == nullptr);
        mLambda = taskThread->lambda();
        mcnThresh = taskThread->cnThresh();
        hasHatmatrix = taskThread->hasHatmatix();
    }
}

int GwmLayerCollinearityGWRItem::childNumber()
{
    if (mParentItem)
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this) + 1;
    return 0;
}

bool GwmLayerCollinearityGWRItem::readXml(QDomNode &node)
{
    if (GwmLayerVectorItem::readXml(node))
    {
        mDataPointsSize = mLayer->featureCount();

        QDomElement analyse = node.toElement();
        hasHatmatrix = analyse.attribute("hasHatmatrix").toInt();
        isRegressionPointGiven = analyse.attribute("isRegressionPointGiven").toInt();
        isBandwidthOptimized = analyse.attribute("isBandwidthOptimized").toInt();
        mLambda = analyse.attribute("lambda").toInt();
        mcnThresh = analyse.attribute("cnThresh").toInt();

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
        else return false;

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
            else
            {
                hasHatmatrix = false;
            }
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

bool GwmLayerCollinearityGWRItem::writeXml(QDomNode &node, QDomDocument &doc)
{
    if (GwmLayerVectorItem::writeXml(node, doc))
    {
        QDomElement nodeAnalyse = node.toElement();
        nodeAnalyse.setAttribute("dataPointSize", mDataPointsSize);
        nodeAnalyse.setAttribute("isBandwidthOptimized", isBandwidthOptimized);
        nodeAnalyse.setAttribute("hasHatmatrix", hasHatmatrix);
        nodeAnalyse.setAttribute("isRegressionPointGiven", isRegressionPointGiven);
        nodeAnalyse.setAttribute("lambda", mLambda);
        nodeAnalyse.setAttribute("cnThresh", mcnThresh);

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

int GwmLayerCollinearityGWRItem::dataPointsSize() const
{
    return mDataPointsSize;
}

GwmVariable GwmLayerCollinearityGWRItem::depVar() const
{
    return mDepVar;
}

QList<GwmVariable> GwmLayerCollinearityGWRItem::indepVars() const
{
    return mIndepVars;
}

GwmBandwidthWeight GwmLayerCollinearityGWRItem::weight() const
{
    return mWeight;
}

GwmDiagnostic GwmLayerCollinearityGWRItem::diagnostic() const
{
    return mDiagnostic;
}

arma::mat GwmLayerCollinearityGWRItem::betas() const
{
    return mBetas;
}

QList<QPair<double, double> > GwmLayerCollinearityGWRItem::bandwidthSelScores() const
{
    return mBandwidthSelScores;
}

bool GwmLayerCollinearityGWRItem::getIsRegressionPointGiven() const
{
    return isRegressionPointGiven;
}

bool GwmLayerCollinearityGWRItem::getIsBandwidthOptimized() const
{
    return isBandwidthOptimized;
}

bool GwmLayerCollinearityGWRItem::getHasHatmatrix() const
{
    return hasHatmatrix;
}

double GwmLayerCollinearityGWRItem::getLambda() const
{
    return mLambda;
}

double GwmLayerCollinearityGWRItem::getMcnThresh() const
{
    return mcnThresh;
}

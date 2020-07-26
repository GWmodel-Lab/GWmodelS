#include "gwmlayerbasicgwritem.h"
#include "gwmlayergroupitem.h"

GwmLayerBasicGWRItem::GwmLayerBasicGWRItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmBasicGWRAlgorithm *taskThread)
    : GwmLayerVectorItem(parent, vector)
{
    if (taskThread)
    {
        mDataPointsSize = taskThread->dataLayer()->featureCount();
        mDepVar = taskThread->dependentVariable();
        mIndepVars = taskThread->independentVariables();
        mWeight = GwmBandwidthWeight(*static_cast<GwmBandwidthWeight*>(taskThread->spatialWeight().weight()));
        mDiagnostic = taskThread->diagnostic();
        mBetas = mat(taskThread->betas());
        mModelSelModels = taskThread->indepVarSelectorCriterions();
        isBandwidthOptimized = taskThread->autoselectBandwidth();
        isModelOptimized = taskThread->autoselectIndepVars();
        mBandwidthSelScores = taskThread->bandwidthSelectorCriterions();
        mFTestResults = taskThread->fTestResult();
        hasHatmatrix = taskThread->hasHatMatrix();
        hasFTest = taskThread->hasFTest();
        isRegressionPointGiven = !(taskThread->regressionLayer() == nullptr);
    }
}

int GwmLayerBasicGWRItem::childNumber()
{
    if (mParentItem)
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this) + 1;
    return 0;
}

bool GwmLayerBasicGWRItem::writeXml(QDomNode &node, QDomDocument &doc)
{
    if (GwmLayerVectorItem::writeXml(node, doc))
    {
        QDomElement nodeAnalyse = node.toElement();
        nodeAnalyse.setAttribute("dataPointSize", mDataPointsSize);
        nodeAnalyse.setAttribute("isBandwidthOptimized", isBandwidthOptimized);
        nodeAnalyse.setAttribute("isModelOptimized", isModelOptimized);
        nodeAnalyse.setAttribute("hasHatmatrix", hasHatmatrix);
        nodeAnalyse.setAttribute("hasFTest", hasFTest);
        nodeAnalyse.setAttribute("isRegressionPointGiven", isRegressionPointGiven);

        QDomElement nodeDepVar = doc.createElement("depVar");
        nodeDepVar.setAttribute("index", mDepVar.index);
        nodeDepVar.setAttribute("isNumeric", mDepVar.isNumeric);
        nodeDepVar.setAttribute("name", mDepVar.name);
        nodeAnalyse.appendChild(nodeDepVar);

        QDomElement nodeIndepVarList = doc.createElement("indepVarList");
        for (auto indepVar : mIndepVars)
        {
            QDomElement nodeIndep = doc.createElement("indepVar");
            nodeIndep.setAttribute("index", indepVar.index);
            nodeIndep.setAttribute("isNumeric", indepVar.isNumeric);
            nodeIndep.setAttribute("name", indepVar.name);
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

        if (isModelOptimized)
        {
            QDomElement nodeSelModels = doc.createElement("modelCriterions");
            for (auto model : mModelSelModels)
            {
                QDomElement nodeModel = doc.createElement("model");
                nodeModel.setAttribute("criterion", model.second);

                QDomElement nodeVarList = doc.createElement("variableList");
                for (auto variable : model.first)
                {
                    QDomElement nodeVar = doc.createElement("variable");
                    nodeVar.setAttribute("index", variable.index);
                    nodeVar.setAttribute("isNumeric", variable.isNumeric);
                    nodeVar.setAttribute("name", variable.name);
                    nodeVarList.appendChild(nodeVar);
                }
                nodeModel.appendChild(nodeVarList);

                nodeSelModels.appendChild(nodeModel);
            }
            nodeAnalyse.appendChild(nodeSelModels);
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

        if (hasFTest)
        {
            QDomElement nodeFTestResults = doc.createElement("FTestResults");

            QDomElement nodeF1Test = doc.createElement("F1Test");
            nodeF1Test.setAttribute("statistic", mFTestResults.f1.s);
            nodeF1Test.setAttribute("df1", mFTestResults.f1.df1);
            nodeF1Test.setAttribute("df2", mFTestResults.f1.df2);
            nodeF1Test.setAttribute("pValue", mFTestResults.f1.p);
            nodeFTestResults.appendChild(nodeF1Test);

            QDomElement nodeF2Test = doc.createElement("F2Test");
            nodeF2Test.setAttribute("statistic", mFTestResults.f2.s);
            nodeF2Test.setAttribute("df1", mFTestResults.f2.df1);
            nodeF2Test.setAttribute("df2", mFTestResults.f2.df2);
            nodeF2Test.setAttribute("pValue", mFTestResults.f2.p);
            nodeFTestResults.appendChild(nodeF2Test);

            QDomElement nodeF3TestList = doc.createElement("F3TestList");
            for (auto f3 : mFTestResults.f3)
            {
                QDomElement nodeF3Test = doc.createElement("F3Test");
                nodeF3Test.setAttribute("statistic", f3.s);
                nodeF3Test.setAttribute("df1", f3.df1);
                nodeF3Test.setAttribute("df2", f3.df2);
                nodeF3Test.setAttribute("pValue", f3.p);
                nodeF3TestList.appendChild(nodeF3Test);
            }
            nodeFTestResults.appendChild(nodeF3TestList);

            QDomElement nodeF4Test = doc.createElement("F4Test");
            nodeF4Test.setAttribute("statistic", mFTestResults.f4.s);
            nodeF4Test.setAttribute("df1", mFTestResults.f4.df1);
            nodeF4Test.setAttribute("df2", mFTestResults.f4.df2);
            nodeF4Test.setAttribute("pValue", mFTestResults.f4.p);
            nodeFTestResults.appendChild(nodeF4Test);

            nodeAnalyse.appendChild(nodeFTestResults);
        }

        return true;
    }
    return false;
}

int GwmLayerBasicGWRItem::dataPointsSize() const
{
    return mDataPointsSize;
}

GwmDiagnostic GwmLayerBasicGWRItem::diagnostic() const
{
    return mDiagnostic;
}

arma::mat GwmLayerBasicGWRItem::betas() const
{
    return mBetas;
}

bool GwmLayerBasicGWRItem::modelOptimized() const
{
    return isModelOptimized;
}

bool GwmLayerBasicGWRItem::bandwidthOptimized() const
{
    return isBandwidthOptimized;
}

bool GwmLayerBasicGWRItem::hatmatrix() const
{
    return hasHatmatrix;
}

bool GwmLayerBasicGWRItem::fTest() const
{
    return hasFTest;
}

bool GwmLayerBasicGWRItem::regressionPointGiven() const
{
    return isRegressionPointGiven;
}

GwmVariable GwmLayerBasicGWRItem::depVar() const
{
    return mDepVar;
}

QList<GwmVariable> GwmLayerBasicGWRItem::indepVars() const
{
    return mIndepVars;
}

GwmBasicGWRAlgorithm::FTestResultPack GwmLayerBasicGWRItem::fTestResults() const
{
    return mFTestResults;
}

QList<QPair<QList<GwmVariable>, double> > GwmLayerBasicGWRItem::modelSelModels() const
{
    return mModelSelModels;
}

QList<QPair<double, double> > GwmLayerBasicGWRItem::bandwidthSelScores() const
{
    return mBandwidthSelScores;
}

GwmBandwidthWeight GwmLayerBasicGWRItem::weight() const
{
    return mWeight;
}

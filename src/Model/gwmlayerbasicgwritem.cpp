#include "gwmlayerbasicgwritem.h"
#include "gwmlayergroupitem.h"
#include "TaskThread/gwmbasicgwralgorithm.h"

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
        hasols = taskThread->OLS();
        mOLSVar = taskThread->getOLSVar();
        isRegressionPointGiven = !(taskThread->regressionLayer() == nullptr);
    }
}

int GwmLayerBasicGWRItem::childNumber()
{
    if (mParentItem)
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this) + 1;
    return 0;
}

bool GwmLayerBasicGWRItem::readXml(QDomNode &node)
{
    if (GwmLayerVectorItem::readXml(node))
    {
        mDataPointsSize = mLayer->featureCount();

        QDomElement analyse = node.toElement();
        hasHatmatrix = analyse.attribute("hasHatmatrix").toInt();
        hasFTest = analyse.attribute("hasFTest").toInt();
        isRegressionPointGiven = analyse.attribute("isRegressionPointGiven").toInt();
        isBandwidthOptimized = analyse.attribute("isBandwidthOptimized").toInt();
        isModelOptimized = analyse.attribute("isModelOptimized").toInt();
        hasols = analyse.attribute("hasols").toInt();
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

        if(hasols)
        {
            QDomElement nodeOLSResult = analyse.firstChildElement("OLSResult");
            if(!nodeOLSResult.isNull())
            {
                mOLSVar.RSD = nodeOLSResult.attribute("rsd").toDouble();
                mOLSVar.R2 = nodeOLSResult.attribute("R2").toDouble();
                mOLSVar.adjR2 = nodeOLSResult.attribute("adjR2").toDouble();
                mOLSVar.AIC = nodeOLSResult.attribute("AIC").toDouble();
                mOLSVar.AICC = nodeOLSResult.attribute("AICC").toDouble();
                QMap<QString,QList<double> > coeffcients;
                QDomElement coeff = nodeOLSResult.firstChildElement("coeff");
                while (!coeff.isNull())
                {
                    if (coeff.hasAttribute("name") && coeff.hasAttribute("coeff") && coeff.hasAttribute("errorStd") )
                    {
                        QString cname = coeff.attribute("name");
                        double ncoeff = coeff.attribute("coeff").toDouble();
                        double error = coeff.attribute("errorStd").toDouble();
                        QList<double> varcoeff;
                        varcoeff.append(ncoeff);
                        varcoeff.append(error);
                        coeffcients[cname] = varcoeff;
                    }
                    coeff = coeff.nextSiblingElement("coeff");
                }
                mOLSVar.Coefficients = coeffcients;
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

        if (isModelOptimized)
        {
            QDomElement modelCriterionsNode = analyse.firstChildElement("modelCriterions");
            if (!modelCriterionsNode.isNull())
            {
                QDomElement modelNode = modelCriterionsNode.firstChildElement("model");
                while (!modelNode.isNull() && modelNode.hasAttribute("criterion"))
                {
                    double aic = modelNode.attribute("criterion").toDouble();

                    bool allVariableCorrect = true;
                    QList<GwmVariable> modelVariables;
                    QDomElement variableNode = modelNode.firstChildElement("variable");
                    while (!variableNode.isNull())
                    {
                        if (variableNode.hasAttribute("name") && variableNode.hasAttribute("index")
                                && variableNode.hasAttribute("isNumeric")
                                && variableNode.hasAttribute("type"))
                        {
                            GwmVariable variable;
                            variable.name = variableNode.attribute("name");
                            variable.index = variableNode.attribute("index").toInt();
                            variable.isNumeric = variableNode.attribute("isNumeric").toInt();
                            variable.type = QVariant::Type(variableNode.attribute("type").toInt());
                            modelVariables.append(variable);
                        }
                        else allVariableCorrect = false;
                        variableNode = variableNode.nextSiblingElement("variable");
                    }
                    if (allVariableCorrect)
                        mModelSelModels.append(qMakePair(modelVariables, aic));

                    modelNode = modelNode.nextSiblingElement("model");
                }
            }
            else
            {
                isModelOptimized = false;
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
                    bandwidthNode = bandwidthNode.nextSiblingElement("bandwidth");
                }
            }
            else
            {
                isBandwidthOptimized = false;
            }
        }

        if (hasFTest)
        {
            QDomElement nodeFTestResults = node.firstChildElement("FTestResults");
            if (!nodeFTestResults.isNull())
            {
                QDomElement nodeF1Test = nodeFTestResults.firstChildElement("F1Test");
                if (!nodeF1Test.isNull()
                        && nodeF1Test.hasAttribute("statistic") && nodeF1Test.hasAttribute("pValue")
                        && nodeF1Test.hasAttribute("df1") && nodeF1Test.hasAttribute("df2"))
                {
                    double statistic = nodeF1Test.attribute("statistic").toDouble();
                    double pValue = nodeF1Test.attribute("pValue").toDouble();
                    double df1 = nodeF1Test.attribute("df1").toDouble();
                    double df2 = nodeF1Test.attribute("df2").toDouble();
                    mFTestResults.f1 = GwmFTestResult(statistic, df1, df2, pValue);
                }
                else hasFTest = false;

                QDomElement nodeF2Test = nodeFTestResults.firstChildElement("F2Test");
                if (!nodeF2Test.isNull()
                        && nodeF2Test.hasAttribute("statistic") && nodeF2Test.hasAttribute("pValue")
                        && nodeF2Test.hasAttribute("df1") && nodeF2Test.hasAttribute("df2"))
                {
                    double statistic = nodeF2Test.attribute("statistic").toDouble();
                    double pValue = nodeF2Test.attribute("pValue").toDouble();
                    double df1 = nodeF2Test.attribute("df1").toDouble();
                    double df2 = nodeF2Test.attribute("df2").toDouble();
                    mFTestResults.f2 = GwmFTestResult(statistic, df1, df2, pValue);
                }
                else hasFTest = false;

                QDomElement nodeF3TestList = nodeFTestResults.firstChildElement("F3TestList");
                if (!nodeF3TestList.isNull())
                {
                    QDomElement nodeF3Test = nodeF3TestList.firstChildElement("F3Test");
                    while (!nodeF3Test.isNull())
                    {
                        if (nodeF3Test.hasAttribute("statistic") && nodeF3Test.hasAttribute("pValue")
                                && nodeF3Test.hasAttribute("df1") && nodeF3Test.hasAttribute("df2"))
                        {
                            double statistic = nodeF3Test.attribute("statistic").toDouble();
                            double pValue = nodeF3Test.attribute("pValue").toDouble();
                            double df1 = nodeF3Test.attribute("df1").toDouble();
                            double df2 = nodeF3Test.attribute("df2").toDouble();
                            mFTestResults.f3.append(GwmFTestResult(statistic, df1, df2, pValue));
                        }
                        nodeF3Test = nodeF3Test.nextSiblingElement("F3Test");
                    }
                }


                QDomElement nodeF4Test = nodeFTestResults.firstChildElement("F4Test");
                if (!nodeF2Test.isNull()
                        && nodeF4Test.hasAttribute("statistic") && nodeF4Test.hasAttribute("pValue")
                        && nodeF4Test.hasAttribute("df1") && nodeF4Test.hasAttribute("df2"))
                {
                    double statistic = nodeF4Test.attribute("statistic").toDouble();
                    double pValue = nodeF4Test.attribute("pValue").toDouble();
                    double df1 = nodeF4Test.attribute("df1").toDouble();
                    double df2 = nodeF4Test.attribute("df2").toDouble();
                    mFTestResults.f4 = GwmFTestResult(statistic, df1, df2, pValue);
                }
            }
            else
            {
                hasFTest = false;
            }
        }

        return true;
    }
    else return false;
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
        nodeAnalyse.setAttribute("hasols",hasols);
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

        if (hasols)
        {
            QDomElement nodeOLSResult = doc.createElement("OLSResult");
            nodeOLSResult.setAttribute("rsd",mOLSVar.RSD);
            nodeOLSResult.setAttribute("R2",mOLSVar.R2);
            nodeOLSResult.setAttribute("adjR2",mOLSVar.adjR2);
            nodeOLSResult.setAttribute("AIC",mOLSVar.AIC);
            nodeOLSResult.setAttribute("AICC",mOLSVar.AICC);
            QMap<QString,QList<double> >::iterator iter = mOLSVar.Coefficients.begin();
            while(iter!=mOLSVar.Coefficients.end())
            {
                QDomElement nodeOLScoef = doc.createElement("coeff");
                nodeOLScoef.setAttribute("name",iter.key());
                nodeOLScoef.setAttribute("coeff",iter.value()[0]);
                nodeOLScoef.setAttribute("errorStd",iter.value()[1]);
                iter++;
                nodeOLSResult.appendChild(nodeOLScoef);
            }
           nodeAnalyse.appendChild(nodeOLSResult);
        }

        if (isModelOptimized)
        {
            QDomElement nodeSelModels = doc.createElement("modelCriterions");
            for (auto model : mModelSelModels)
            {
                QDomElement nodeModel = doc.createElement("model");
                nodeModel.setAttribute("criterion", model.second);

//                QDomElement nodeVarList = doc.createElement("variableList");
                for (auto variable : model.first)
                {
                    QDomElement nodeVar = doc.createElement("variable");
                    nodeVar.setAttribute("index", variable.index);
                    nodeVar.setAttribute("isNumeric", variable.isNumeric);
                    nodeVar.setAttribute("name", variable.name);
                    nodeVar.setAttribute("type", int(variable.type));
                    nodeModel.appendChild(nodeVar);
                }
//                nodeModel.appendChild(nodeVarList);

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

bool GwmLayerBasicGWRItem::ols() const
{
    return hasols;
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

GwmBasicGWRAlgorithm::OLSVar GwmLayerBasicGWRItem::OLSResults() const
{
    return mOLSVar;
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

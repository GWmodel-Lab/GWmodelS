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
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this);
    return 0;
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

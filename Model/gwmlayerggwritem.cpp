#include "gwmlayerggwritem.h"

GwmLayerGGWRItem::GwmLayerGGWRItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmGGWRAlgorithm* taskThread)
    :GwmLayerBasicGWRItem(parent,vector)
{
    if (taskThread)
    {
        mDataPointsSize = taskThread->dataLayer()->featureCount();
        mDepVar = taskThread->dependentVariable();
        mIndepVars = taskThread->independentVariables();
        mWeight = GwmBandwidthWeight(*static_cast<GwmBandwidthWeight*>(taskThread->spatialWeight().weight()));
        mBetas = mat(taskThread->betas());
        mModelSelModels = taskThread->indepVarSelectorCriterions();
        isBandwidthOptimized = taskThread->autoselectBandwidth();
        isModelOptimized = taskThread->autoselectIndepVars();
        mBandwidthSelScores = taskThread->bandwidthSelectorCriterions();
        mFTestResults = taskThread->fTestResult();
        hasHatmatrix = taskThread->hasHatMatrix();
        hasFTest = taskThread->hasFTest();
        isRegressionPointGiven = !(taskThread->regressionLayer() == nullptr);
        mDiagnostic = taskThread->getDiagnostic();
        mGLMDiagnostic = taskThread->getGLMDiagnostic();
    }
}

GwmGGWRDiagnostic GwmLayerGGWRItem::diagnostic() const
{
    return mDiagnostic;
}

GwmGLMDiagnostic GwmLayerGGWRItem::GLMdiagnostic() const
{
    return mGLMDiagnostic;
}

GwmGGWRAlgorithm::Family GwmLayerGGWRItem::family() const
{
    return mFamily;
}

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

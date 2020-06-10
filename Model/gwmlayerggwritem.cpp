#include "gwmlayerggwritem.h"

GwmLayerGGWRItem::GwmLayerGGWRItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmGGWRTaskThread* taskThread)
    :GwmLayerGWRItem(parent,vector)
{
    if (taskThread)
    {
        mDataPointsSize = taskThread->getFeatureList().size();
        mDepVarIndex = taskThread->getDepVarIndex();
        mIndepVarsIndex = taskThread->getIndepVarsIndex();
        mIndepVarsOrigin = taskThread->indepVars();
        mBandwidthType = taskThread->bandwidthType();
        mBandwidthSize = (mBandwidthType == GwmGWRTaskThread::BandwidthType::Fixed ?
                              taskThread->getBandwidthSizeOrigin() :
                              taskThread->getBandwidthSize());
        mBandwidthUnit = taskThread->getBandwidthUnit();
        mBandwidthKernelFunction = taskThread->getBandwidthKernelFunction();
        mDiagnostic = taskThread->getDiagnostic();
        mGLMDiagnostic = taskThread->getGLMDiagnostic();
        mBetas = mat(taskThread->getBetas());
        mModelSelModels = taskThread->getModelSelModels();
        mModelSelAICcs = taskThread->getModelSelAICcs();
        isBandwidthOptimized = taskThread->getIsBandwidthSizeAutoSel();
        isModelOptimized = taskThread->enableIndepVarAutosel();
        mBandwidthSelScores = taskThread->getBwScore();
        mFTestResults = taskThread->fTestResults();
        hasHatmatrix = taskThread->getHasHatMatrix();
        hasFTest = taskThread->getHasFTest();
        mFamily = taskThread->getFamily();
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

GwmGGWRTaskThread::Family GwmLayerGGWRItem::family() const
{
    return mFamily;
}

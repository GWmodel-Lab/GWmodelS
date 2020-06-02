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
        mBetas = mat(taskThread->getBetas());
        mModelSelModels = taskThread->getModelSelModels();
        mModelSelAICcs = taskThread->getModelSelAICcs();
        isBandwidthOptimized = taskThread->getIsBandwidthSizeAutoSel();
        isModelOptimized = taskThread->enableIndepVarAutosel();
        mBandwidthSelScores = taskThread->getBwScore();
        mFTestResults = taskThread->fTestResults();
        hasHatmatrix = taskThread->getHasHatMatrix();
        hasFTest = taskThread->getHasFTest();
    }
}

GwmGGWRDiagnostic GwmLayerGGWRItem::diagnostic() const
{
    return mDiagnostic;
}

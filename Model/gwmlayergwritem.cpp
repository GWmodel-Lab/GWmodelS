#include "gwmlayergwritem.h"
#include "gwmlayergroupitem.h"

GwmLayerGWRItem::GwmLayerGWRItem(GwmLayerItem* parent, QgsVectorLayer* vector, const GwmGWRTaskThread* taskThread)
    : GwmLayerVectorItem(parent, vector)
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
        isRegressionPointGiven = !(taskThread->getRegressionLayer() == nullptr);
    }
}

int GwmLayerGWRItem::childNumber()
{
    if (mParentItem)
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this);
    return 0;
}

int GwmLayerGWRItem::dataPointsSize() const
{
    return mDataPointsSize;
}

double GwmLayerGWRItem::bandwidthSize() const
{
    return mBandwidthSize;
}

QString GwmLayerGWRItem::bandwidthUnit() const
{
    return mBandwidthUnit;
}

GwmGWRTaskThread::BandwidthType GwmLayerGWRItem::bandwidthType() const
{
    return mBandwidthType;
}

GwmGWRDiagnostic GwmLayerGWRItem::diagnostic() const
{
    return mDiagnostic;
}

arma::mat GwmLayerGWRItem::betas() const
{
    return mBetas;
}

QList<QStringList> GwmLayerGWRItem::modelSelModels() const
{
    return mModelSelModels;
}

QList<double> GwmLayerGWRItem::modelSelAICcs() const
{
    return mModelSelAICcs;
}

bool GwmLayerGWRItem::getIsModelOptimized() const
{
    return isModelOptimized;
}

bool GwmLayerGWRItem::getIsBandwidthOptimized() const
{
    return isBandwidthOptimized;
}

int GwmLayerGWRItem::getDepVarIndex() const
{
    return mDepVarIndex;
}

QList<int> GwmLayerGWRItem::getIndepVarIndex() const
{
    return mIndepVarsIndex;
}

QList<GwmLayerAttributeItem *> GwmLayerGWRItem::getIndepVarsOrigin() const
{
    return mIndepVarsOrigin;
}

QMap<double, double> GwmLayerGWRItem::getBandwidthSelScores() const
{
    return mBandwidthSelScores;
}

QList<GwmFTestResult> GwmLayerGWRItem::getFTestResults() const
{
    return mFTestResults;
}

bool GwmLayerGWRItem::getHasHatmatrix() const
{
    return hasHatmatrix;
}

bool GwmLayerGWRItem::getHasFTest() const
{
    return hasFTest;
}

bool GwmLayerGWRItem::getIsRegressionPointGiven() const
{
    return isRegressionPointGiven;
}

GwmGWRTaskThread::KernelFunction GwmLayerGWRItem::bandwidthKernelFunction() const
{
    return mBandwidthKernelFunction;
}

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
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this);
    return 0;
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

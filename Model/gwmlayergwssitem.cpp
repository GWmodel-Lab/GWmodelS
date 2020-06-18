#include "gwmlayergwssitem.h"
#include "gwmlayergroupitem.h"


GwmLayerGWSSItem::GwmLayerGWSSItem(GwmLayerItem* parentItem, QgsVectorLayer* vector, const GwmGWSSTaskThread* taskThread)
    : GwmLayerVectorItem(parentItem, vector)
{
    mBWS = taskThread->getBWS();
    mVariables = taskThread->variables();
    mBandwidth = taskThread->bandwidth();
    mQuantile = taskThread->quantile();

    mLocalMean = taskThread->localmean();
    mStandardDev = taskThread->standarddev();
    mLocalSkewness = taskThread->localskewness();
    mLCV = taskThread->lcv();
    mLVar = taskThread->lvar();

    if(mQuantile){
        mLocalMedian = taskThread->localmedian();
        mIQR = taskThread->iqr();
        mQI = taskThread->qi();
    }

    if(mVariables.size() >= 2){
        mCovmat = taskThread->covmat();
        mCorrmat = taskThread->corrmat();
        mSCorrmat = taskThread->scorrmat();
    }

}

int GwmLayerGWSSItem::childNumber()
{
    if (mParentItem)
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this);
    return 0;
}


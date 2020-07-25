#include "gwmlayergwssitem.h"
#include "gwmlayergroupitem.h"


GwmLayerGWSSItem::GwmLayerGWSSItem(GwmLayerItem* parentItem, QgsVectorLayer* vector, const GwmGWSSTaskThread* taskThread)
    : GwmLayerVectorItem(parentItem, vector)
{
    mDataPointsSize = taskThread->dataPointsSize();
    mVariables = taskThread->variables();
    mBandwidth = taskThread->bandwidth();
    mQuantile = taskThread->quantile();
    mResultList = taskThread->resultlist();

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
        return ((GwmLayerGroupItem*)mParentItem)->analyseChildren().indexOf(this) + 1;
    return 0;
}

int GwmLayerGWSSItem::dataPointsSize() const
{
    return mDataPointsSize;
}

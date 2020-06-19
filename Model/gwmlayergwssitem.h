#ifndef GWMLAYERGWSSITEM_H
#define GWMLAYERGWSSITEM_H

#include "gwmlayervectoritem.h"
#include "TaskThread/gwmgwsstaskthread.h"

class GwmLayerGWSSItem : public GwmLayerVectorItem
{
public:
    GwmLayerGWSSItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmGWSSTaskThread* taskThread = nullptr);

    virtual int childNumber() override;

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::GWSS; }

    mat bws() const
    {
        return mBWS;
    }

    int dataPointsSize() const;

    mat localmean() const{return mLocalMean;}
    mat standarddev() const{return mStandardDev;}
    mat localskewness() const{return mLocalSkewness;}
    mat lcv() const{return mLCV;}
    mat lvar() const{return mLVar;}

    mat localmedian() const{return mLocalMedian;}
    mat iqr() const{return mIQR;}
    mat qi() const{return mQI;}

    mat covmat() const{return mCovmat;}
    mat corrmat() const{return mCorrmat;}
    mat scorrmat() const{return mSCorrmat;}

    bool quantile() const{return mQuantile;}

    GwmGWSSTaskThread::CreateResultLayerData resultlist() const{return mResultList;}

    QList<GwmVariable> variables() const
    {
        return mVariables;
    }

    GwmBandwidthWeight* bandwidth() const
    {
        return mBandwidth;
    }



protected:
    int mDataPointsSize;
    mat mBWS;
    QList<GwmVariable> mVariables;
    GwmBandwidthWeight* mBandwidth;
    bool mQuantile;

protected:
    mat mLocalMean;
    mat mStandardDev;
    mat mLocalSkewness;
    mat mLCV;
    mat mLVar;

    mat mLocalMedian;
    mat mIQR;
    mat mQI;

    mat mCovmat;
    mat mCorrmat;
    mat mSCorrmat;

    GwmGWSSTaskThread::CreateResultLayerData mResultList;
};

#endif // GWMLAYERGWSSITEM_H

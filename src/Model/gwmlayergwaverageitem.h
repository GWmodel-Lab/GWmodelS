#ifndef GWMLAYERGWAVERAGEITEM_H
#define GWMLAYERGWAVERAGEITEM_H

#include "gwmlayervectoritem.h"
#include "TaskThread/gwmgwaveragetaskthread.h"
#include "TaskThread/gwmmultiscalegwralgorithm.h"

class GwmLayerGWAverageItem : public GwmLayerVectorItem
{
public:
    //average构造函数
    GwmLayerGWAverageItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmGWAverageTaskThread* taskThread = nullptr);
    ~GwmLayerGWAverageItem();

    virtual int childNumber() override;

    inline virtual GwmLayerItemType itemType() override { return GwmLayerItemType::GWAverage; }

    virtual bool readXml(QDomNode &node) override;
    virtual bool writeXml(QDomNode &node, QDomDocument &doc) override;

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

    GwmGWAverageTaskThread::CreateResultLayerData resultlist() const{return mResultList;}


    QList<GwmVariable> variables() const
    {
        return mVariables;
    }

    QList<GwmVariable> variablesY() const
    {
        return mVariablesY;
    }

    GwmBandwidthWeight* bandwidth() const
    {
        return mBandwidth;
    }

    //coreelation带宽部分
    QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> bandwidthSelectionApproach() const{
        return mBandwidthSelectionApproach;
    };

    QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> bandwidthInitilize() const{
        return mBandwidthInitilize;
    };

    QList<GwmBandwidthWeight> bandwidthWeights() const{
        return mBandwidthWeights;
    };

    QList<GwmDistance::DistanceType> distaneTypes() const{
        return mDistaneTypes;
    };

    //设置类型
    void setType(int t){
        mType = t;
    }

    int getType(){
        return mType;
    }

protected:
    int mDataPointsSize;
    QList<GwmVariable> mVariables;
    QList<GwmVariable> mVariablesY;
    GwmBandwidthWeight* mBandwidth;
    bool mQuantile;
    // correlation带宽信息
    QList<GwmBandwidthWeight> mBandwidthWeights;
    QList<GwmDistance::DistanceType> mDistaneTypes;
    QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> mBandwidthSelectionApproach;
    QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> mBandwidthInitilize;
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

    GwmGWAverageTaskThread::CreateResultLayerData mResultList ;
    int mType = 0;
};

#endif // GWMLAYERGWSSITEM_H

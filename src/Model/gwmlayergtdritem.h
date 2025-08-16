#ifndef GWMLAYERGTDRITEM_H
#define GWMLAYERGTDRITEM_H

#include "gwmlayervectoritem.h"
#include "TaskThread/gwmgtdrtaskthread.h"
#include "TaskThread/gwmgwaveragetaskthread.h"
#include "TaskThread/gwmgwcorrelationtaskthread.h"
#include "TaskThread/gwmmultiscalegwralgorithm.h"

class GwmLayerGTDRItem : public GwmLayerVectorItem
{
public:

    explicit GwmLayerGTDRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmGTDRTaskThread* taskThread = nullptr);

    virtual int childNumber() override;

    inline virtual GwmLayerItemType itemType() override { return GwmLayerItemType::GTDR; }

    virtual bool readXml(QDomNode &node) override;
    virtual bool writeXml(QDomNode &node, QDomDocument &doc) override;

    int dataPointsSize() const;

    GwmDiagnostic diagnostic() const;

    arma::mat betas() const;

    bool modelOptimized() const;

    bool bandwidthOptimized() const;

    bool hatmatrix() const;

    // bool fTest() const;

    // bool regressionPointGiven() const;

    // bool ols() const;

    // GwmGTDRAlgorithm::FTestResultPack fTestResults() const;

    QList<QPair<QList<GwmVariable>, double> > modelSelModels() const;

    QList<QPair<double, double> > bandwidthSelScores() const;

    GwmBandwidthWeight weight() const;

    // GwmGTDRAlgorithm::OLSVar OLSResults() const;

    // GwmLayerGTDRItem(GwmLayerItem* parentItem, QgsVectorLayer* vector, const GwmGTDRTaskThread* taskThread);
    ~GwmLayerGTDRItem();

    // virtual int childNumber() override;

    // inline virtual GwmLayerItemType itemType() override { return GwmLayerItemType::GTDR; }

    // virtual bool readXml(QDomNode &node) override;
    // virtual bool writeXml(QDomNode &node, QDomDocument &doc) override;

    // int dataPointsSize() const;

    // mat localmean() const{return mLocalMean;}
    // mat standarddev() const{return mStandardDev;}
    // mat localskewness() const{return mLocalSkewness;}
    // mat lcv() const{return mLCV;}
    // mat lvar() const{return mLVar;}

    // mat localmedian() const{return mLocalMedian;}
    // mat iqr() const{return mIQR;}
    // mat qi() const{return mQI;}

    // mat covmat() const{return mCovmat;}
    // mat corrmat() const{return mCorrmat;}
    // mat scorrmat() const{return mSCorrmat;}

    // bool quantile() const{return mQuantile;}

    GwmGTDRTaskThread::CreateResultLayerData resultlist() const{return mResultList;}


    GwmVariable depVar() const
    {
        return mDepVar;
    }

    QList<GwmVariable> indepVar() const
    {
        return mIndepVars;
    }

    GwmBandwidthWeight* bandwidth() const
    {
        return mBandwidth;
    }

    // //coreelationdai带宽部分
    // QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> bandwidthSelectionApproach() const{
    //     return mBandwidthSelectionApproach;
    // };

    // QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> bandwidthInitilize() const{
    //     return mBandwidthInitilize;
    // };

    // QList<GwmBandwidthWeight> bandwidthWeights() const{
    //     return mBandwidthWeights;
    // };

    // QList<GwmDistance::DistanceType> distaneTypes() const{
    //     return mDistaneTypes;
    // };

    // //设置类型
    // void setType(int t){
    //     mType = t;
    // }

    // int getType(){
    //     return mType;
    // }

protected:

    int mDataPointsSize;
    GwmVariable mDepVar;
    QList<GwmVariable> mIndepVars;
    GwmBandwidthWeight mWeight;
    GwmDiagnostic mDiagnostic;
    arma::mat mBetas;

    GwmBandwidthWeight* mBandwidth;

    QList<QPair<QList<GwmVariable>, double> > mModelSelModels;
    QList<QPair<double, double> > mBandwidthSelScores;
    // GwmGTDRAlgorithm::FTestResultPack mFTestResults;
    // GwmGTDRAlgorithm::OLSVar mOLSVar;
    bool isRegressionPointGiven;
    bool isModelOptimized;
    bool isBandwidthOptimized;
    bool hasHatmatrix;
    bool hasFTest;
    bool hasols;

    // int mDataPointsSize;
    // QList<GwmVariable> mVariables;
    // QList<GwmVariable> mVariablesY;
    // GwmBandwidthWeight* mBandwidth;
    // bool mQuantile;
    // // correlation带宽信息
    // QList<GwmBandwidthWeight> mBandwidthWeights;
    // QList<GwmDistance::DistanceType> mDistaneTypes;
    // QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> mBandwidthSelectionApproach;
    // QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> mBandwidthInitilize;

    // mat mLocalMean;
    // mat mStandardDev;
    // mat mLocalSkewness;
    // mat mLCV;
    // mat mLVar;

    // mat mLocalMedian;
    // mat mIQR;
    // mat mQI;

    // mat mCovmat;
    // mat mCorrmat;
    // mat mSCorrmat;

    GwmGTDRTaskThread::CreateResultLayerData mResultList;
    // //类别标识符 1为average，2为correlation
    // int mType = 0;
};

#endif // GWMLAYERGTDRITEM_H

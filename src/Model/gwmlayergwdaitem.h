#ifndef GWMLAYERGWDAITEM_H
#define GWMLAYERGWDAITEM_H

#include "gwmlayervectoritem.h"
#include "TaskThread/gwmgwdataskthread.h"
#include "SpatialWeight/gwmbandwidthweight.h"

class GwmLayerGWDAItem : public GwmLayerVectorItem
{
public:
    explicit GwmLayerGWDAItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmGWDATaskThread* taskThread = nullptr);

    virtual int childNumber() override;

    inline virtual GwmLayerItemType itemType() override { return GwmLayerItemType::GWDA; }

    virtual bool readXml(QDomNode &node) override;
    virtual bool writeXml(QDomNode &node, QDomDocument &doc) override;

    int dataPointsSize() const;

    double correctRate() const { return mCorrectRate; }

    GwmVariable groupVariable() const { return mGroupVariable; }

    QList<GwmVariable> independentVariables() const { return mIndependentVariables; }

    GwmBandwidthWeight weight() const { return mWeight; }

    GwmBandwidthWeight* bandwidth() const { return mBandwidth; }

    bool isWqda() const { return mIsWqda; }

    bool hasCov() const { return mHasCov; }

    bool hasMean() const { return mHasMean; }

    bool hasPrior() const { return mHasPrior; }

    ~GwmLayerGWDAItem();

protected:
    int mDataPointsSize;
    GwmVariable mGroupVariable;
    QList<GwmVariable> mIndependentVariables;
    GwmBandwidthWeight mWeight;
    GwmBandwidthWeight* mBandwidth;
    double mCorrectRate;
    bool mIsWqda;
    bool mHasCov;
    bool mHasMean;
    bool mHasPrior;
};

#endif // GWMLAYERGWDAITEM_H

#ifndef GWMLAYERGWPCAITEM_H
#define GWMLAYERGWPCAITEM_H

#include "prefix.h"

#include <QObject>
#include <qgsvectorlayer.h>
#include <armadillo>
#include "gwmlayervectoritem.h"
#include "Model/gwmlayerattributeitem.h"

#include "TaskThread/gwmgwpcataskthread.h"

class GwmLayerGWPCAItem : public GwmLayerVectorItem
{
    Q_OBJECT
public:
    explicit GwmLayerGWPCAItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmGWPCATaskThread* taskThread = nullptr);

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::GWPCA; }

    virtual bool readXml(QDomNode &node) override;
    virtual bool writeXml(QDomNode &node, QDomDocument &doc) override;

public:
    GwmBandwidthWeight weight() const;

    // GwmLayerItem interface
public:
    bool isBandwidthOptimized;

    bool bandwidthOptimized() const
    {
        return isBandwidthOptimized;
    }

    QList<QPair<double, double> > bandwidthSelScores() const;

public:
    int mK = 0;
    mat mDResult1;
    mat mLocalPV;
    //存其他参数
    cube mLoadings;
    cube mScores;
    mat mVariance;
    GwmBandwidthWeight mWeight;
    QList<QPair<double, double> > mBandwidthSelScores;

};

#endif // GWMLAYERBASICGWRITEM_H

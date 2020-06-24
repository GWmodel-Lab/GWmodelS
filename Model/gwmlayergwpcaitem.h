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

protected:

public:

    GwmBandwidthWeight weight() const;
    GwmBandwidthWeight mWeight;

    double mk=2;
    mat mdResult1;
    mat mLocalPV;
    //存其他参数
    cube mLoadings;
    cube mScores;
    mat mVariance;
    // GwmLayerItem interface
public:
    bool isBandwidthOptimized;

    GwmLayerItemType itemType() {return GwmLayerItemType::GWPCA;};

    bool GwmLayerGWPCAItem::bandwidthOptimized() const
    {
        return isBandwidthOptimized;
    }

    QList<QPair<double, double> > mBandwidthSelScores;
    QList<QPair<double, double> > bandwidthSelScores() const;
};

#endif // GWMLAYERBASICGWRITEM_H

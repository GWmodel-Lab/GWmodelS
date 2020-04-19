#ifndef GWMLAYERGWRITEM_H
#define GWMLAYERGWRITEM_H

#include "prefix.h"

#include <QObject>
#include <qgsvectorlayer.h>
#include "gwmlayervectoritem.h"
#include "Model/gwmlayerattributeitem.h"
#include "TaskThread/gwmgwrtaskthread.h"

class GwmLayerGWRItem : public GwmLayerVectorItem
{
    Q_OBJECT
public:
    explicit GwmLayerGWRItem(GwmLayerItem* parentItem = nullptr, QgsVectorLayer* vector = nullptr, const GwmGWRTaskThread* taskThread = nullptr);

    virtual int childNumber() override;

    inline virtual GwmLayerItemType itemType() { return GwmLayerItemType::GWR; }

    int dataPointsSize() const;

    GwmLayerAttributeItem *depVar() const;

    QList<GwmLayerAttributeItem *> indepVars() const;

    double bandwidthSize() const;

    QString bandwidthUnit() const;

    GwmGWRTaskThread::BandwidthType bandwidthType() const;

    GwmGWRTaskThread::KernelFunction bandwidthKernelFunction() const;

    GwmGWRDiagnostic diagnostic() const;

private:
    int mDataPointsSize;
    GwmLayerAttributeItem* mDepVar;
    QList<GwmLayerAttributeItem*> mIndepVars;
    double mBandwidthSize;
    QString mBandwidthUnit;
    GwmGWRTaskThread::BandwidthType mBandwidthType;
    GwmGWRTaskThread::KernelFunction mBandwidthKernelFunction;
    GwmGWRDiagnostic mDiagnostic;
};

#endif // GWMLAYERGWRITEM_H

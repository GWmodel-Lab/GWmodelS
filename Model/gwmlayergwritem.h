#ifndef GWMLAYERGWRITEM_H
#define GWMLAYERGWRITEM_H

#include "prefix.h"

#include <QObject>
#include <qgsvectorlayer.h>
#include <armadillo>
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

    double bandwidthSize() const;

    QString bandwidthUnit() const;

    GwmGWRTaskThread::BandwidthType bandwidthType() const;

    GwmGWRTaskThread::KernelFunction bandwidthKernelFunction() const;

    GwmGWRDiagnostic diagnostic() const;

    arma::mat betas() const;

    QList<QStringList> modelSelModels() const;

    QList<double> modelSelAICcs() const;

    bool getIsModelOptimized() const;

    bool getIsBandwidthOptimized() const;

    int getDepVarIndex() const;

    QList<int> getIndepVarIndex() const;

    QList<GwmLayerAttributeItem *> getIndepVarsOrigin() const;

private:
    int mDataPointsSize;
    int mDepVarIndex;
    QList<int> mIndepVarsIndex;
    QList<GwmLayerAttributeItem*> mIndepVarsOrigin;
    double mBandwidthSize;
    QString mBandwidthUnit;
    GwmGWRTaskThread::BandwidthType mBandwidthType;
    GwmGWRTaskThread::KernelFunction mBandwidthKernelFunction;
    GwmGWRDiagnostic mDiagnostic;
    arma::mat mBetas;
    QList<QStringList> mModelSelModels;
    QList<double> mModelSelAICcs;

    bool isModelOptimized;
    bool isBandwidthOptimized;
};

#endif // GWMLAYERGWRITEM_H

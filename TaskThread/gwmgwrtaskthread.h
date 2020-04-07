#ifndef GWMGWRTASKTHREAD_H
#define GWMGWRTASKTHREAD_H

#include "gwmtaskthread.h"
#include <qgsvectorlayer.h>
#include <armadillo>

#include "Model/gwmlayerattributeitem.h"

using namespace arma;

class GwmGWRTaskThread : public GwmTaskThread
{
    Q_OBJECT

public:

    enum BandwidthType
    {
        Adaptive,
        Fixed
    };

    enum ParallelMethod
    {
        None,
        Multithread,
        GPU
    };

    enum KernelFunction
    {
        Gaussian,
        Exponential,
        Bisquare,
        Tricube,
        Boxcar
    };

    enum DistanceSourceType
    {
        CRS,
        Minkowski,
        DMatFile
    };

public:
    GwmGWRTaskThread(QgsVectorLayer* layer, GwmLayerAttributeItem* depVar, QList<GwmLayerAttributeItem*> indepVars);

protected:
    void run() override;

public:
    bool isValid(QString& message);

private:
    QgsVectorLayer* mLayer = nullptr;
    GwmLayerAttributeItem* mDepVar;
    QList<GwmLayerAttributeItem*> mIndepVars;
    int mDepVarIndex;
    QList<int> mIndepVarsIndex;
    bool isEnableIndepVarAutosel = false;

    BandwidthType mBandwidthType = BandwidthType::Adaptive;
    double mBandwidthSize = 0.0;
    bool isBandwidthSizeAutoSel = true;
    KernelFunction mBandwidthKernelFunction = KernelFunction::Gaussian;

    DistanceSourceType mDistSrcType = DistanceSourceType::CRS;
    QVariant mDistSrcParameters = QVariant();

    double mCRSRotateTheta = 0.0;
    double mCRSRotateP = 0.0;

    ParallelMethod mParallelMethodType = ParallelMethod::None;
    QVariant mParallelParameter = 0;

    QList<QgsFeatureId> mFeatureIds;

    mat mX;
    mat mY;
    mat mBetas;
    mat mDataPoints;

private:
    bool isNumeric(QVariant::Type type);
    bool setXY();

    vec distance(const QgsFeatureId& id);
    vec distanceCRS(const QgsFeatureId& id);
    vec distanceMinkowski(const QgsFeatureId& id);
};

#endif // GWMGWRTASKTHREAD_H

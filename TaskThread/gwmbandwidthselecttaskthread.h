#ifndef GWMBANDWIDTHSELECT_H
#define GWMBANDWIDTHSELECT_H

#include "gwmgwrtaskthread.h"
#include <qgsvectorlayer.h>
#include <armadillo>

class GwmBandwidthSelectTaskThread;
typedef double (GwmBandwidthSelectTaskThread::*pfApproach)(const mat& , const vec& , const mat& , double , int , bool );

class GwmBandwidthSelectTaskThread:public GwmGWRTaskThread
{
    Q_OBJECT
public:
    GwmBandwidthSelectTaskThread(QgsVectorLayer* layer, GwmLayerAttributeItem* depVar, QList<GwmLayerAttributeItem*> indepVars);
public:
    enum Approach
    {
        CV,
        AICc
    };
protected:
    void run() override;
private:
    QgsVectorLayer* mLayer = nullptr;
    GwmLayerAttributeItem* mDepVar;
    QList<GwmLayerAttributeItem*> mIndepVars;
    QList<QgsFeatureId> mFeatureIds;
    mat mX;
    mat mY;
    mat mBetas;
    mat mDataPoints;
    int mDepVarIndex;
    QList<int> mIndepVarsIndex;
    vec distance(const QgsFeatureId& id);
    vec distanceCRS(const QgsFeatureId& id);
    vec distanceMinkowski(const QgsFeatureId& id);
    DistanceSourceType mDistSrcType = DistanceSourceType::CRS;
    QVariant mDistSrcParameters = QVariant();
    bool setXY();
    bool isNumeric(QVariant::Type type);

    BandwidthType mBandwidthType = BandwidthType::Adaptive;
    double mBandwidthSize = 0.0;
    bool isBandwidthSizeAutoSel = true;
    KernelFunction mBandwidthKernelFunction = KernelFunction::Gaussian;

    Approach mApproach = Approach::CV;

    double gwCVAll(const mat& x, const vec& y,const mat& dp,double bw, int kernel, bool adaptive);
    double gwrCV(double bw, mat x, vec y, int kernel, bool adaptive, mat dp);

    double AICc1(vec y, mat x, mat beta, vec s_hat);

    double AICRes(const mat& x, const vec& y, const mat& dp,double bw,int kernel,bool adaptive);

    double gold(pfApproach p,double xL, double xU, bool adaptBw,const mat& x, const vec& y, const mat& dp, int kernel, bool adaptive);
};

#endif // GWMBANDWIDTHSELECT_H

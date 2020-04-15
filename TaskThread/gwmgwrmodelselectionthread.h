#ifndef GWMGWRMODELSELECTIONTHREAD_H
#define GWMGWRMODELSELECTIONTHREAD_H

#include "gwmgwrtaskthread.h"
#include <qgsvectorlayer.h>
#include <Model/gwmlayerattributeitem.h>


class GwmGWRModelSelectionThread : public GwmGWRTaskThread
{
     Q_OBJECT

public:
    GwmGWRModelSelectionThread(QgsVectorLayer* layer, GwmLayerAttributeItem* depVar, QList<GwmLayerAttributeItem*> indepVars);

public:
    mat gw_reg_all(mat X,mat Y);
    bool isNumeric(QVariant::Type type);
    QList<mat> setXY(int depVarIndex,QList<int> inDepVarsIndex);
    vec distance(const QgsFeatureId& id);
    vec distanceCRS(const QgsFeatureId& id);
    vec distanceMinkowski(const QgsFeatureId& id);
    bool calDmat();
    QList<QStringList> modelSort(QList<QStringList> modelList,QList<double> modelAICcs);
    QMap<QStringList,double> modelSelection();

protected:
    void run() override;

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

    QList<QStringList> mModelInDepVars;
    QList<double> mModelAICcs;

    mat mDmat;

    mat mX;
    mat mY;
    mat mBetas;
    mat mDataPoints;
};

#endif // GWMGWRMODELSELECTIONTHREAD_H

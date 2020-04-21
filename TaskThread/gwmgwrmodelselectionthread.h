#ifndef GWMGWRMODELSELECTIONTHREAD_H
#define GWMGWRMODELSELECTIONTHREAD_H

#include "gwmgwrtaskthread.h"
#include <qgsvectorlayer.h>
#include <Model/gwmlayerattributeitem.h>

#include <qwt_plot_curve.h>
#include<qwt_plot.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_grid.h>
#include <qpen.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_legend_label.h>
#include <qwt_column_symbol.h>

class GwmGWRModelSelectionThread : public GwmGWRTaskThread
{
     Q_OBJECT

public:
    GwmGWRModelSelectionThread();

public:
    mat gw_reg_all(mat X,mat Y);
//    bool isNumeric(QVariant::Type type);
    QList<mat> setXY(int depVarIndex,QList<int> inDepVarsIndex);
//    vec distance(int focus);
//    vec distanceCRS(int focus);
//    vec distanceMinkowski(int focus);
    bool calDmat();
//    void setBandwidth(BandwidthType type, double size, QString unit);

//    QgsVectorLayer *layer() const;
//    void setLayer(QgsVectorLayer *layer);

//    QList<GwmLayerAttributeItem *> indepVars() const;
//    void setIndepVars(const QList<GwmLayerAttributeItem *> &indepVars);

//    GwmLayerAttributeItem *depVar() const;
//    void setDepVar(GwmLayerAttributeItem *depVar);

    QList<QStringList> modelSort(QList<QStringList> modelList,QList<double> modelAICcs);
    QMap<QStringList,double> modelSelection();

    void model_View();

//    QwtPlot* resultPlot();

protected:
    void run() override;

private:
//    QgsVectorLayer* mLayer = nullptr;
//    GwmLayerAttributeItem* mDepVar;
//    QList<GwmLayerAttributeItem*> mIndepVars;
//    int mDepVarIndex;
//    QList<int> mIndepVarsIndex;
//    bool isEnableIndepVarAutosel = false;

//    BandwidthType mBandwidthType = BandwidthType::Adaptive;
//    double mBandwidthSize = 0.0;
//    bool isBandwidthSizeAutoSel = true;
//    KernelFunction mBandwidthKernelFunction = KernelFunction::Gaussian;

//    DistanceSourceType mDistSrcType = DistanceSourceType::CRS;
//    QVariant mDistSrcParameters = QVariant();

//    double mCRSRotateTheta = 0.0;
//    double mCRSRotateP = 0.0;

//    ParallelMethod mParallelMethodType = ParallelMethod::None;
//    QVariant mParallelParameter = 0;

//    QgsFeatureList mFeatureList;

    QList<QStringList> mModelInDepVars;
    QList<double> mModelAICcs;

    mat mDmat;

//    mat mX;
//    mat mY;
//    mat mBetas;
//    mat mDataPoints;

//public:

//    QwtPlot* plot;
};

#endif // GWMGWRMODELSELECTIONTHREAD_H

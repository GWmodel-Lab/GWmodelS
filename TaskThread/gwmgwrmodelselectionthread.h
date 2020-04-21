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

    QList<mat> setXY(int depVarIndex,QList<int> inDepVarsIndex);

    bool calDmat();

    QList<QStringList> modelSort(QList<QStringList> modelList,QList<double> modelAICcs);
    QMap<QStringList,double> modelSelection();

    void model_View();


protected:
    void run() override;

private:

    QList<QStringList> mModelInDepVars;
    QList<double> mModelAICcs;

    mat mDmat;
};

#endif // GWMGWRMODELSELECTIONTHREAD_H

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
#include <qwt_plot_textlabel.h>

class GwmGWRModelSelectionThread : public GwmGWRTaskThread
{
     Q_OBJECT

public:
    static void plotModelOrder(QVariant data, QwtPlot* plot);
    static void plotModelAICcs(QVariant data, QwtPlot* plot);

public:
    GwmGWRModelSelectionThread();
    GwmGWRModelSelectionThread(const GwmGWRTaskThread& gwrTaskThread);

public:
    double gwRegAll();

    QList<mat> setXY(int depVarIndex,QList<int> inDepVarsIndex);

    bool calDmat();

    QPair<QList<int>,double> modelSelection();
    QList<QStringList> modelSort(QList<QStringList> modelList,QList<double> modelAICcs,QList<QList<int>> modelIndexList);

    void viewModels();


    QList<QStringList> getModelInDepVars();
    QList<double> getModelAICcs();
    QList<QList<int>> getModelInDepVarsIndex();

protected:
    void run() override;

private:
    bool createdFromGWRTaskThread = false;

    QList<QStringList> mModelInDepVars;
    QList<double> mModelAICcs;

    QList<QList<int>> mModelInDepVarsIndex;

    mat mDmat;

private:
    double getFixedBwUpper();
};

#endif // GWMGWRMODELSELECTIONTHREAD_H

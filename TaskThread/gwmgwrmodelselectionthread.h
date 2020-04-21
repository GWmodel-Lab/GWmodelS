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
    GwmGWRModelSelectionThread(const GwmGWRTaskThread& gwrTaskThread);

public:
    mat gwRegAll();

    QList<mat> setXY(int depVarIndex,QList<int> inDepVarsIndex);

    bool calDmat();

    QList<QStringList> modelSort(QList<QStringList> modelList,QList<double> modelAICcs);
    QMap<QStringList,double> modelSelection();

    void viewModels();


protected:
    void run() override;

private:
    bool createdFromGWRTaskThread = false;

    QList<QStringList> mModelInDepVars;
    QList<double> mModelAICcs;

    mat mDmat;

private:
    double getFixedBwUpper();
};

#endif // GWMGWRMODELSELECTIONTHREAD_H

#ifndef GWMBANDWIDTHSELECT_H
#define GWMBANDWIDTHSELECT_H

#include "gwmgwrtaskthread.h"
#include <qgsvectorlayer.h>
#include <armadillo>

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

class GwmBandwidthSelectTaskThread;
typedef double (GwmBandwidthSelectTaskThread::*pfApproach)(const mat& , const vec& , const mat& , double , int , bool );

class GwmBandwidthSelectTaskThread:public GwmGWRTaskThread
{
    Q_OBJECT
public:
    GwmBandwidthSelectTaskThread();

    GwmBandwidthSelectTaskThread(const GwmGWRTaskThread& gwrTaskThread);
    // 根据带宽和Aic/cv值输出结果图
    static void plotBandwidthResult(QVariant data, QwtPlot *plot);
    // 获取bw和score
    QMap<double,double> getBwScore();
protected:
    void run() override;

private:
    bool createdFromGWRTaskThread = false;

    double cvAll(const mat& x, const vec& y,const mat& dp,double bw, int kernel, bool adaptive);
    double aicAll(const mat& x, const vec& y, const mat& dp,double bw,int kernel,bool adaptive);

    double gold(pfApproach p,double xL, double xU, bool adaptBw,const mat& x, const vec& y, const mat& dp, int kernel, bool adaptive);

    double getFixedBwUpper();

    QString createOutputMessage(double bw, double score);

    QMap<double,double> mBwScore;

    QwtPlot* mPlot=nullptr;
};

#endif // GWMBANDWIDTHSELECT_H

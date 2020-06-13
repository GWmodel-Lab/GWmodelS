#include "gwmbandwidthsizeselector.h"

#include <qwt_plot_curve.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qpen.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_legend_label.h>
#include <qwt_column_symbol.h>

void GwmBandwidthSizeSelector::PlotBandwidthResult(QVariant data, QwtPlot *plot)
{
    BandwidthCriterionList result = data.value<BandwidthCriterionList>();
    //设置窗口属性
    plot->plotLayout()->setAlignCanvasToScales(true);
    //新建一个曲线对象
    QwtPlotCurve *curve = new QwtPlotCurve("curve");
    //设置曲线颜色 粗细
    curve->setPen(Qt::blue,1.0,Qt::DashLine);
    //线条光滑化
    curve->setRenderHint(QwtPlotItem::RenderAntialiased,true);
    //设置样本点的颜色、大小
    QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse, QBrush( Qt::yellow ), QPen( Qt::red, 0.5 ), QSize( 5, 5) );
    //添加样本点形状
    curve->setSymbol( symbol );
    //输入数据
    QVector<double> xData;
    QVector<double> yData;
    for(auto i = result.constBegin();i!=result.constEnd();++i){
        xData.push_back(i->first);
        yData.push_back(i->second);
    }
    //设置X与Y坐标范围
    //返回xData与yData最大最小值
    //拷贝xData与yData并返回sort
    QVector<double> xData_2(xData);
    QVector<double> yData_2(yData);
    //从小到大排序
    std::sort(xData_2.begin(),xData_2.end());
    std::sort(yData_2.begin(),yData_2.end());
    plot->setAxisScale(QwtPlot::xBottom,xData_2[0],xData_2[xData.length()-1]);
    plot->setAxisScale(QwtPlot::yLeft, yData_2[0], yData_2[yData.length()-1]);
    //设置数据
    curve->setSamples(xData,yData);
    curve->attach(plot);
    curve->setLegendAttribute(curve->LegendShowLine);

    plot->replot();
}

GwmBandwidthSizeSelector::GwmBandwidthSizeSelector()
{

}

QList<QPair<double, double> > GwmBandwidthSizeSelector::bandwidthCriterion() const
{
    QList<QPair<double, double> > criterions;
    for (double key : mBandwidthCriterion.keys())
    {
        criterions.append(qMakePair(key, mBandwidthCriterion[key]));
    }
    std::sort(criterions.begin(), criterions.end(), [](const QPair<double, double>& a, const QPair<double, double>& b){
        return a.first < b.first;
    });
    return criterions;
}

GwmBandwidthWeight* GwmBandwidthSizeSelector::optimize(IBandwidthSizeSelectable *instance)
{
    GwmBandwidthWeight* w1 = new GwmBandwidthWeight(*mBandwidth);
    GwmBandwidthWeight* w2 = new GwmBandwidthWeight(*mBandwidth);
    double xU = mUpper, xL = mLower;
    bool adaptBw = mBandwidth->adaptive();
    const double eps = 1e-4;
    const double R = (sqrt(5)-1)/2;
    int iter = 0;
    double d = R * (xU - xL);
    double x1 = adaptBw ? floor(xL + d) : (xL + d);
    double x2 = adaptBw ? round(xU - d) : (xU - d);
    w1->setBandwidth(x1);
    w2->setBandwidth(x2);
    double f1 = instance->criterion(w1);
    double f2 = instance->criterion(w2);
    mBandwidthCriterion[x1] = f1;
    mBandwidthCriterion[x2] = f2;
    double d1 = f2 - f1;
    double xopt = f1 < f2 ? x1 : x2;
    double ea = 100;
    while ((fabs(d) > eps) && (fabs(d1) > eps) && iter < ea)
    {
        d = R * d;
        if (f1 < f2)
        {
            xL = x2;
            x2 = x1;
            x1 = adaptBw ? round(xL + d) : (xL + d);
            f2 = f1;
            w1->setBandwidth(x1);
            f1 = instance->criterion(w1);
            mBandwidthCriterion[x1] = f1;
        }
        else
        {
            xU = x1;
            x1 = x2;
            x2 = adaptBw ? floor(xU - d) : (xU - d);
            f1 = f2;
            w2->setBandwidth(x2);
            f2 = instance->criterion(w2);
            mBandwidthCriterion[x2] = f2;
        }
        iter = iter + 1;
        xopt = (f1 < f2) ? x1 : x2;
        d1 = f2 - f1;
    }
    delete w1;
    delete w2;
    GwmBandwidthWeight* wopt = new GwmBandwidthWeight(*mBandwidth);
    wopt->setBandwidth(xopt);
    return wopt;
}

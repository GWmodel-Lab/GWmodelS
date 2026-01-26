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
#include <QDebug>

void GwmBandwidthSizeSelector::PlotBandwidthResult(QVariant data, QwtPlot *plot)
{
    if (!data.canConvert<QVector<QPair<double,double>>>()) {
        qDebug() << "Data cannot convert to QVector<QPair<double,double>>!";
        return;
    }

    QVector<QPair<double,double>> result = data.value<QVector<QPair<double,double>>>();
    qDebug() << "PlotBandwidthResult received size:" << result.size();

    QVector<double> xData, yData;
    for (int i = 0; i < result.size(); ++i) {
        qDebug() << "Bandwidth:" << result[i].first << ", Criterion:" << result[i].second;
        xData.push_back(result[i].first);
        yData.push_back(result[i].second);
    }

    if (xData.isEmpty() || yData.isEmpty()) {
        qDebug() << "xData or yData is empty!";
        return;
    }

    plot->plotLayout()->setAlignCanvasToScales(true);

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
    QVector<QPair<double, double>> points;
    points.reserve(result.size());
    for (auto it = result.constBegin(); it != result.constEnd(); ++it)
    {
        points.append(qMakePair(it->first, it->second));
    }
    if (points.isEmpty())
    {
        plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
        plot->replot();
        return;
    }
    std::sort(points.begin(), points.end(), [](const QPair<double, double>& a, const QPair<double, double>& b){
        if (a.first == b.first)
            return a.second < b.second;
        return a.first < b.first;
    });
    QVector<double> xData;
    QVector<double> yData;
    xData.reserve(points.size());
    yData.reserve(points.size());
    for (const auto& pt : points)
    {
        xData.append(pt.first);
        yData.append(pt.second);
    }
    QVector<double> xDataSorted(xData);
    QVector<double> yDataSorted(yData);
    std::sort(xDataSorted.begin(), xDataSorted.end());
    std::sort(yDataSorted.begin(), yDataSorted.end());
    plot->setAxisScale(QwtPlot::xBottom, xDataSorted.first(), xDataSorted.last());
    plot->setAxisScale(QwtPlot::yLeft, yDataSorted.first(), yDataSorted.last());
    curve->setSamples(xData, yData);
    curve->attach(plot);

    // 设置坐标轴范围
    auto [xMinIt, xMaxIt] = std::minmax_element(xData.begin(), xData.end());
    auto [yMinIt, yMaxIt] = std::minmax_element(yData.begin(), yData.end());
    plot->setAxisScale(QwtPlot::xBottom, *xMinIt, *xMaxIt);
    plot->setAxisScale(QwtPlot::yLeft, *yMinIt, *yMaxIt);

    qDebug() << "x range:" << *xMinIt << "-" << *xMaxIt;
    qDebug() << "y range:" << *yMinIt << "-" << *yMaxIt;

    curve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    plot->replot();
    qDebug() << "Plot replot done";

}



GwmBandwidthSizeSelector::GwmBandwidthSizeSelector()
{

}

BandwidthCriterionList GwmBandwidthSizeSelector::bandwidthCriterion() const
{
    BandwidthCriterionList criterions;
    for (auto it = mBandwidthCriterion.constBegin(); it != mBandwidthCriterion.constEnd(); ++it)
    {
        criterions.push_back(std::make_pair(it.key(), it.value()));
    }
    std::sort(criterions.begin(), criterions.end(),
              [](const std::pair<double, double>& a, const std::pair<double, double>& b){
                  return a.first < b.first;
              });
    return criterions;
}

GwmBandwidthWeight* GwmBandwidthSizeSelector::optimize(IBandwidthSizeSelectable *instance)
{
    GwmBandwidthWeight* w1 = static_cast<GwmBandwidthWeight*>(mBandwidth->clone());
    GwmBandwidthWeight* w2 = static_cast<GwmBandwidthWeight*>(mBandwidth->clone());
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
    double f1 = checkCanceled() ? DBL_MAX : instance->criterion(w1);
    counter++;
    double f2 = checkCanceled() ? DBL_MAX : instance->criterion(w2);
    counter++;
    if (f1 < DBL_MAX)
        mBandwidthCriterion[x1] = f1;
    if (f2 < DBL_MAX)
        mBandwidthCriterion[x2] = f2;
    double d1 = f2 - f1;
    double xopt = f1 < f2 ? x1 : x2;
    double ea = 100;
    while ((fabs(d) > eps) && (fabs(d1) > eps) && iter < ea && !checkCanceled())
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
            if (f1 < DBL_MAX)
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
            if (f2 < DBL_MAX)
                mBandwidthCriterion[x2] = f2;
        }
        iter = iter + 1;
        xopt = (f1 < f2) ? x1 : x2;
        d1 = f2 - f1;
        counter++;
    }
    delete w1;
    delete w2;
    if(!checkCanceled())
    {
        GwmBandwidthWeight* wopt = new GwmBandwidthWeight();
        wopt->setKernel(mBandwidth->kernel());
        wopt->setAdaptive(mBandwidth->adaptive());
        wopt->setBandwidth(xopt);
        return wopt;
    }
    else return nullptr;
}

bool GwmBandwidthSizeSelector::checkCanceled()
{
    if(isCanceled())
    {
        return true;
    }
    else
    {
        return false;
    }
}

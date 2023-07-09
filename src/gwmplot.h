#ifndef GWMPLOT_H
#define GWMPLOT_H

#include <qwt_plot.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>

class GwmPlot : public QwtPlot
{
public:
    GwmPlot(QWidget* parent = nullptr);

    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    QwtPlotMagnifier* mMagnifier;
    QwtPlotPanner* mPanner;
};

#endif // GWMPLOT_H

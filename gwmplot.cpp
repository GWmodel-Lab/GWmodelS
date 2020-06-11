#include "gwmplot.h"

#include <QEvent>

GwmPlot::GwmPlot(QWidget* parent)
    : QwtPlot(parent)
{
    mMagnifier = new QwtPlotMagnifier(canvas());
    mPanner = new QwtPlotPanner(canvas());
    installEventFilter(this);
}

bool GwmPlot::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == this)
    {
        if (e->type() == QEvent::Wheel)
        {
            e->setAccepted(true);
            return true;
        }
        else return false;
    }
    else return false;
}

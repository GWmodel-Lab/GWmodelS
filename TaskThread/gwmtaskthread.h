#ifndef GWMTASKTHREAD_H
#define GWMTASKTHREAD_H

#include <QThread>
#include <QMetaType>
#include <qwt_plot.h>

typedef void (*PlotFunction)(QVariant data, QwtPlot* widget);

class GwmTaskThread : public QThread
{
    Q_OBJECT
public:
    GwmTaskThread();

signals:
    void tick(int current, int total);
    void message(QString message);
    void success();
    void error(QString e);
    void plot(QVariant data, PlotFunction func);

public:
    virtual QString name() const;
};

#endif // GWMTASKTHREAD_H

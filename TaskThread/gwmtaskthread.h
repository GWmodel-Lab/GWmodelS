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

    bool isCanceled() const;
    void setCanceled(bool newCanceled);

signals:
    void tick(int current, int total);
    void message(QString message);
    void success();
    void canceled();
    void error(QString e);
    void plot(QVariant data, PlotFunction func);

public:
    virtual QString name() const;

protected:
    bool mIsCanceled = false;
};

inline bool GwmTaskThread::isCanceled() const
{
    return mIsCanceled;
}

inline void GwmTaskThread::setCanceled(bool newCanceled)
{
    mIsCanceled = newCanceled;
}

#endif // GWMTASKTHREAD_H

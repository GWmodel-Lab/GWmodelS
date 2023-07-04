#ifndef GWMTASKTHREAD_H
#define GWMTASKTHREAD_H

#include <QDebug>
#include <QThread>
#include <QMetaType>
#include <qwt_plot.h>
#include <gwmodel.h>

typedef void (*PlotFunction)(QVariant data, QwtPlot* widget);


class GwmTaskThread : public QThread
{
    Q_OBJECT
public:
    GwmTaskThread() {};

    bool isCanceled() const
    {
        return mIsCanceled;
    }

    virtual void setCanceled(bool newCanceled)
    {
        mIsCanceled = newCanceled;
    }

signals:
    void tick(int current, int total) const;
    void message(QString message) const;
    void success() const;
    void canceled() const;
    void error(QString e) const;
    void plot(QVariant data, PlotFunction func) const;

public:
    void print_error(QString message) const { emit error(message); }
    void print_message(QString msg) const { emit message(msg); }
    void progress(std::size_t current, std::size_t total) const { emit tick(current, total); }
    void progress(double percent) const { emit tick(percent * 100, 100); }

public:
    virtual QString name() const;

public:
    bool checkCanceled() const;

protected:
    bool mIsCanceled = false;
};


class GwmTaskThreadTelegram : public gwm::ITelegram
{
public:
    GwmTaskThreadTelegram() {}

    GwmTaskThreadTelegram(const GwmTaskThread* task) : mTask(task) {}

    ~GwmTaskThreadTelegram() {}

public:

    void print(std::string message, gwm::ITelegram::LogLevel level, std::string fun_name, std::string file_name);

    void progress(std::size_t current, std::size_t total)
    {
        mTask->progress(current, total); 
    }

    void progress(double percent) { mTask->progress(percent); }

    bool stop() { return mTask->checkCanceled(); }

private:
    const GwmTaskThread* mTask = nullptr;
};

#endif // GWMTASKTHREAD_H

#ifndef GWMTASKTHREAD_H
#define GWMTASKTHREAD_H

#include <QThread>

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
};

#endif // GWMTASKTHREAD_H

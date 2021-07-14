#ifndef GWMPROGRESSDIALOG_H
#define GWMPROGRESSDIALOG_H

#include <QDialog>
#include "TaskThread/gwmtaskthread.h"

namespace Ui {
class GwmProgressDialog;
}

class GwmProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmProgressDialog(GwmTaskThread* thread, QWidget *parent = nullptr);
    ~GwmProgressDialog();

public:
    virtual void showEvent(QShowEvent *e) override;

    GwmTaskThread *taskThread() const;
    void setTaskThread(GwmTaskThread *taskThread);

    virtual void accept() override;
    virtual void reject() override;

public slots:
    void onTick(int current, int total);
    void onMessage(QString message);
    void onSuccess();
    void onCanceled();
    void onError(QString e);
    void onPlot(QVariant data, PlotFunction func);

private:
    Ui::GwmProgressDialog *ui;
    GwmTaskThread* mTaskThread;
    bool isAutoClose = true;

private:
    void onAutoCloseToggled(bool checked);
};

#endif // GWMPROGRESSDIALOG_H

#include "gwmprogressdialog.h"
#include "ui_gwmprogressdialog.h"

#include <QDebug>

GwmProgressDialog::GwmProgressDialog(GwmTaskThread* thread, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmProgressDialog),
    mTaskThread(thread)
{
    ui->setupUi(this);
    setWindowTitle(thread->name());
    connect(mTaskThread, &GwmTaskThread::tick, this, &GwmProgressDialog::onTick);
    connect(mTaskThread, &GwmTaskThread::message, this, &GwmProgressDialog::onMessage);
    connect(mTaskThread, &GwmTaskThread::success, this, &GwmProgressDialog::onSuccess);
    connect(mTaskThread, &GwmTaskThread::error, this, &GwmProgressDialog::onError);
}

GwmProgressDialog::~GwmProgressDialog()
{
    delete ui;
}

void GwmProgressDialog::showEvent(QShowEvent *e)
{
    mTaskThread->start();
}

GwmTaskThread *GwmProgressDialog::taskThread() const
{
    return mTaskThread;
}

void GwmProgressDialog::setTaskThread(GwmTaskThread *taskThread)
{
    mTaskThread = taskThread;
}

void GwmProgressDialog::accept()
{
    return QDialog::accept();
}

void GwmProgressDialog::reject()
{
    mTaskThread->exit(1);
    return QDialog::reject();
}

void GwmProgressDialog::onTick(int current, int total)
{
//    qDebug() << "[GwmProgressDialog::onTick]"
//             << "current" << current << "total" << total;
    ui->progressBar->setRange(0, total);
    ui->progressBar->setValue(current);
}

void GwmProgressDialog::onMessage(QString message)
{
    qDebug() << "[GwmProgressDialog::onMessage]"
             << "message" << message;
    ui->progressMessage->setText(message);
}

void GwmProgressDialog::onSuccess()
{
    qDebug() << "[GwmProgressDialog::onSuccess]"
             << "onSuccess";
    accept();
}

void GwmProgressDialog::onError(QString e)
{
    qDebug() << "[GwmProgressDialog::onError]"
             << "error" << e;
    ui->progressMessage->setText(QStringLiteral("Error: ") + e);
}

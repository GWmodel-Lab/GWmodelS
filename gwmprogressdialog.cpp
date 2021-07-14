#include "gwmprogressdialog.h"
#include "ui_gwmprogressdialog.h"

#include <QDebug>
#include "gwmplot.h"

GwmProgressDialog::GwmProgressDialog(GwmTaskThread* thread, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmProgressDialog),
    mTaskThread(thread)
{
    ui->setupUi(this);
    setWindowTitle(thread->name());

    connect(ui->cbxAutoClose, &QAbstractButton::toggled, this, &GwmProgressDialog::onAutoCloseToggled);

    connect(mTaskThread, &GwmTaskThread::tick, this, &GwmProgressDialog::onTick);
    connect(mTaskThread, &GwmTaskThread::message, this, &GwmProgressDialog::onMessage);
    connect(mTaskThread, &GwmTaskThread::success, this, &GwmProgressDialog::onSuccess);
    connect(mTaskThread, &GwmTaskThread::error, this, &GwmProgressDialog::onError);
    connect(mTaskThread, &GwmTaskThread::plot, this, &GwmProgressDialog::onPlot);
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
    mTaskThread->setCanceled(true);
    connect(mTaskThread, &GwmTaskThread::canceled, this, &GwmProgressDialog::onCanceled);
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
//    ui->progressMessage->setText(message);
    QLabel* logItem = new QLabel(message, ui->logScrollAreaContents);
    ui->logScrollAreaContents->layout()->addWidget(logItem);
}

void GwmProgressDialog::onSuccess()
{
    qDebug() << "[GwmProgressDialog::onSuccess]"
             << "onSuccess";
    ui->progressMessage->setText("Success");
    if (isAutoClose)
    {
        accept();
    }
    else
    {
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok |
                                          QDialogButtonBox::Cancel);
    }
}

void GwmProgressDialog::onCanceled()
{
    disconnect(mTaskThread, &GwmTaskThread::canceled, this, &GwmProgressDialog::onCanceled);
    qDebug() << "[GwmProgressDialog::onCanceled]" << "mTaskThread live is" << mTaskThread->isRunning();
    return QDialog::reject();
}

void GwmProgressDialog::onError(QString e)
{
    qDebug() << "[GwmProgressDialog::onError]"
             << "error" << e;
    QLabel* logItem = new QLabel(e, ui->logScrollAreaContents);
    logItem->setPalette(QPalette(QPalette::WindowText, Qt::red));
    ui->logScrollAreaContents->layout()->addWidget(logItem);
    ui->progressMessage->setText(QStringLiteral("Error: ") + e);
}

void GwmProgressDialog::onPlot(QVariant data, PlotFunction func)
{
    qDebug() << "[GwmProgressDialog::onPlot]";
    QwtPlot* plot = new GwmPlot(ui->logScrollAreaContents);
    (*func)(data, plot);
    ui->logScrollAreaContents->layout()->addWidget(plot);
}

void GwmProgressDialog::onAutoCloseToggled(bool checked)
{
    isAutoClose = checked;
}

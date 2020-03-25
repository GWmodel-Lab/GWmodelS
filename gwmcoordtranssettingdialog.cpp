#include "gwmcoordtranssettingdialog.h"
#include "ui_gwmcoordtranssettingdialog.h"
#include <QtWidgets>

#include <QString>

#include "qgsprojectionselectionwidget.h"
#include "qgsprojectionselectiondialog.h"

#include "gwmcoordtransthread.h"

GwmCoordTransSettingDialog::GwmCoordTransSettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmCoordTransSettingDialog)
{
    ui->setupUi(this);
    connect(ui->btnOK, &QPushButton::clicked, this, &GwmCoordTransSettingDialog::accept);
    connect(ui->btnCancel, &QPushButton::clicked, this, &GwmCoordTransSettingDialog::reject);

    //m_transThread = new GwmCoordTransThread(this);
}

GwmCoordTransSettingDialog::~GwmCoordTransSettingDialog()
{
    delete ui;
}

void GwmCoordTransSettingDialog::accept()
{
    return QDialog::accept();
}

void GwmCoordTransSettingDialog::reject()
{
    return QDialog::reject();
}


QgsCoordinateReferenceSystem GwmCoordTransSettingDialog::desCrs() const
{
    return ui->mDesCRSSelector->crs();
}

void GwmCoordTransSettingDialog::setDesCrs(const QgsCoordinateReferenceSystem &desCrs)
{
    ui->mDesCRSSelector->setCrs(desCrs);
}

QgsCoordinateReferenceSystem GwmCoordTransSettingDialog::srcCrs() const
{
    return ui->mSrcCRSSelector->crs();
}

void GwmCoordTransSettingDialog::setSrcCrs(const QgsCoordinateReferenceSystem &srcCrs)
{
    ui->mSrcCRSSelector->setCrs(srcCrs);
}

void GwmCoordTransSettingDialog::transformCoordinate(QgsCoordinateReferenceSystem desCrs, QgsVectorLayer *handleLayer)
{
    mTransThread = new GwmCoordTransThread(handleLayer,desCrs);
    connect(mTransThread, &GwmCoordTransThread::percentTransd,this, &GwmCoordTransSettingDialog::updateTransProgress);
    connect(this, &GwmCoordTransSettingDialog::cancelTransSignal, mTransThread, &GwmCoordTransThread::onCancelTrans);

    this->progressDialog = new QProgressDialog();
    this->progressDialog->show();
    this->progressDialog->setModal(true);
    this->progressDialog->setLabelText("Reprojecting ...");
    this->progressDialog->autoClose();

    mTransThread->start();
}

void GwmCoordTransSettingDialog::updateTransProgress(int progress,int total)
{
    this->progressDialog->setRange(0,total-1);
    this->progressDialog->setValue(progress);

    int cancelFlag = 1;

    if(this->progressDialog->wasCanceled()){
        emit cancelTransSignal(cancelFlag);
        this->progressDialog->close();
    }
}

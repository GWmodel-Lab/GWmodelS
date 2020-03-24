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
//    qDebug() << desCrs.authid();
//    qDebug() << handleLayer->fields().names();
//    qDebug() << handleLayer->getFeature(0).geometry().asJson();
    m_transThread = new GwmCoordTransThread(handleLayer,desCrs);

    connect(m_transThread,SIGNAL(percentTransd(int,int)),this,SLOT(updateTransProgress(int,int)));
    connect(this,SIGNAL(cancelTransSig(int)),m_transThread,SLOT(cancelTransSlo(int)));

    this->progressDialog = new QProgressDialog();

    this->progressDialog->show();
//    this->progressDialog->setRange(0,50000);
    this->progressDialog->setModal(true);
    this->progressDialog->setLabelText("Processing......");
    this->progressDialog->autoClose();

//    for(int i=0;i<50000;i++)
//    {
//        for(int j=0;j<20000;j++);
//        Sleep(1);
//        this->progressDialog->setValue(i);
//        if(this->progressDialog->wasCanceled()){
//        break;
//        }
//    }

    m_transThread->start();
}

void GwmCoordTransSettingDialog::updateTransProgress(int progress,int total)
{
    this->progressDialog->setRange(0,total-1);
    this->progressDialog->setValue(progress);

    int cancelFlag = 1;

    if(this->progressDialog->wasCanceled()){
        emit cancelTransSig(cancelFlag);
        this->progressDialog->close();
    }
}

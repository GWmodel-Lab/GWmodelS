#include "gwmcoordinate.h"
#include "ui_gwmcoordinate.h"
#include <QtWidgets>

#include <QString>

#include "qgsprojectionselectionwidget.h"
#include "qgsprojectionselectiondialog.h"

GwmCoordinate::GwmCoordinate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmCoordinate)
{
    ui->setupUi(this);
    //ui->lineEditOri->setText(QString(this->property("resPrj").toString()));
    mCRSSelector = new QgsProjectionSelectionWidget(this);
    ui->horizontalLayout_2->addWidget(mCRSSelector);
}

GwmCoordinate::~GwmCoordinate()
{
    delete ui;
}

void GwmCoordinate::on_btnOK_clicked()
{
    emit sendSigCoordinate(this->mCRSSelector->crs().toWkt(),this->property("MapIndex").toModelIndex());
}

void GwmCoordinate::on_btnCancel_clicked()
{
    this->hide();
}

void GwmCoordinate::on_BtnCrs_clicked()
{
    QgsProjectionSelectionWidget test;
    test.selectCrs();
    //qDebug() << test.crs().toProj();
    //qDebug() << test.crsOptionText(test.crs());
    //this->ui->lineEditTar->setText(test.crsOptionText(test.crs()));
    //this->setProperty("crs",test.crs());
    //emit sendSigSetCoordinate(test.crs(),test.crsOptionText(test.crs()),this->property("MapIndex").toModelIndex());
    this->setProperty("crs",test.crs().toWkt());
}

void GwmCoordinate::on_BtnResPrj_clicked()
{
    this->ui->lineEditOri->setText(QString(this->property("resPrj").toString()));
}

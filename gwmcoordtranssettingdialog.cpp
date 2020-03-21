#include "gwmcoordtranssettingdialog.h"
#include "ui_gwmcoordtranssettingdialog.h"
#include <QtWidgets>

#include <QString>

#include "qgsprojectionselectionwidget.h"
#include "qgsprojectionselectiondialog.h"

GwmCoordTransSettingDialog::GwmCoordTransSettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmCoordTransSettingDialog)
{
    ui->setupUi(this);
    connect(ui->btnOK, &QPushButton::clicked, this, &GwmCoordTransSettingDialog::accept);
    connect(ui->btnCancel, &QPushButton::clicked, this, &GwmCoordTransSettingDialog::reject);
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
    return mDesCrs;
}

void GwmCoordTransSettingDialog::setDesCrs(const QgsCoordinateReferenceSystem &desCrs)
{
    mDesCrs = desCrs;
    ui->mDesCRSSelector->setCrs(desCrs);
}

QgsCoordinateReferenceSystem GwmCoordTransSettingDialog::srcCrs() const
{
    return mSrcCrs;
}

void GwmCoordTransSettingDialog::setSrcCrs(const QgsCoordinateReferenceSystem &srcCrs)
{
    mSrcCrs = srcCrs;
    ui->mSrcCRSSelector->setCrs(srcCrs);
}

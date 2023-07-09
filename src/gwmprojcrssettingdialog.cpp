#include "gwmprojcrssettingdialog.h"
#include "ui_gwmprojcrssettingdialog.h"

#include <QtWidgets>

#include <QString>

#include "qgsprojectionselectionwidget.h"
#include "qgsprojectionselectiondialog.h"

GwmProjCRSSettingDialog::GwmProjCRSSettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmProjCRSSettingDialog)
{
    ui->setupUi(this);
    connect(ui->btnok,&QPushButton::clicked, this, &GwmProjCRSSettingDialog::accept);
    connect(ui->btncancel, &QPushButton::clicked, this, &GwmProjCRSSettingDialog::reject);
}

GwmProjCRSSettingDialog::~GwmProjCRSSettingDialog()
{
    delete ui;
}

void GwmProjCRSSettingDialog::accept()
{
    return QDialog::accept();
}

void GwmProjCRSSettingDialog::reject()
{
    return QDialog::reject();
}


QgsCoordinateReferenceSystem GwmProjCRSSettingDialog::desProjCrs() const
{
    return ui->mProjCRSSelector->crs();
}

void GwmProjCRSSettingDialog::setDesProjCrs(const QgsCoordinateReferenceSystem &desProjCrs)
{
    ui->mProjCRSSelector->setCrs(desProjCrs);
}

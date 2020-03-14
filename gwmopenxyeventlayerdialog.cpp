#include "gwmopenxyeventlayerdialog.h"
#include "ui_gwmopenxyeventlayerdialog.h"
#include <QDebug>

GwmOpenXYEventLayerDialog::GwmOpenXYEventLayerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmOpenXYEventLayerDialog)
{
    ui->setupUi(this);
    connect(ui->formatCSVRadio, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::onFormatCSVRadioToggled);
    connect(ui->formatRegRadio, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::onFormatRegRadioToggled);
    connect(ui->formatCustomRadio, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::onFormatCustomRadioToggled);
    this->setAttribute(Qt::WA_DeleteOnClose);
}

GwmOpenXYEventLayerDialog::~GwmOpenXYEventLayerDialog()
{
    delete ui;
}

void GwmOpenXYEventLayerDialog::onFormatCSVRadioToggled(bool checked)
{
    qDebug() << "[GwmOpenXYEventLayerDialog::onFormatCSVRadioToggled]"
             << "checked:" << checked;
    if (checked)
    {
        ui->formatSettingStack->setCurrentIndex(0);
    }
}

void GwmOpenXYEventLayerDialog::onFormatRegRadioToggled(bool checked)
{
    qDebug() << "[GwmOpenXYEventLayerDialog::onFormatCSVRadioToggled]"
             << "checked:" << checked;
    if (checked)
    {
        ui->formatSettingStack->setCurrentIndex(1);
    }
}

void GwmOpenXYEventLayerDialog::onFormatCustomRadioToggled(bool checked)
{
    qDebug() << "[GwmOpenXYEventLayerDialog::onFormatCSVRadioToggled]"
             << "checked:" << checked;
    if (checked)
    {
        ui->formatSettingStack->setCurrentIndex(2);
    }
}

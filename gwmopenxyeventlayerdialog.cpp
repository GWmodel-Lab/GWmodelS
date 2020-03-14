#include "gwmopenxyeventlayerdialog.h"
#include "ui_gwmopenxyeventlayerdialog.h"
#include <QDebug>
#include <qgsvectordataprovider.h>

GwmOpenXYEventLayerDialog::GwmOpenXYEventLayerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmOpenXYEventLayerDialog)
{
    ui->setupUi(this);

    // 字符编码选择框
    ui->encodingCombo->addItems(QgsVectorDataProvider::availableEncodings());
    ui->encodingCombo->setCurrentIndex(ui->encodingCombo->findText(QStringLiteral("UTF-8")));

    // 文件格式配置区
    connect(ui->formatCSVRadio, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::onFormatCSVRadioToggled);
    connect(ui->formatRegRadio, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::onFormatRegRadioToggled);
    connect(ui->formatCustomRadio, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::onFormatCustomRadioToggled);

    // 设置对话框属性
    this->setAttribute(Qt::WA_DeleteOnClose);
}

GwmOpenXYEventLayerDialog::~GwmOpenXYEventLayerDialog()
{
    delete ui;
}

void GwmOpenXYEventLayerDialog::onFormatCSVRadioToggled(bool checked)
{
    if (checked)
    {
        ui->formatSettingStack->setCurrentIndex(0);
    }
}

void GwmOpenXYEventLayerDialog::onFormatRegRadioToggled(bool checked)
{
    if (checked)
    {
        ui->formatSettingStack->setCurrentIndex(1);
    }
}

void GwmOpenXYEventLayerDialog::onFormatCustomRadioToggled(bool checked)
{
    if (checked)
    {
        ui->formatSettingStack->setCurrentIndex(2);
    }
}

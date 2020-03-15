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
    ui->encodingCombo->clear();
    ui->encodingCombo->addItems(QgsVectorDataProvider::availableEncodings());
    ui->encodingCombo->setCurrentIndex(ui->encodingCombo->findText(QStringLiteral("UTF-8")));

    // 文件格式配置区
    connect(ui->formatCSVRadio, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::onFormatCSVRadioToggled);
    connect(ui->formatRegRadio, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::onFormatRegRadioToggled);
    connect(ui->formatCustomRadio, &QAbstractButton::toggled, this, &GwmOpenXYEventLayerDialog::onFormatCustomRadioToggled);

    updateFieldsAndEnable();

    connect(ui->fileNameEdit, &QLineEdit::textChanged, this, &GwmOpenXYEventLayerDialog::enableAccept);
    connect(ui->encodingCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);

    connect(ui->formatCSVRadio, &QAbstractButton::toggle, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatRegRadio, &QAbstractButton::toggle, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomRadio, &QAbstractButton::toggle, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);

    connect(ui->formatCustomDelimiterCommaRadio, &QAbstractButton::toggle, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomDelimiterSpaceRadio, &QAbstractButton::toggle, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomDelimiterTabRadio, &QAbstractButton::toggle, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomDelimiterSemicolonRadio, &QAbstractButton::toggle, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomDelimiterColonRadio, &QAbstractButton::toggle, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);

    connect(ui->formatCustomDelimiterOthersRadio, &QAbstractButton::toggle, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomDelimiterOthersEdit, &QLineEdit::textChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomEscapeEdit, &QLineEdit::textChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);
    connect(ui->formatCustomQuoteEdit, &QLineEdit::textChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable);

    connect(ui->recordNumberDiscardLinesSpin, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable );
    connect(ui->recordFirstLineAsFieldNames, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable );
    connect(ui->recordDiscardEmptyFields, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable );
    connect(ui->recordTrimFields, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable );
    connect(ui->recordDecimalSepComma, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable );

    connect(ui->geometryDMSCoord, &QCheckBox::stateChanged, this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable );
    connect(ui->geometryCRSCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmOpenXYEventLayerDialog::updateFieldsAndEnable );

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

void GwmOpenXYEventLayerDialog::updateFieldsAndEnable()
{

}

void GwmOpenXYEventLayerDialog::enableAccept()
{

}

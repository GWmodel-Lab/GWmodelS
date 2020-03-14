#include "gwmopenxyeventlayerdialog.h"
#include "ui_gwmopenxyeventlayerdialog.h"

GwmOpenXYEventLayerDialog::GwmOpenXYEventLayerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmOpenXYEventLayerDialog)
{
    ui->setupUi(this);
}

GwmOpenXYEventLayerDialog::~GwmOpenXYEventLayerDialog()
{
    delete ui;
}

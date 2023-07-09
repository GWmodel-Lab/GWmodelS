#include "gwmpropertydefaulttab.h"
#include "ui_gwmpropertydefaulttab.h"

GwmPropertyDefaultTab::GwmPropertyDefaultTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GwmPropertyDefaultTab)
{
    ui->setupUi(this);
}

GwmPropertyDefaultTab::~GwmPropertyDefaultTab()
{
    delete ui;
}

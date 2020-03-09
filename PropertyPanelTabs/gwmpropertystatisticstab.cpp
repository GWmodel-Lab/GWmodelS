#include "gwmpropertystatisticstab.h"
#include "ui_gwmpropertystatisticstab.h"

GwmPropertyStatisticsTab::GwmPropertyStatisticsTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GwmPropertyStatisticsTab)
{
    ui->setupUi(this);
}

GwmPropertyStatisticsTab::~GwmPropertyStatisticsTab()
{
    delete ui;
}

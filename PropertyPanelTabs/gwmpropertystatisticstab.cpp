#include "gwmpropertystatisticstab.h"
#include "ui_gwmpropertystatisticstab.h"

GwmPropertyStatisticsTab::GwmPropertyStatisticsTab(QWidget *parent, QStandardItemModel* model) :
    QWidget(parent),
    ui(new Ui::GwmPropertyStatisticsTab),
    propertyModel(model)
{
    ui->setupUi(this);
    ui->propertyView->setModel(propertyModel);
    ui->propertyView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    propertyModel->setHorizontalHeaderLabels(QStringList() << "Property" << "Value");
}

GwmPropertyStatisticsTab::~GwmPropertyStatisticsTab()
{
    delete ui;
}

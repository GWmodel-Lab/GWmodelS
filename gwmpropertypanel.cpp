#include "gwmpropertypanel.h"

GwmPropertyPanel::GwmPropertyPanel(QWidget *parent, QStandardItemModel* model) :
    QTabWidget(parent),
    defaultTab(new GwmPropertyDefaultTab)
{
    mapModel = model;
    addTab(defaultTab, "Default");
}

GwmPropertyPanel::~GwmPropertyPanel()
{
}

void GwmPropertyPanel::setupTabs()
{

}

void GwmPropertyPanel::addStatisticTab(QModelIndex index)
{
    QStandardItem* item = mapModel->itemFromIndex(index);
}

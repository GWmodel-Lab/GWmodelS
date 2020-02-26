#include <QtWidgets>
#include "mainwidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , toolBar(new GWmodelToolbar)
{
    createToolBar();
    createMap();
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(map);
    setLayout(mainLayout);
//    mainLayout->setStretchFactor(toolBar, 1);
    mainLayout->setStretchFactor(map, 1);
}

MainWidget::~MainWidget()
{

}

void MainWidget::createToolBar()
{

}

void MainWidget::createMap()
{
    map = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(map);
    layout->addWidget(new QLabel(tr("Map")));
    map->setLayout(layout);
}

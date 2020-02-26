#include <QtWidgets>
#include "mainwidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
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
    toolBar = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(toolBar);
    layout->setSpacing(1);
    layout->addWidget(new QPushButton(tr("Open")));
    layout->addWidget(new QPushButton(tr("Save")));
    layout->addWidget(new QPushButton(tr("Export")));
    layout->addWidget(new QPushButton(tr("Edit")));
    layout->addWidget(new QPushButton(tr("Move")));
    layout->addWidget(new QPushButton(tr("Full")));
    layout->addWidget(new QPushButton(tr("Position")));
    layout->addWidget(new QPushButton(tr("GWSS")));
    layout->addWidget(new QPushButton(tr("GWR")));
    layout->addWidget(new QPushButton(tr("GWPCA")));
    layout->addWidget(new QPushButton(tr("Map")));
    layout->addWidget(new QPushButton(tr("Atlas")));
    layout->addStretch();
    layout->setMargin(0);
}

void MainWidget::createMap()
{
    map = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(map);
    layout->addWidget(new QLabel(tr("Map")));
    map->setLayout(layout);
}

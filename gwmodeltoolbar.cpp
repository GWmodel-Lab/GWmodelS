#include "gwmodeltoolbar.h"

GWmodelToolbar::GWmodelToolbar(QWidget *parent) :
    QWidget(parent)
{
    createButtons();
    QHBoxLayout* widgetLayout = new QHBoxLayout(this);
    widgetLayout->addWidget(openLayerBtn);
    widgetLayout->addWidget(saveLayerBtn);
    widgetLayout->addWidget(exportLayerBtn);
    widgetLayout->addWidget(editBtn);
    widgetLayout->addWidget(moveBtn);
    widgetLayout->addWidget(fullScreenBtn);
    widgetLayout->addWidget(showPositionBtn);
    widgetLayout->addWidget(gwmodelGWRBtn);
    widgetLayout->addWidget(gwmodelGWSSBtn);
    widgetLayout->addWidget(gwmodelGWPCABtn);
    this->setLayout(widgetLayout);

    connect(openLayerBtn,&QPushButton::clicked,this,&GWmodelToolbar::openFileImportShapefile);
    connect(saveLayerBtn,&QPushButton::clicked,this,&GWmodelToolbar::openFileImportJson);
    connect(exportLayerBtn,&QPushButton::clicked,this,&GWmodelToolbar::openFileImportCsv);
}

GWmodelToolbar::~GWmodelToolbar()
{

}

void GWmodelToolbar::openFileImportShapefile(){
    emit openFileImportShapefileSignal();
}

void GWmodelToolbar::openFileImportJson(){
    emit openFileImportJsonSignal();
}

void GWmodelToolbar::openFileImportCsv(){
    emit openFileImportCsvSignal();
}

void GWmodelToolbar::createButtons()
{
    openLayerBtn = new QPushButton(tr("Open"));
    saveLayerBtn = new QPushButton(tr("Save"));
    exportLayerBtn = new QPushButton(tr("Export"));
    editBtn = new QPushButton(tr("Edit"));
    moveBtn = new QPushButton(tr("Move"));
    fullScreenBtn = new QPushButton(tr("Full"));
    showPositionBtn = new QPushButton(tr("Pos"));
    gwmodelGWRBtn = new QPushButton(tr("GWR"));
    gwmodelGWSSBtn = new QPushButton(tr("GWSS"));
    gwmodelGWPCABtn = new QPushButton(tr("GWPCA"));
}

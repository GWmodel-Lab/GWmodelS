#include "gwmtoolbar.h"
#include "ui_gwmtoolbar.h"

GwmToolbar::GwmToolbar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GwmToolbar)
{
    ui->setupUi(this);
    connect(ui->openLayerBtn, &QPushButton::clicked, this, &GwmToolbar::openFileImportShapefileSignal);
    connect(ui->saveLayerBtn, &QPushButton::clicked, this, &GwmToolbar::openFileImportJsonSignal);
    connect(ui->saveAsBtn, &QPushButton::clicked, this, &GwmToolbar::openFileImportCsvSignal);
    connect(ui->openByXYBtn, &QPushButton::clicked, this, &GwmToolbar::openByXYBtnSingnal);
    connect(ui->selectBtn,  &QPushButton::clicked,  this , &GwmToolbar::selectBtnSignal);
    connect(ui->moveBtn, &QPushButton::clicked, this, &GwmToolbar::moveBtnSignal);
    connect(ui->editBtn, &QPushButton::clicked, this, &GwmToolbar::editBtnSignal);
    connect(ui->zoomFullBtn, &QPushButton::clicked, this, &GwmToolbar::zoomFullBtnSignal);
    connect(ui->zoomToLayerBtn, &QPushButton::clicked, this, &GwmToolbar::zoomToLayerBtnSignal);
    connect(ui->zoomToSelectionBtn, &QPushButton::clicked, this, &GwmToolbar::zoomToSelectionBtnSignal);
    connect(ui->gwmodelGWRBtn, &QPushButton::clicked, this, &GwmToolbar::gwmodelGWRBtnSignal);
    connect(ui->gwmodelGWSSBtn, &QPushButton::clicked, this, &GwmToolbar::gwmodelGWSSBtnSignal);
    connect(ui->gwmodelGWPCABtn, &QPushButton::clicked, this, &GwmToolbar::gwmodelGWPCABtnSignal);
}

GwmToolbar::~GwmToolbar()
{
    delete ui;
}

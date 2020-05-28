#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMenuBar>
#include <QAction>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mainWidget(new MainWidget())
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupMenus();
    setAttribute(Qt::WA_QuitOnClose);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMenus()
{
    connect(ui->action_ESRI_Shapefile, &QAction::triggered, mainWidget, &MainWidget::openFileImportShapefile);
    connect(ui->actionGeo_Json, &QAction::triggered, mainWidget, &MainWidget::openFileImportJson);
    connect(ui->action_CSV, &QAction::triggered, mainWidget, &MainWidget::openFileImportCsv);
    connect(ui->action_CsvToDat, &QAction::triggered, mainWidget, &MainWidget::onCsvToDat);
    //鲁棒GWR
    connect(ui->actionRobustGWR,&QAction::triggered,mainWidget,&MainWidget::onRobustGWR);
}


void MainWindow::on_actionScalable_GWR_triggered()
{
    mainWidget->onScalableGWRBtnClicked();
}

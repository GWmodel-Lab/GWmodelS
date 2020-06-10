#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMenuBar>
#include <QAction>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
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
    connect(ui->action_ESRI_Shapefile, &QAction::triggered, ui->centralwidget, &MainWidget::openFileImportShapefile);
    connect(ui->actionGeo_Json, &QAction::triggered, ui->centralwidget, &MainWidget::openFileImportJson);
    connect(ui->action_CSV, &QAction::triggered, ui->centralwidget, &MainWidget::openFileImportCsv);
    connect(ui->action_CsvToDat, &QAction::triggered, ui->centralwidget, &MainWidget::onCsvToDat);
    //鲁棒GWR
    connect(ui->actionRobustGWR,&QAction::triggered,ui->centralwidget,&MainWidget::onRobustGWR);
    connect(ui->actionScalable_GWR, &QAction::triggered,ui->centralwidget,&MainWidget::onScalableGWRBtnClicked);
    connect(ui->action_GGWR,&QAction::triggered, ui->centralwidget, &MainWidget::onGGWRBtnClicked);
    connect(ui->actionLocal_collinearity_GWR,&QAction::triggered, ui->centralwidget, &MainWidget::onLcrGWRBtnClicked);
}

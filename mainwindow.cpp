#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMenuBar>
#include <QAction>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mainWidget(new MainWidget())
//    , ui(new Ui::MainWindow)
{
    setMinimumWidth(1280);
    setMinimumHeight(720);
    createMenus();
    setCentralWidget(mainWidget);
}

MainWindow::~MainWindow()
{
//    delete ui;
}

void MainWindow::createMenus()
{
    // Menu File
    QMenu* menuFile = menuBar()->addMenu(tr("&File"));
    // Menu File Open
    QAction* menuFileOpen = new QAction(tr("&Open Project"));
    menuFile->addAction(menuFileOpen);
    // Menu File Save
    QAction* menuFileSave = new QAction(tr("&Save Project"));
    menuFile->addAction(menuFileSave);
    // Menu File ImportLayer
    QMenu* menuFileImport = menuFile->addMenu(tr("&Import Layer"));
    // Menu FIle ImportLayer Shpfile
    QAction* actFileImportShp = new QAction(tr("ESRI &Shapefile"));
    connect(actFileImportShp, &QAction::triggered, this, &MainWindow::openFileImportShapefile);
    menuFileImport->addAction(actFileImportShp);
    // Menu File ImportLayer JSON
    QAction* actFileImportJson = new QAction(tr("Geo&Json"));
    connect(actFileImportJson, &QAction::triggered, this, &MainWindow::openFileImportJson);
    menuFileImport->addAction(actFileImportJson);
    // Menu File ImportLayer CSV
    QAction* actFileImportCsv = new QAction(tr("&CSV"));
    connect(actFileImportCsv, &QAction::triggered, this, &MainWindow::openFileImportCsv);
    menuFileImport->addSeparator();
    menuFileImport->addAction(actFileImportCsv);
    // Menu File Exit
    QAction* menuFileExit = new QAction(tr("&Exit"));
    menuFile->addSeparator();
    menuFile->addAction(menuFileExit);

    connect(mainWidget,&MainWidget::openFileImportShapefileSignal,this, &MainWindow::openFileImportShapefile);
    connect(mainWidget,&MainWidget::openFileImportJsonSignal,this, &MainWindow::openFileImportJson);
    connect(mainWidget,&MainWidget::openFileImportCsvSignal,this, &MainWindow::openFileImportCsv);

    // Show Menu Bar
    menuBar()->show();
}

void MainWindow::openFileImportShapefile()
{
    QFileDialog::getOpenFileName(this, tr("Open ESRI Shapefile"), tr(""), tr("ESRI Shapefile (*.shp)"));
}

void MainWindow::openFileImportJson()
{
    QFileDialog::getOpenFileName(this, tr("Open GeoJson"), tr(""), tr("GeoJson (*.json *.geojson)"));
}

void MainWindow::openFileImportCsv()
{
    QFileDialog::getOpenFileName(this, tr("Open CSV"), tr(""), tr("CSV (*.csv)"));
}


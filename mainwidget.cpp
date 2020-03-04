#include <QtWidgets>
#include <QFileInfo>
#include <qgsvectorlayer.h>
#include "mainwidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , mainLayout(new QVBoxLayout)
    , toolBar(new GWmodelToolbar)
{
    mapModel = new QStandardItemModel();

    createMainZone();
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(mainZone);
    setLayout(mainLayout);
    mainLayout->setStretchFactor(mainZone, 1);

    connect(toolBar, &GWmodelToolbar::openFileImportShapefileSignal, this, &MainWidget::openFileImportShapefile);
    connect(toolBar, &GWmodelToolbar::openFileImportJsonSignal, this, &MainWidget::openFileImportJson);
    connect(toolBar, &GWmodelToolbar::openFileImportCsvSignal, this, &MainWidget::openFileImportCsv);
}

MainWidget::~MainWidget()
{

}

void MainWidget::openFileImportShapefile(){
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open ESRI Shapefile"), tr(""), tr("ESRI Shapefile (*.shp)"));
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists())
    {
        QString fileName = fileInfo.baseName();
        QStandardItem* item = new QStandardItem(fileName);
        QMap<QString, QVariant> itemData;
        itemData["path"] = QVariant(filePath);
        item->setData(QVariant(itemData));
        item->setCheckable(true);
        item->setCheckState(Qt::CheckState::Checked);
        mapModel->appendRow(item);
    }
}

void MainWidget::openFileImportJson(){
    QFileDialog::getOpenFileName(this, tr("Open GeoJson"), tr(""), tr("GeoJson (*.json *.geojson)"));
}

void MainWidget::openFileImportCsv(){
    QFileDialog::getOpenFileName(this, tr("Open CSV"), tr(""), tr("CSV (*.csv)"));
}

void MainWidget::createMainZone()
{
    mainZone = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(mainZone);

    createFeaturePanel();
    createPropertyPanel();
    mapPanel = new GWmodelMapPanel(mainZone, mapModel);

    layout->addWidget(featurePanel);
    layout->addWidget(mapPanel);
    layout->addWidget(propertyPanel);

    layout->setStretchFactor(mapPanel, 1);
}

void MainWidget::createFeaturePanel()
{
    featurePanel = new QTreeView(mainZone);
    featurePanel->setColumnWidth(0, 320);
    QStringList headerLabels = QStringList() << tr("Features");
    mapModel->setHorizontalHeaderLabels(headerLabels);
    featurePanel->setModel(mapModel);
}

void MainWidget::createPropertyPanel()
{
    propertyPanel = new QTabWidget(mainZone);
    propertyPanel->setFixedWidth(420);
    // Demo Tab
    QLabel* demoTab = new QLabel(tr("Select a feature to show its property."), propertyPanel);
    propertyPanel->addTab(demoTab, tr("Property"));
    // [End] Demo Tab
}

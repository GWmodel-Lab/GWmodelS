#include <QtWidgets>
#include <qgsvectorlayer.h>
#include "mainwidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , toolBar(new GWmodelToolbar)
{
    createToolBar();
    createMainZone();
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(mainZone);
    setLayout(mainLayout);
//    mainLayout->setStretchFactor(toolBar, 1);
    mainLayout->setStretchFactor(mainZone, 1);
}

MainWidget::~MainWidget()
{

}

void MainWidget::createToolBar()
{

}

void MainWidget::createMainZone()
{
    mainZone = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(mainZone);

    createFeaturePanel();
    layout->addWidget(featurePanel);

    createMapPanel();
    layout->addWidget(mapPanel);

    createPropertyPanel();
    layout->addWidget(propertyPanel);

    layout->setStretchFactor(mapPanel, 1);
}

void MainWidget::createMapPanel()
{
    mapPanel = new QgsMapCanvas(mainZone);
    // Demo Layer
    QString demoLayerPath = tr("C:/Users/HuYG0/Documents/Build/GWmodel Desktop/Data/road/路网.shp");
    QgsVectorLayer* demoLayer = new QgsVectorLayer(demoLayerPath);
    if (!demoLayer->isValid()) {
        QMessageBox::information(this, tr("Error"), tr("Layer is not valid!"));
    }
    QList<QgsMapLayer*> layers;
    layers.append(demoLayer);
    mapPanel->setExtent(demoLayer->extent());
    mapPanel->enableAntiAliasing(true);
    mapPanel->setVisible(true);
    mapPanel->setLayers(layers);
    mapPanel->refresh();
    // [End] Demo Layer
}

void MainWidget::createFeaturePanel()
{
    featurePanel = new QTreeView(mainZone);
    featurePanel->setColumnWidth(0, 320);

    // Demo Model
    QStandardItemModel* model = new QStandardItemModel(featurePanel);
    model->setHorizontalHeaderLabels(QStringList() << tr("Features"));
    QStandardItem* itemFeature = new QStandardItem(tr("road"));
    itemFeature->setCheckable(true);
    itemFeature->setAutoTristate(true);
    model->appendRow(itemFeature);
    QStandardItem* itemChild = new QStandardItem(tr("Origin"));
    itemChild->setCheckable(true);
    itemFeature->appendRow(itemChild);
    // [End] Demo Model
    featurePanel->setModel(model);
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

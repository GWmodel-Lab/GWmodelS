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

    createMapPanel();
    layout->addWidget(mapPanel);



    // FeatureZone
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

#ifndef MAINLAYOUT_H
#define MAINLAYOUT_H

#include <QWidget>
#include <gwmodeltoolbar.h>
#include <qgsmapcanvas.h>

//namespace Ui {
//class MainLayout;
//}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();


signals:
    void openFileImportShapefileSignal();
    void openFileImportJsonSignal();
    void openFileImportCsvSignal();


private:
    GWmodelToolbar* toolBar;
    QWidget* mainZone;
    QgsMapCanvas* mapPanel;
    QTreeView* featurePanel;
    QTabWidget* propertyPanel;


    void openFileImportShapefile();
    void openFileImportJson();
    void openFileImportCsv();
    void createToolBar();
    void createMainZone();
    void createMapPanel();
    void createFeaturePanel();
    void createPropertyPanel();
};

#endif // MAINLAYOUT_H

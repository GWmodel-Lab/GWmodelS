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

private:
    GWmodelToolbar* toolBar;
    QWidget* mainZone;
    QgsMapCanvas* mapPanel;
    QTreeView* featurePanel;
    QTabWidget* propertyPanel;

    void createToolBar();
    void createMainZone();
    void createMapPanel();
    void createFeaturePanel();
    void createPropertyPanel();
};

#endif // MAINLAYOUT_H

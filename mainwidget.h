#ifndef MAINLAYOUT_H
#define MAINLAYOUT_H

#include <QWidget>
#include <gwmodeltoolbar.h>
#include <gwmodelmappanel.h>

//namespace Ui {
//class MainLayout;
//}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();


public:
    QVBoxLayout* mainLayout;
    GWmodelToolbar* toolBar;
    QWidget* mainZone;
    GWmodelMapPanel* mapPanel;
    QTreeView* featurePanel;
    QTabWidget* propertyPanel;

    QStandardItemModel* mapModel;

public slots:
    void openFileImportShapefile();
    void openFileImportJson();
    void openFileImportCsv();

private:
    void createMainZone();
    void createFeaturePanel();
    void createPropertyPanel();
};

#endif // MAINLAYOUT_H

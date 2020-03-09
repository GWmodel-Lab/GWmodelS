#ifndef MAINLAYOUT_H
#define MAINLAYOUT_H

#include <QWidget>
#include <gwmtoolbar.h>
#include <gwmfeaturepanel.h>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>

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
    QWidget* mainZone;
    GwmToolbar* toolBar;
    GwmFeaturePanel* featurePanel;
    QTabWidget* propertyPanel;

    QStandardItemModel* mapModel;
    QList<QgsMapLayer*> mapLayerSet;
    QgsMapCanvas* mapCanvas;

public slots:
    void openFileImportShapefile();
    void openFileImportJson();
    void openFileImportCsv();

private:
    void createMainZone();
    void createFeaturePanel();
    void createPropertyPanel();
    void createMapPanel();
    /**
     * @brief Map item inserted slot.
     */
    void onMapItemInserted(const QModelIndex &parent, int first, int last);
};

#endif // MAINLAYOUT_H

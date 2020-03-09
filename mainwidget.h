#ifndef MAINLAYOUT_H
#define MAINLAYOUT_H

#include <QWidget>
#include <gwmtoolbar.h>
#include <gwmfeaturepanel.h>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <gwmpropertypanel.h>

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
    GwmToolbar* toolbar;
    GwmFeaturePanel* featurePanel;
    GwmPropertyPanel* propertyPanel;

    QStandardItemModel* mapModel;
    QList<QgsMapLayer*> mapLayerSet;
    QgsMapCanvas* mapCanvas;
    QMap<QString, QgsVectorLayer*> mapLayerNameDict;

public slots:
    void openFileImportShapefile();
    void openFileImportJson();
    void openFileImportCsv();
    void onShowLayerProperty(const QModelIndex &index);

private:
    void createToolbar();
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

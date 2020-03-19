#ifndef MAINLAYOUT_H
#define MAINLAYOUT_H

#include <QWidget>
#include <gwmtoolbar.h>
#include <gwmfeaturepanel.h>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>

#include <gwmpropertypanel.h>
#include <qgsmaptoolpan.h>
#include <gwmmaptoolidentifyfeature.h>

#include <qgssymbolselectordialog.h>

#include "symbolwindow/gwmsymbolwindow.h"


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

    bool isFeaturePanelDragging;

    GwmLayerItemModel* mapModel;
    QList<QgsMapLayer*> mapLayerList;
    QgsMapCanvas* mapCanvas;
    QMap<QString, QgsVectorLayer*> mapLayerIdDict;
    QgsMapTool* mapPanTool;
    QgsMapTool* mapIdentifyTool;
    QPoint mapPoint0;
    QMap<QgsVectorLayer*, QList<QgsRubberBand*>> mapLayerRubberDict;



    GwmSymbolWindow* symbolWindow;

public slots:
    void openFileImportShapefile();
    void openFileImportJson();
    void openFileImportCsv();
    void symbolSlot(const QModelIndex &index);

private:
    void createToolbar();
    void createMainZone();
    void createFeaturePanel();
    void createPropertyPanel();
    void createMapPanel();

    void addLayerToModel(const QString &uri, const QString &layerName, const QString &providerKey = QString("ogr"));

    void createSymbolWindow(const QModelIndex &index);
    // 要素区属性表窗口
    void onShowAttributeTable(const QModelIndex &index);
    void onAttributeTableSelected(QgsVectorLayer* layer, QList<QgsFeatureId> list);
    void onFullScreen();

private slots:
    void onMapSelectionChanged(QgsVectorLayer* layer);

    /**
     * @brief 当模型项发生改变时触发的槽
     * @param item 改变的项
     */
    void onMapModelChanged();

    void onShowLayerProperty(const QModelIndex& index);

    void onFeaturePanelRowOrderChanged(int from, int dest);

    void onFeaturePanelBeginDragDrop();

    void onFeaturePanelEndDragDrop();

    /**
     * @brief 移除图层的槽函数
     * @param index 项的索引
     */
    void onZoomToLayer(const QModelIndex &index);
    void onSelectMode();
    void onNavigateMode();
    void onEditMode();

    void refreshCanvas();
};

#endif // MAINLAYOUT_H

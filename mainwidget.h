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

#include <gwmcoordtranssettingdialog.h>
#include <gwmgwroptionsdialog.h>

namespace Ui {
    class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();


private:
    Ui::MainWidget* ui;
    GwmToolbar* toolbar;
    GwmFeaturePanel* featurePanel;
    GwmPropertyPanel* propertyPanel;

    GwmLayerItemModel* mapModel;
    QgsMapCanvas* mapCanvas;
    QList<QgsMapLayer*> mapLayerList;
    QgsMapTool* mapPanTool;
    QgsMapTool* mapIdentifyTool;
    QMap<QgsVectorLayer*, QList<QgsRubberBand*>> mapLayerRubberDict;



    GwmSymbolWindow* symbolWindow;

public slots:
    void openFileImportShapefile();
    void openFileImportJson();
    void openFileImportCsv();
    void onShowSymbolSetting(const QModelIndex &index);

public:
    bool eventFilter(QObject* obj, QEvent* e);

private:
    void setupToolbar();
    void setupFeaturePanel();
    void setupPropertyPanel();
    void setupMapPanel();

    void addLayerToModel(QgsVectorLayer* layer);
    void createLayerToModel(const QString &uri, const QString &layerName, const QString &providerKey = QString("ogr"));

    void createSymbolWindow(const QModelIndex &index);
    // 要素区属性表窗口
    void onShowAttributeTable(const QModelIndex &index);
    void onAttributeTableSelected(QgsVectorLayer* layer, QList<QgsFeatureId> list);
    void onFullScreen();

    /**
     * @brief 从模型中导出地图所需要显示的图层
     */
    void deriveLayersFromModel();

    bool askUserForDatumTransfrom(const QgsCoordinateReferenceSystem& sourceCrs, const QgsCoordinateReferenceSystem& destinationCrs, const QgsMapLayer* layer = nullptr);

public slots:
    void onMapSelectionChanged(QgsVectorLayer* layer);

    /**
     * @brief 当模型项发生改变时触发的槽
     * @param item 改变的项
     */
    void onMapModelChanged();

    void onShowLayerProperty(const QModelIndex& index);

    void onFeaturePanelRowOrderChanged(int from, int dest);

    /**
     * @brief 移除图层的槽函数
     * @param index 项的索引
     */
    void onZoomToLayer(const QModelIndex &index);
    void onSelectMode();
    void onNavigateMode();
    void onEditMode();
    void onSaveLayer();
    void onExportLayerAsShpfile();
    void onExportLayerAsCsv(const QModelIndex &index);
    void onExportLayer(QString filetype);
    void onZoomToSelection();
    void onZoomToLayerBtn();

    void onFeaturePanelCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

    void refreshCanvas();
    void onShowCoordinateTransDlg(const QModelIndex &index);
    void onGWRBtnClicked();

   // void transformCoordinate(const QgsCoordinateReferenceSystem des, const QModelIndex& index);

    //void setNewCoordinate(QgsCoordinateReferenceSystem,QString,QModelIndex);
};

#endif // MAINLAYOUT_H

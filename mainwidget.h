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
#include <gwmcsvtodatdialog.h>

#include "gwmrobustgwroptionsdialog.h"
#include "gwmlcrgwroptionsdialog.h"
#include "gwmgwpcaoptionsdialog.h"

namespace Ui {
    class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

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
    void openFileImportShapefile();
    void openFileImportJson();
    void openFileImportCsv();
    void onShowSymbolSetting(const QModelIndex &index);
    void onCsvToDat();
    void onRobustGWR();
    void onGGWRBtnClicked();
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
    void onGWSSBtnClicked();
    void onGWRNewBtnClicked();
    void onScalableGWRBtnClicked();
    void onMultiscaleGWRBtnClicked();

    void onRobustGWRBtnClicked();

    void onLcrGWRBtnClicked();

    void onGWPCABtnClicked();


private:
    Ui::MainWidget* ui;
    GwmToolbar* mToolbar;
    GwmFeaturePanel* mFeaturePanel;
    GwmPropertyPanel* mPropertyPanel;

    GwmLayerItemModel* mMapModel;
    QgsMapCanvas* mMapCanvas;
    QList<QgsMapLayer*> mMapLayerList;
    QgsMapTool* mMapPanTool;
    QgsMapTool* mMapIdentifyTool;
    QMap<QgsVectorLayer*, QList<QgsRubberBand*>> mMapLayerRubberDict;

    GwmSymbolWindow* mSymbolWindow;
};

#endif // MAINLAYOUT_H

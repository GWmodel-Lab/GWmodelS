#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mainwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
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
    void setupMenus();
    void toggleToolbarGeneral(bool flag);

private:
    Ui::MainWindow *ui;
    GwmFeaturePanel* mFeaturePanel;
    GwmPropertyPanel* mPropertyPanel;

    GwmLayerItemModel* mMapModel;
    QgsMapCanvas* mMapCanvas;
    QList<QgsMapLayer*> mMapLayerList;
    QgsMapTool* mMapPanTool;
    QgsMapTool* mMapIdentifyTool;
    QMap<QgsVectorLayer*, QList<QgsRubberBand*>> mMapLayerRubberDict;

    GwmSymbolWindow* mSymbolWindow;

//    MainWidget* mainWidget;
};
#endif // MAINWINDOW_H

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

    QStandardItemModel* mapModel;
    QList<QgsMapLayer*> mapLayerList;
    QgsMapCanvas* mapCanvas;
    QMap<QString, QgsVectorLayer*> mapLayerIdDict;
    QgsMapTool* mapPanTool;
    QgsMapTool* mapIdentifyTool;
    QPoint mapPoint0;
    QMap<QgsVectorLayer*, QList<QgsRubberBand*>> mapLayerRubberDict;



public slots:
    void openFileImportShapefile();
    void openFileImportJson();
    void openFileImportCsv();
    // 要素区属性表窗口
    void receiveAttributeTable(const QModelIndex &index);
    void receiveSigAttriToMap(QList<QgsFeatureId> list);

    /**
     * @brief 移除图层的槽函数
     * @param index 项的索引
     */
    void onRemoveLayer(const QModelIndex &index);
    void onShowLayerProperty(const QModelIndex &index);
    void onSelectMode();
    void onNavigateMode();
    void onEditMode();

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
    void onFullScreen();

    /**
     * @brief 从模型中导出地图所需要显示的图层
     */
    void deriveLayersFromModel();

private slots:
    void onMapSelectionChanged(QgsVectorLayer* layer);

    /**
     * @brief 当模型项发生改变时触发的槽
     * @param item 改变的项
     */
    void onMapModelItemChanged(QStandardItem* item);

    void onFeaturePanelRowOrderChanged(int from, int dest);

    void onFeaturePanelBeginDragDrop();

    void onFeaturePanelEndDragDrop();
};

#endif // MAINLAYOUT_H

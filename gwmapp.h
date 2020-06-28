#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgsmaptoolpan.h>
#include <qgssymbolselectordialog.h>
#include <qgslayoutcustomdrophandler.h>
#include <qgsapplication.h>
#include <qgsruntimeprofiler.h>
#include <qgsmasterlayoutinterface.h>

#include "gwmtoolbar.h"
#include "gwmfeaturepanel.h"
#include "gwmmaptoolidentifyfeature.h"
#include "gwmpropertypanel.h"
#include "Layout/gwmlayoutdesigner.h"

#include "symbolwindow/gwmsymbolwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class GwmApp : public QMainWindow
{
    Q_OBJECT

public:		// 静态
	static GwmApp* Instance();

private:
	static GwmApp* mInstance;


public:
    GwmApp(QWidget *parent = nullptr);
    ~GwmApp();

    GwmLayerItemModel *mapModel() const;

    QgsMapCanvas *mapCanvas() const;

    QList<QgsMapLayer *> mapLayerList() const;

public:
    bool eventFilter(QObject* obj, QEvent* e);

private:
    void setupToolbar();
	void setupFeaturePanel();
    void setupPropertyPanel();
    void setupMapPanel();
	void initLayouts();

	void startProfile(const QString &name)
	{
		QgsApplication::profiler()->start(name);
	}

	void endProfile()
	{
		QgsApplication::profiler()->end();
	}

	void functionProfile(void (GwmApp::*fnc)(), GwmApp *instance, const QString &name)
	{
		startProfile(name);
		(instance->*fnc)();
		endProfile();
	}


private:
	GwmLayoutDesigner* createPrintLayout(const QString& t);
	GwmLayoutDesigner* openLayoutDesignerDialog(QgsMasterLayoutInterface* layout);

	void registerCustomLayoutDropHandler(QgsLayoutCustomDropHandler *handler)
	{
		if (!mCustomLayoutDropHandlers.contains(handler))
			mCustomLayoutDropHandlers << handler;
	}

	void unregisterCustomLayoutDropHandler(QgsLayoutCustomDropHandler *handler)
	{
		mCustomLayoutDropHandlers.removeOne(handler);
	}

private:

    void addLayerToModel(QgsVectorLayer* layer);
    void createLayerToModel(const QString &uri, const QString &layerName, const QString &providerKey = QString("ogr"));

    void createSymbolWindow(const QModelIndex &index);
    // 要素区属性表窗口
    void onShowAttributeTable(const QModelIndex &index);
    void onAttributeTableSelected(QgsVectorLayer* layer, QList<QgsFeatureId> list);

    bool askUserForDatumTransfrom(const QgsCoordinateReferenceSystem& sourceCrs, const QgsCoordinateReferenceSystem& destinationCrs, const QgsMapLayer* layer = nullptr);


public slots:
    void onOpenFileImportShapefile();
    void onOpenFileImportJson();
    void onOpenFileImportCsv();
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
    void onEditMode();
    void onSaveLayer();
    void onExportLayerAsCsv(const QModelIndex &index);
    void onExportLayer(QString filetype);
    void onZoomToSelection();
    void onZoomToLayerBtn();

    void onFeaturePanelCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

    void onShowCoordinateTransDlg(const QModelIndex &index);
    void onGWRBtnClicked();
    void onGWSSBtnClicked();
    void onGWRNewBtnClicked();
    void onScalableGWRBtnClicked();
    void onMultiscaleGWRBtnClicked();

    void onRobustGWRBtnClicked();

    void onLcrGWRBtnClicked();

    void onGWPCABtnClicked();

	void populateLayoutsMenu(QMenu *menu);

	
private:
    void setupMenus();
    void toggleToolbarGeneral(bool flag);

private:
    Ui::MainWindow *ui;

    GwmFeaturePanel* mFeaturePanel;
    GwmPropertyPanel* mPropertyPanel;

	QVector<QPointer<QgsLayoutCustomDropHandler>> mCustomLayoutDropHandlers;
	QgsLayoutCustomDropHandler *mLayoutQptDropHandler = nullptr;
	QgsLayoutCustomDropHandler *mLayoutImageDropHandler = nullptr;

    GwmLayerItemModel* mMapModel;
    QgsMapCanvas* mMapCanvas;
    QList<QgsMapLayer*> mMapLayerList;
    QgsMapTool* mMapPanTool;
    QgsMapTool* mMapIdentifyTool;
    QMap<QgsVectorLayer*, QList<QgsRubberBand*>> mMapLayerRubberDict;

    GwmSymbolWindow* mSymbolWindow;

//    MainWidget* mainWidget;

};


inline QList<QgsMapLayer *> GwmApp::mapLayerList() const
{
    return mMapLayerList;
}

inline QgsMapCanvas *GwmApp::mapCanvas() const
{
    return mMapCanvas;
}

inline GwmLayerItemModel *GwmApp::mapModel() const
{
    return mMapModel;
}

#endif // MAINWINDOW_H

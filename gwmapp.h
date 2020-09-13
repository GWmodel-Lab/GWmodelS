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
#include "gwmproject.h"

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


public:
	GwmLayoutDesigner* createPrintLayout(const QString& t);
	GwmLayoutDesigner* createNewReport(QString title);
	GwmLayoutDesigner* openLayoutDesignerDialog(QgsMasterLayoutInterface* layout);
	GwmLayoutDesigner* duplicateLayout(QgsMasterLayoutInterface *layout, const QString &t = QString());
	void showLayoutManager();
	bool uniqueLayoutTitle(QWidget *parent, QString &title, bool acceptEmpty, QgsMasterLayoutInterface::Type type, const QString &currentTitle = QString());
	void registerCustomLayoutDropHandler(QgsLayoutCustomDropHandler *handler);
	void unregisterCustomLayoutDropHandler(QgsLayoutCustomDropHandler *handler);
    void onSaveProject();
    void onOpenProject();
    void addWindow( QAction *action );
    void removeWindow( QAction *action );


private:
    void addLayerToModel(QgsVectorLayer* layer);
    void createLayerToModel(const QString &uri, const QString &layerName, const QString &providerKey = QString("ogr"));
    void createSymbolWindow(const QModelIndex &index);
    /**
     * @brief 显示要素区属性表
     * @param index 图层项索引
     */
    void onShowAttributeTable(const QModelIndex &index);
    void onAttributeTableSelected(QgsVectorLayer* layer, QList<QgsFeatureId> list);
    bool askUserForDatumTransfrom(const QgsCoordinateReferenceSystem& sourceCrs, const QgsCoordinateReferenceSystem& destinationCrs, const QgsMapLayer* layer = nullptr);
    void updateWindowTitle();


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
    void onGTWRBtnClicked();
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

#ifdef Q_OS_MAC
    QAction *mActionWindowMinimize = nullptr;
    QAction *mActionWindowZoom = nullptr;
    QAction *mActionWindowSeparator1 = nullptr;
    QAction *mActionWindowAllToFront = nullptr;
    QAction *mActionWindowSeparator2 = nullptr;
    QActionGroup *mWindowActions = nullptr;
#endif

#ifdef Q_OS_MAC
    QMenu *mWindowMenu = nullptr;
#endif

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

inline void GwmApp::registerCustomLayoutDropHandler(QgsLayoutCustomDropHandler * handler)
{
	if (!mCustomLayoutDropHandlers.contains(handler))
		mCustomLayoutDropHandlers << handler;
}

inline void GwmApp::unregisterCustomLayoutDropHandler(QgsLayoutCustomDropHandler * handler)
{
    mCustomLayoutDropHandlers.removeOne(handler);
}



#endif // MAINWINDOW_H

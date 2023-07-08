#ifndef GWMLAYOUTDESIGNER_H
#define GWMLAYOUTDESIGNER_H


#include <QMainWindow>
#include "ui_gwmlayoutdesigner.h"

#include <QUndoView>
#include <QSlider>
#include <QLabel>
#include <QProgressBar>
#include <QPrinter>
#include <QToolButton>
#include <QComboBox>

#include <qgis.h>
#include <qgsmasterlayoutinterface.h>
#include <qgslayoutdesignerinterface.h>
#include <qgslayoutitempage.h>
#include <qgslayoutexporter.h>

class QgsLayout;
class QgsLayoutItem;
class QgsLayoutView;
class QgsLayoutRuler;
class QgsLayoutViewToolAddItem;
class QgsLayoutViewToolAddNodeItem;
class QgsLayoutViewToolPan;
class QgsLayoutViewToolZoom;
class QgsLayoutViewToolSelect;
class QgsLayoutViewToolEditNodes;
class QgsLayoutViewToolMoveItemContent;
class QgsLayoutItemsListView;
class QgsDockWidget;
class QgsPanelWidgetStack;
class QgsLayoutDesignerInterface;
class QgsLayoutPropertiesWidget;
class QgsLayoutGuideWidget;
class QgsMessageBar;
class QgsLayoutAtlas;
class GwmLayoutDesignerInterface;


class GwmLayoutDesigner : public QMainWindow, public Ui::GwmLayoutDesigner
{
	Q_OBJECT

private:
	static bool sInitializedRegistry;

	struct PanelStatus
	{
		PanelStatus(bool visible = true, bool active = false)
			: isVisible(visible)
			, isActive(active)
		{
		}
		bool isVisible;
		bool isActive;
	};

public:
	GwmLayoutDesigner(QWidget *parent = nullptr);
	~GwmLayoutDesigner();

	GwmLayoutDesignerInterface* iface();

	void setTitle(const QString &title);

	QgsLayout * currentLayout();
	void setCurrentLayout(QgsLayout *layout);

	void setMasterLayout(QgsMasterLayoutInterface *layout);

	void updateActionNames(QgsMasterLayoutInterface::Type type);

	QgsLayoutAtlas *atlas();

	QPrinter* printer();

public:		// QgsLayoutDesignerInterface interface
	QgsLayout *layout();
	QgsMasterLayoutInterface *masterLayout();
	QWidget *window();
	QgsLayoutView *view();
	QgsMessageBar *messageBar();
	void selectItems(const QList< QgsLayoutItem * > &items);
	void setAtlasPreviewEnabled(bool enabled);
	bool atlasPreviewEnabled() const;
	QMenu *layoutMenu();
	QMenu *editMenu();
	QMenu *viewMenu();
	QMenu *itemsMenu();
	QMenu *atlasMenu();
	QMenu *reportMenu();
	QMenu *settingsMenu();
	QToolBar *layoutToolbar();
	QToolBar *navigationToolbar();
	QToolBar *actionsToolbar();
	QToolBar *atlasToolbar();
	void showRulers(bool visible);
	void showItemOptions(QgsLayoutItem *item, bool bringPanelToFront = true);
    void setAtlasFeature( const QgsFeature &feature );
	QgsLayoutDesignerInterface::ExportResults *lastExportResults() { return mLastExportResults; }

public:
	void activate();
	void open();

private:
	void initializeRegistry();

	void toggleActions(bool layoutAvailable);

	void createLayoutView();
	void createLayoutTools();
	void createCutCopyPasteActions();
	void createDocks();
	void createAtlasWidget();
	void createReportWidget();
	void createLayoutPropertiesWidget();
	void setupToolbars();
	void setupPreviewActions();
	void setupStatusBar();

	void itemTypeAdded(int id);
	void activateNewItemCreationTool(int id, bool nodeBasedItem);

	void loadPredefinedScalesFromProject();
	QVector<double> predefinedScales() const;

	void setPrinterPageOrientation(QgsLayoutItemPage::Orientation orientation);

	bool checkBeforeExport();
	bool containsWmsLayers() const;
	bool requiresRasterization() const;
	void showWmsPrintingWarning();
	void showRasterizationWarning();
	bool showFileSizeWarning();
	void showForceVectorWarning();
	void showSvgExportWarning();
	QString defaultExportPath();
	void setLastExportPath(const QString &path) const;
	bool getRasterExportSettings(QgsLayoutExporter::ImageExportSettings &settings, QSize &imageSize);
	bool getPdfExportSettings(QgsLayoutExporter::PdfExportSettings &settings);
	bool getSvgExportSettings(QgsLayoutExporter::SvgExportSettings &settings);
    bool containsAdvancedEffects() const;


public slots:
	void updateWindowTitle();
	void pageOrientationChanged();

	void paste();
	void atlasPreviewTriggered(bool checked);
	void atlasPageComboEditingFinished();

	void refreshLayout();
	//void saveProject();
	void newLayout();
	void showManager();
	void renameLayout();
	void deleteLayout();
	void duplicate();

	void showGrid(bool visible);
	void showBoxes(bool visible);
	void showPages(bool visible);
	void showGuides(bool visible);

	void snapToGrid(bool enabled);
	void snapToGuides(bool enabled);
	void snapToItems(bool enabled);

	void pasteInPlace();
	void atlasNext();
	void atlasPrevious();
	void atlasFirst();
	void atlasLast();

	void print();
	void exportToRaster();
	void exportToPdf();
	void exportToSvg();

	void printReport();
	void exportReportToRaster();
	void exportReportToSvg();
	void exportReportToPdf();

	void printAtlas();
	void exportAtlasToRaster();
	void exportAtlasToSvg();
	void exportAtlasToPdf();

	void pageSetup();

	void saveAsTemplate();

	void raiseSelectedItems();
	void lowerSelectedItems();
	void moveSelectedItemsToTop();
	void moveSelectedItemsToBottom();
	void unlockAllItems();
	void lockSelectedItems();
	void addItemsFromTemplate();

	void addPages();

	void setPanelVisibility(bool hidden);

	void statusZoomCombo_currentIndexChanged(int index);
	void statusZoomCombo_zoomEntered();
	void sliderZoomChanged(int value);
	void updateStatusZoom();
	void updateStatusCursorPos(QPointF position);


private:
	QString mTitle;
	QString mSectionTitle;
	GwmLayoutDesignerInterface* mInterface = nullptr;
	QgsLayoutGuideWidget *mGuideWidget = nullptr;

	std::unique_ptr<QPrinter> mPrinter = nullptr;

	//QgsLayoutAppMenuProvider *mMenuProvider = nullptr;

	QgsMessageBar *mMessageBar = nullptr;

	bool mSetPageOrientation = false;

	QgsLayoutView* mView = nullptr;
	QgsLayoutRuler *mHorizontalRuler = nullptr;
	QgsLayoutRuler *mVerticalRuler = nullptr;
	QWidget *mRulerLayoutFix = nullptr;

	QActionGroup *mToolsActionGroup = nullptr;
	QMap< QString, QToolButton * > mItemGroupToolButtons;
	QMap< QString, QMenu * > mItemGroupSubmenus;

	QgsMasterLayoutInterface* mMasterLayout = nullptr;
	QgsLayout* mLayout = nullptr;


	QgsLayoutViewToolAddItem *mAddItemTool = nullptr;
	QgsLayoutViewToolAddNodeItem *mAddNodeItemTool = nullptr;
	QgsLayoutViewToolPan *mPanTool = nullptr;
	QgsLayoutViewToolZoom *mZoomTool = nullptr;
	QgsLayoutViewToolSelect *mSelectTool = nullptr;
	QgsLayoutViewToolEditNodes *mNodesTool = nullptr;
	QgsLayoutViewToolMoveItemContent *mMoveContentTool = nullptr;

	QUndoView *mUndoView = nullptr;
	QgsDockWidget *mUndoDock = nullptr;

	QAction *mUndoAction = nullptr;
	QAction *mRedoAction = nullptr;
	QAction *mActionCut = nullptr;
	QAction *mActionCopy = nullptr;
	QAction *mActionPaste = nullptr;

	bool mBlockItemOptions = false;

	QgsDockWidget *mItemDock = nullptr;
	QgsPanelWidgetStack *mItemPropertiesStack = nullptr;
	QgsDockWidget *mGeneralDock = nullptr;
	QgsPanelWidgetStack *mGeneralPropertiesStack = nullptr;
	QgsDockWidget *mGuideDock = nullptr;
	QgsPanelWidgetStack *mGuideStack = nullptr;
	QgsDockWidget *mAtlasDock = nullptr;

	QgsDockWidget *mItemsDock = nullptr;
	QgsLayoutItemsListView *mItemsTreeView = nullptr;

	QgsDockWidget *mReportDock = nullptr;

	QgsLayoutPropertiesWidget *mLayoutPropertiesWidget = nullptr;
	QComboBox *mAtlasPageComboBox = nullptr;

	QMap< QString, PanelStatus > mPanelStatus;

	//! Combobox in status bar which shows/adjusts current zoom level
	QComboBox *mStatusZoomCombo = nullptr;
	QSlider *mStatusZoomSlider = nullptr;
	//! Labels in status bar which shows current mouse position
	QLabel *mStatusCursorXLabel = nullptr;
	QLabel *mStatusCursorYLabel = nullptr;
	QLabel *mStatusCursorPageLabel = nullptr;
	QProgressBar *mStatusProgressBar = nullptr;

	QgsLayoutDesignerInterface::ExportResults *mLastExportResults = nullptr;
};


class GwmLayoutDesignerInterface : public QgsLayoutDesignerInterface
{
public:
	GwmLayoutDesignerInterface(GwmLayoutDesigner* dialog);

	virtual QgsLayout *layout() override
	{
		return mDesigner->layout();
	}
	virtual QgsMasterLayoutInterface *masterLayout() override
	{
		return mDesigner->masterLayout();
	}
	virtual QWidget *window() override
	{
		return mDesigner;
	}
	virtual QgsLayoutView *view() override
	{
		return mDesigner->view();
	}
	virtual QgsMessageBar *messageBar() override
	{
		return mDesigner->messageBar();
	}
	virtual void selectItems(const QList< QgsLayoutItem * > &items) override
	{
		return selectItems(items);
	}
	virtual void setAtlasPreviewEnabled(bool enabled) override
	{
		mDesigner->setAtlasPreviewEnabled(enabled);
	}
	virtual bool atlasPreviewEnabled() const override
	{
		return mDesigner->atlasPreviewEnabled();
	}
	virtual void showItemOptions(QgsLayoutItem *item, bool bringPanelToFront = true) override
	{
		mDesigner->showItemOptions(item, bringPanelToFront);
	}
	virtual QMenu *layoutMenu() override
	{
		return mDesigner->layoutMenu();
	}
	virtual QMenu *editMenu() override
	{
		return mDesigner->editMenu();
	}
	virtual QMenu *viewMenu() override
	{
		return mDesigner->viewMenu();
	}
	virtual QMenu *itemsMenu() override
	{
		return mDesigner->itemsMenu();
	}
	virtual QMenu *atlasMenu() override
	{
		return mDesigner->atlasMenu();
	}
	virtual QMenu *reportMenu() override
	{
		return mDesigner->reportMenu();
	}
	virtual QMenu *settingsMenu() override
	{
		return mDesigner->settingsMenu();
	}
	virtual QToolBar *layoutToolbar() override
	{
		return mDesigner->layoutToolbar();
	}
	virtual QToolBar *navigationToolbar() override
	{
		return mDesigner->navigationToolbar();
	}
	virtual QToolBar *actionsToolbar() override
	{
		return mDesigner->actionsToolbar();
	}
	virtual QToolBar *atlasToolbar() override
	{
		return mDesigner->atlasToolbar();
	}
	virtual void addDockWidget(Qt::DockWidgetArea area, QDockWidget *dock) override
	{
		mDesigner->addDockWidget(area, dock);
	}
	virtual void removeDockWidget(QDockWidget *dock) override
	{
		mDesigner->removeDockWidget(dock);
	}
	virtual void activateTool(StandardTool tool) override
	{
		switch (tool)
		{
			case QgsLayoutDesignerInterface::ToolMoveItemContent:
				if (!mDesigner->mActionMoveItemContent->isChecked())
					mDesigner->mActionMoveItemContent->trigger();
				break;

			case QgsLayoutDesignerInterface::ToolMoveItemNodes:
				if (!mDesigner->mActionEditNodesItem->isChecked())
					mDesigner->mActionEditNodesItem->trigger();
				break;

		}
	}
	virtual void close() override
	{
		mDesigner->close();
	}
	virtual void showRulers(bool visible) override
	{
		mDesigner->showRulers(visible);
	}
    virtual void setAtlasFeature(const QgsFeature &feature) override
    {
        mDesigner->setAtlasFeature(feature);
    }
	virtual QgsLayoutDesignerInterface::ExportResults *lastExportResults() const override
	{
		return mDesigner->lastExportResults();
	}

private:
    GwmLayoutDesigner* mDesigner;
};

inline GwmLayoutDesignerInterface * GwmLayoutDesigner::iface()
{
	return mInterface;
}

inline void GwmLayoutDesigner::setTitle(const QString & title)
{
	mTitle = title;
	updateWindowTitle();
}

inline QgsLayout* GwmLayoutDesigner::currentLayout()
{
	return mLayout;
}

inline QgsMasterLayoutInterface* GwmLayoutDesigner::masterLayout()
{
	return mMasterLayout;
}

inline void GwmLayoutDesigner::pageOrientationChanged()
{
	mSetPageOrientation = false;
}

inline QgsLayout *GwmLayoutDesigner::layout()
{
	return mLayout;
}

inline QWidget *GwmLayoutDesigner::window()
{
	return this;
}

inline QgsLayoutView *GwmLayoutDesigner::view()
{
	return mView;
}

inline QgsMessageBar *GwmLayoutDesigner::messageBar()
{
	return mMessageBar;
}

inline void GwmLayoutDesigner::setAtlasPreviewEnabled(bool enabled)
{
	whileBlocking(mActionAtlasPreview)->setChecked(enabled);
}

inline bool GwmLayoutDesigner::atlasPreviewEnabled() const
{
	return mActionAtlasPreview->isChecked();
}

inline QMenu *GwmLayoutDesigner::layoutMenu()
{
	return mLayoutMenu;
}

inline QMenu *GwmLayoutDesigner::editMenu()
{
	return menuEdit;
}

inline QMenu *GwmLayoutDesigner::viewMenu()
{
	return mMenuView;
}

inline QMenu *GwmLayoutDesigner::itemsMenu()
{
	return menuLayout;
}

inline QMenu *GwmLayoutDesigner::atlasMenu()
{
	return mMenuAtlas;
}

inline QMenu *GwmLayoutDesigner::reportMenu()
{
	return mMenuReport;
}

inline QMenu *GwmLayoutDesigner::settingsMenu()
{
	return menuSettings;
}

inline QToolBar *GwmLayoutDesigner::layoutToolbar()
{
	return mLayoutToolbar;
}

inline QToolBar *GwmLayoutDesigner::navigationToolbar()
{
	return mNavigationToolbar;
}

inline QToolBar *GwmLayoutDesigner::actionsToolbar()
{
	return mActionsToolbar;
}

inline QToolBar *GwmLayoutDesigner::atlasToolbar()
{
	return mAtlasToolbar;
}

#endif // !GWMLAYOUTDESIGNER_H

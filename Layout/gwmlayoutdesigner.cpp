#include "gwmlayoutdesigner.h"

#include <QToolButton>

#include <qgsgui.h>
#include <qgsproject.h>
#include <qgslayoutitemguiregistry.h>
#include <qgslayoutpagepropertieswidget.h>
#include <qgslayoutviewtooladditem.h>
#include <qgslayoutviewtooladdnodeitem.h>
#include <qgslayoutview.h>
#include <qgslayoutruler.h>
#include <qgslayout.h>
#include <qgsprintlayout.h>
#include <qgsreport.h>
#include <qgslayoutundostack.h>
#include <qundoview.h>
#include <qgslayoutviewtoolpan.h>
#include <qgslayoutviewtoolzoom.h>
#include <qgslayoutviewtoolselect.h>
#include <qgslayoutviewtooleditnodes.h>
#include <qgslayoutviewtoolmoveitemcontent.h>
#include <qgslayoutitem.h>
#include <qgslayoutitemslistview.h>
#include <qgsdockwidget.h>
#include <qgspanelwidgetstack.h>
#include <qgslayoutdesignerinterface.h>
#include <qgslayoutatlas.h>
#include <qgslayoutatlaswidget.h>
#include <qgslayoutpropertieswidget.h>
#include <qgslayoutguidewidget.h>
#include <qgsmessagebar.h>
#include <qgsprojectviewsettings.h>
#include <qgslayoutatlas.h>

#include "gwmapp.h"


#define FIT_LAYOUT -101
#define FIT_LAYOUT_WIDTH -102


static bool cmpByText_(QAction *a, QAction *b)
{
	return QString::localeAwareCompare(a->text(), b->text()) < 0;
}


GwmLayoutDesignerInterface::GwmLayoutDesignerInterface(GwmLayoutDesigner * dialog)
	: QgsLayoutDesignerInterface(dialog)
{
	mDesigner = dialog;
}

bool GwmLayoutDesigner::sInitializedRegistry = false;


GwmLayoutDesigner::GwmLayoutDesigner(QWidget *parent)
	: QMainWindow(parent)
	, mToolsActionGroup(new QActionGroup(this))
	, mInterface(new GwmLayoutDesignerInterface(this))
{
	if (!sInitializedRegistry)
	{
		initializeRegistry();
	}
	setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose);
	setDockOptions(dockOptions() | QMainWindow::GroupedDragging);

	createLayoutView();

	// populate with initial items...
	const QList< int > itemMetadataIds = QgsGui::layoutItemGuiRegistry()->itemMetadataIds();
	for (int id : itemMetadataIds)
	{
		itemTypeAdded(id);
	}
	//..and listen out for new item types
	connect(QgsGui::layoutItemGuiRegistry(), &QgsLayoutItemGuiRegistry::typeAdded, this, &GwmLayoutDesigner::itemTypeAdded);

	QToolButton *orderingToolButton = new QToolButton(this);
	orderingToolButton->setPopupMode(QToolButton::InstantPopup);
	orderingToolButton->setAutoRaise(true);
	orderingToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	orderingToolButton->addAction(mActionRaiseItems);
	orderingToolButton->addAction(mActionLowerItems);
	orderingToolButton->addAction(mActionMoveItemsToTop);
	orderingToolButton->addAction(mActionMoveItemsToBottom);
	orderingToolButton->setDefaultAction(mActionRaiseItems);
	mActionsToolbar->addWidget(orderingToolButton);

	QToolButton *alignToolButton = new QToolButton(this);
	alignToolButton->setPopupMode(QToolButton::InstantPopup);
	alignToolButton->setAutoRaise(true);
	alignToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	alignToolButton->addAction(mActionAlignLeft);
	alignToolButton->addAction(mActionAlignHCenter);
	alignToolButton->addAction(mActionAlignRight);
	alignToolButton->addAction(mActionAlignTop);
	alignToolButton->addAction(mActionAlignVCenter);
	alignToolButton->addAction(mActionAlignBottom);
	alignToolButton->setDefaultAction(mActionAlignLeft);
	mActionsToolbar->addWidget(alignToolButton);

	QToolButton *distributeToolButton = new QToolButton(this);
	distributeToolButton->setPopupMode(QToolButton::InstantPopup);
	distributeToolButton->setAutoRaise(true);
	distributeToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	distributeToolButton->addAction(mActionDistributeLeft);
	distributeToolButton->addAction(mActionDistributeHCenter);
	distributeToolButton->addAction(mActionDistributeHSpace);
	distributeToolButton->addAction(mActionDistributeRight);
	distributeToolButton->addAction(mActionDistributeTop);
	distributeToolButton->addAction(mActionDistributeVCenter);
	distributeToolButton->addAction(mActionDistributeVSpace);
	distributeToolButton->addAction(mActionDistributeBottom);
	distributeToolButton->setDefaultAction(mActionDistributeLeft);
	mActionsToolbar->addWidget(distributeToolButton);

	QToolButton *resizeToolButton = new QToolButton(this);
	resizeToolButton->setPopupMode(QToolButton::InstantPopup);
	resizeToolButton->setAutoRaise(true);
	resizeToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	resizeToolButton->addAction(mActionResizeNarrowest);
	resizeToolButton->addAction(mActionResizeWidest);
	resizeToolButton->addAction(mActionResizeShortest);
	resizeToolButton->addAction(mActionResizeTallest);
	resizeToolButton->addAction(mActionResizeToSquare);
	resizeToolButton->setDefaultAction(mActionResizeNarrowest);
	mActionsToolbar->addWidget(resizeToolButton);

	QToolButton *atlasExportToolButton = new QToolButton(mAtlasToolbar);
	atlasExportToolButton->setIcon(QgsApplication::getThemeIcon(QStringLiteral("mActionExport.svg")));
	atlasExportToolButton->setPopupMode(QToolButton::InstantPopup);
	atlasExportToolButton->setAutoRaise(true);
	atlasExportToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	atlasExportToolButton->addAction(mActionExportAtlasAsImage);
	atlasExportToolButton->addAction(mActionExportAtlasAsSVG);
	atlasExportToolButton->addAction(mActionExportAtlasAsPDF);
	atlasExportToolButton->setToolTip(tr("Export Atlas"));
	mAtlasToolbar->insertWidget(mActionAtlasSettings, atlasExportToolButton);
	mAtlasPageComboBox = new QComboBox();
	mAtlasPageComboBox->setEditable(true);
	mAtlasPageComboBox->addItem(QString::number(1));
	mAtlasPageComboBox->setCurrentIndex(0);
	mAtlasPageComboBox->setMinimumHeight(mAtlasToolbar->height());
	mAtlasPageComboBox->setMinimumContentsLength(6);
	mAtlasPageComboBox->setMaxVisibleItems(20);
	mAtlasPageComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	mAtlasPageComboBox->setInsertPolicy(QComboBox::NoInsert);
	connect(mAtlasPageComboBox->lineEdit(), &QLineEdit::editingFinished, this, &GwmLayoutDesigner::atlasPageComboEditingFinished);
	connect(mAtlasPageComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmLayoutDesigner::atlasPageComboEditingFinished);
	mAtlasToolbar->insertWidget(mActionAtlasNext, mAtlasPageComboBox);

	createLayoutTools();
	createCutCopyPasteActions();

	setupStatusBar();

	connect(mView, &QgsLayoutView::itemFocused, this, [=](QgsLayoutItem * item)
	{
		showItemOptions(item, false);
	});

	setupPreviewActions();
	setupToolbars();

	createDocks();

	mActionAtlasPreview->setEnabled(false);
	mActionAtlasPreview->setChecked(false);
	mActionAtlasFirst->setEnabled(false);
	mActionAtlasLast->setEnabled(false);
	mActionAtlasNext->setEnabled(false);
	mActionAtlasPrev->setEnabled(false);
	mActionPrintAtlas->setEnabled(false);
	mAtlasPageComboBox->setEnabled(false);
	mActionExportAtlasAsImage->setEnabled(false);
	mActionExportAtlasAsSVG->setEnabled(false);
	mActionExportAtlasAsPDF->setEnabled(false);

	mLayoutsMenu->setObjectName(QStringLiteral("mLayoutsMenu"));
	connect(mLayoutsMenu, &QMenu::aboutToShow, this, [&]()
	{
		GwmApp::Instance()->populateLayoutsMenu(mLayoutMenu);
	});

	QList<QAction *> actions = mPanelsMenu->actions();
	std::sort(actions.begin(), actions.end(), cmpByText_);
	mPanelsMenu->insertActions(nullptr, actions);

	actions = mToolbarMenu->actions();
	std::sort(actions.begin(), actions.end(), cmpByText_);
	mToolbarMenu->insertActions(nullptr, actions);

	//restoreWindowState();

	//listen out to status bar updates from the view
	//connect(mView, &QgsLayoutView::statusMessage, this, &GwmLayoutDesigner::statusMessageReceived);

	connect(QgsProject::instance(), &QgsProject::isDirtyChanged, this, &GwmLayoutDesigner::updateWindowTitle);
}

GwmLayoutDesigner::~GwmLayoutDesigner()
{
}

void GwmLayoutDesigner::initializeRegistry()
{
	sInitializedRegistry = true;
	auto createPageWidget = ([](QgsLayoutItem * item)->QgsLayoutItemBaseWidget *
	{
		std::unique_ptr< QgsLayoutPagePropertiesWidget > newWidget = qgis::make_unique< QgsLayoutPagePropertiesWidget >(nullptr, item);
		return newWidget.release();
	});

	QgsGui::layoutItemGuiRegistry()->addLayoutItemGuiMetadata(new QgsLayoutItemGuiMetadata(QgsLayoutItemRegistry::LayoutPage, QObject::tr("Page"), QIcon(), createPageWidget, nullptr, QString(), false, QgsLayoutItemAbstractGuiMetadata::FlagNoCreationTools));

}

void GwmLayoutDesigner::showRulers(bool visible)
{
	//show or hide rulers
	mHorizontalRuler->setVisible(visible);
	mVerticalRuler->setVisible(visible);
	mRulerLayoutFix->setVisible(visible);
}

void GwmLayoutDesigner::createLayoutView()
{
	// create layout view
	QGridLayout* viewLayout = new QGridLayout();
	viewLayout->setSpacing(0);
	viewLayout->setMargin(0);
	viewLayout->setContentsMargins(0, 0, 0, 0);
	centralWidget()->layout()->setSpacing(0);
	centralWidget()->layout()->setMargin(0);
	centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
	mHorizontalRuler = new QgsLayoutRuler(nullptr, Qt::Horizontal);
	mVerticalRuler = new QgsLayoutRuler(nullptr, Qt::Vertical);
	mRulerLayoutFix = new QWidget();
	mRulerLayoutFix->setAttribute(Qt::WA_NoMousePropagation);
	mRulerLayoutFix->setBackgroundRole(QPalette::Window);
	mRulerLayoutFix->setFixedSize(mVerticalRuler->rulerSize(), mHorizontalRuler->rulerSize());
	viewLayout->addWidget(mRulerLayoutFix, 0, 0);
	viewLayout->addWidget(mHorizontalRuler, 0, 1);
	viewLayout->addWidget(mVerticalRuler, 1, 0);//initial state of rulers

	bool showRulers = true;
	mActionShowRulers->setChecked(showRulers);
	mHorizontalRuler->setVisible(showRulers);
	mVerticalRuler->setVisible(showRulers);
	mRulerLayoutFix->setVisible(showRulers);
	mActionShowRulers->blockSignals(false);
	connect(mActionShowRulers, &QAction::triggered, this, &GwmLayoutDesigner::showRulers);

	QMenu *rulerMenu = new QMenu(this);
	rulerMenu->addAction(mActionShowGuides);
	rulerMenu->addAction(mActionSnapGuides);
	rulerMenu->addAction(mActionManageGuides);
	rulerMenu->addAction(mActionClearGuides);
	rulerMenu->addSeparator();
	rulerMenu->addAction(mActionShowRulers);
	mHorizontalRuler->setContextMenu(rulerMenu);
	mVerticalRuler->setContextMenu(rulerMenu);

	mView = new QgsLayoutView(mViewFrame);
	//mView->setMapCanvas( mQgis->mapCanvas() );
	mView->setContentsMargins(0, 0, 0, 0);
	mView->setHorizontalRuler(mHorizontalRuler);
	mView->setVerticalRuler(mVerticalRuler);
	viewLayout->addWidget(mView, 1, 1);
	//view does not accept focus via tab
	mView->setFocusPolicy(Qt::ClickFocus);
	mViewFrame->setLayout(viewLayout);
	mViewFrame->setContentsMargins(0, 0, 0, 1); // 1 is deliberate!
	mView->setFrameShape(QFrame::NoFrame);
}

void GwmLayoutDesigner::createLayoutTools()
{
	mAddItemTool = new QgsLayoutViewToolAddItem(mView);
	mAddNodeItemTool = new QgsLayoutViewToolAddNodeItem(mView);
	mPanTool = new QgsLayoutViewToolPan(mView);
	mPanTool->setAction(mActionPan);
	mToolsActionGroup->addAction(mActionPan);
	connect(mActionPan, &QAction::triggered, mPanTool, [=] { mView->setTool(mPanTool); });
	mZoomTool = new QgsLayoutViewToolZoom(mView);
	mZoomTool->setAction(mActionZoomTool);
	mToolsActionGroup->addAction(mActionZoomTool);
	connect(mActionZoomTool, &QAction::triggered, mZoomTool, [=] { mView->setTool(mZoomTool); });
	mSelectTool = new QgsLayoutViewToolSelect(mView);
	mSelectTool->setAction(mActionSelectMoveItem);
	mToolsActionGroup->addAction(mActionSelectMoveItem);
	connect(mActionSelectMoveItem, &QAction::triggered, mSelectTool, [=] { mView->setTool(mSelectTool); });
	// after creating an item with the add item tool, switch immediately to select tool
	connect(mAddItemTool, &QgsLayoutViewToolAddItem::createdItem, this, [=] { mView->setTool(mSelectTool); });
	connect(mAddNodeItemTool, &QgsLayoutViewToolAddNodeItem::createdItem, this, [=] { mView->setTool(mSelectTool); });

	mNodesTool = new QgsLayoutViewToolEditNodes(mView);
	mNodesTool->setAction(mActionEditNodesItem);
	mToolsActionGroup->addAction(mActionEditNodesItem);
	connect(mActionEditNodesItem, &QAction::triggered, mNodesTool, [=] { mView->setTool(mNodesTool); });

	mMoveContentTool = new QgsLayoutViewToolMoveItemContent(mView);
	mMoveContentTool->setAction(mActionMoveItemContent);
	mToolsActionGroup->addAction(mActionMoveItemContent);
	connect(mActionMoveItemContent, &QAction::triggered, mMoveContentTool, [=] { mView->setTool(mMoveContentTool); });

}

void GwmLayoutDesigner::createCutCopyPasteActions()
{
	//cut/copy/paste actions. Note these are not included in the ui file
  //as ui files have no support for QKeySequence shortcuts
	mActionCut = new QAction(tr("Cu&t"), this);
	mActionCut->setShortcuts(QKeySequence::Cut);
	mActionCut->setStatusTip(tr("Cut"));
	mActionCut->setIcon(QgsApplication::getThemeIcon(QStringLiteral("/mActionEditCut.svg")));
	connect(mActionCut, &QAction::triggered, this, [=]
	{
		mView->copySelectedItems(QgsLayoutView::ClipboardCut);
	});

	mActionCopy = new QAction(tr("&Copy"), this);
	mActionCopy->setShortcuts(QKeySequence::Copy);
	mActionCopy->setStatusTip(tr("Copy"));
	mActionCopy->setIcon(QgsApplication::getThemeIcon(QStringLiteral("/mActionEditCopy.svg")));
	connect(mActionCopy, &QAction::triggered, this, [=]
	{
		mView->copySelectedItems(QgsLayoutView::ClipboardCopy);
	});

	mActionPaste = new QAction(tr("&Paste"), this);
	mActionPaste->setShortcuts(QKeySequence::Paste);
	mActionPaste->setStatusTip(tr("Paste"));
	mActionPaste->setIcon(QgsApplication::getThemeIcon(QStringLiteral("/mActionEditPaste.svg")));
	connect(mActionPaste, &QAction::triggered, this, &GwmLayoutDesigner::paste);

	menuEdit->insertAction(mActionPasteInPlace, mActionCut);
	menuEdit->insertAction(mActionPasteInPlace, mActionCopy);
	menuEdit->insertAction(mActionPasteInPlace, mActionPaste);
}

void GwmLayoutDesigner::createDocks()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
	int minDockWidth(fontMetrics().width(QStringLiteral("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")));
#else
	int minDockWidth = fontMetrics().horizontalAdvance('X') * 38;
#endif

	setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
	mGeneralDock = new QgsDockWidget(tr("Layout"), this);
	mGeneralDock->setObjectName(QStringLiteral("LayoutDock"));
	mGeneralDock->setMinimumWidth(minDockWidth);
	mGeneralPropertiesStack = new QgsPanelWidgetStack();
	mGeneralDock->setWidget(mGeneralPropertiesStack);
	mPanelsMenu->addAction(mGeneralDock->toggleViewAction());
	connect(mActionLayoutProperties, &QAction::triggered, this, [=]
	{
		mGeneralDock->setUserVisible(true);
	});

	mItemDock = new QgsDockWidget(tr("Item Properties"), this);
	mItemDock->setObjectName(QStringLiteral("ItemDock"));
	mItemDock->setMinimumWidth(minDockWidth);
	mItemPropertiesStack = new QgsPanelWidgetStack();
	mItemDock->setWidget(mItemPropertiesStack);
	mPanelsMenu->addAction(mItemDock->toggleViewAction());

	mGuideDock = new QgsDockWidget(tr("Guides"), this);
	mGuideDock->setObjectName(QStringLiteral("GuideDock"));
	mGuideDock->setMinimumWidth(minDockWidth);
	mGuideStack = new QgsPanelWidgetStack();
	mGuideDock->setWidget(mGuideStack);
	mPanelsMenu->addAction(mGuideDock->toggleViewAction());
	connect(mActionManageGuides, &QAction::triggered, this, [=]
	{
		mGuideDock->setUserVisible(true);
	});

	mUndoDock = new QgsDockWidget(tr("Undo History"), this);
	mUndoDock->setObjectName(QStringLiteral("UndoDock"));
	mPanelsMenu->addAction(mUndoDock->toggleViewAction());
	mUndoView = new QUndoView(this);
	mUndoDock->setWidget(mUndoView);

	mItemsDock = new QgsDockWidget(tr("Items"), this);
	mItemsDock->setObjectName(QStringLiteral("ItemsDock"));
	mPanelsMenu->addAction(mItemsDock->toggleViewAction());

	//items tree widget
	mItemsTreeView = new QgsLayoutItemsListView(mItemsDock, iface());
	mItemsDock->setWidget(mItemsTreeView);

	mAtlasDock = new QgsDockWidget(tr("Atlas"), this);
	mAtlasDock->setObjectName(QStringLiteral("AtlasDock"));
	mAtlasDock->setToggleVisibilityAction(mActionAtlasSettings);

	mReportDock = new QgsDockWidget(tr("Report Organizer"), this);
	mReportDock->setObjectName(QStringLiteral("ReportDock"));
	mReportDock->setToggleVisibilityAction(mActionReportSettings);

	const QList<QDockWidget *> docks = findChildren<QDockWidget *>();
	for (QDockWidget *dock : docks)
	{
		connect(dock, &QDockWidget::visibilityChanged, this, [&](bool visible) 
		{
			if (visible)
			{
				whileBlocking(mActionHidePanels)->setChecked(false);
			}
		});
	}

	addDockWidget(Qt::RightDockWidgetArea, mItemDock);
	addDockWidget(Qt::RightDockWidgetArea, mGeneralDock);
	addDockWidget(Qt::RightDockWidgetArea, mGuideDock);
	addDockWidget(Qt::RightDockWidgetArea, mUndoDock);
	addDockWidget(Qt::RightDockWidgetArea, mItemsDock);
	addDockWidget(Qt::RightDockWidgetArea, mAtlasDock);
	addDockWidget(Qt::LeftDockWidgetArea, mReportDock);

	createLayoutPropertiesWidget();

	mUndoDock->show();
	mItemDock->show();
	mGeneralDock->show();
	mAtlasDock->show();
	mReportDock->show();
	mItemsDock->show();

	tabifyDockWidget(mGeneralDock, mUndoDock);
	tabifyDockWidget(mItemDock, mUndoDock);
	tabifyDockWidget(mGeneralDock, mItemDock);
	tabifyDockWidget(mItemDock, mItemsDock);
	tabifyDockWidget(mItemDock, mAtlasDock);

	toggleActions(false);
}

void GwmLayoutDesigner::createLayoutPropertiesWidget()
{
	if (!mLayout)
	{
		return;
	}

	// update layout based widgets
	QgsLayoutPropertiesWidget *oldLayoutWidget = qobject_cast<QgsLayoutPropertiesWidget *>(mGeneralPropertiesStack->takeMainPanel());
	delete oldLayoutWidget;
	QgsLayoutGuideWidget *oldGuideWidget = qobject_cast<QgsLayoutGuideWidget *>(mGuideStack->takeMainPanel());
	delete oldGuideWidget;

	mLayoutPropertiesWidget = new QgsLayoutPropertiesWidget(mGeneralDock, mLayout);
	mLayoutPropertiesWidget->setDockMode(true);
	mLayoutPropertiesWidget->setMasterLayout(mMasterLayout);
	mGeneralPropertiesStack->setMainPanel(mLayoutPropertiesWidget);

	mGuideWidget = new QgsLayoutGuideWidget(mGuideDock, mLayout, mView);
	mGuideWidget->setDockMode(true);
	mGuideStack->setMainPanel(mGuideWidget);
}

void GwmLayoutDesigner::setupToolbars()
{
	// Panel and toolbar submenus
	mToolbarMenu->addAction(mLayoutToolbar->toggleViewAction());
	mToolbarMenu->addAction(mNavigationToolbar->toggleViewAction());
	mToolbarMenu->addAction(mToolsToolbar->toggleViewAction());
	mToolbarMenu->addAction(mActionsToolbar->toggleViewAction());
	mToolbarMenu->addAction(mAtlasToolbar->toggleViewAction());
	mToolbarMenu->addAction(mReportToolbar->toggleViewAction());

	//connect(mActionToggleFullScreen, &QAction::toggled, this, &GwmLayoutDesigner::toggleFullScreen);

	//mMenuProvider = new QgsLayoutAppMenuProvider(this);
	//mView->setMenuProvider(mMenuProvider);

	connect(mActionRefreshView, &QAction::triggered, this, &GwmLayoutDesigner::refreshLayout);
	//connect(mActionSaveProject, &QAction::triggered, this, &GwmLayoutDesigner::saveProject);
	//connect(mActionNewLayout, &QAction::triggered, this, &GwmLayoutDesigner::newLayout);
	//connect(mActionLayoutManager, &QAction::triggered, this, &GwmLayoutDesigner::showManager);
	//connect(mActionRemoveLayout, &QAction::triggered, this, &GwmLayoutDesigner::deleteLayout);

	//connect(mActionPrint, &QAction::triggered, this, &GwmLayoutDesigner::print);
	//connect(mActionExportAsImage, &QAction::triggered, this, &GwmLayoutDesigner::exportToRaster);
	//connect(mActionExportAsPDF, &QAction::triggered, this, &GwmLayoutDesigner::exportToPdf);
	//connect(mActionExportAsSVG, &QAction::triggered, this, &GwmLayoutDesigner::exportToSvg);

	//connect(mActionShowGrid, &QAction::triggered, this, &GwmLayoutDesigner::showGrid);
	//connect(mActionSnapGrid, &QAction::triggered, this, &GwmLayoutDesigner::snapToGrid);

	//connect(mActionShowGuides, &QAction::triggered, this, &GwmLayoutDesigner::showGuides);
	//connect(mActionSnapGuides, &QAction::triggered, this, &GwmLayoutDesigner::snapToGuides);
	//connect(mActionSmartGuides, &QAction::triggered, this, &GwmLayoutDesigner::snapToItems);

	//connect(mActionShowBoxes, &QAction::triggered, this, &GwmLayoutDesigner::showBoxes);
	//connect(mActionShowPage, &QAction::triggered, this, &GwmLayoutDesigner::showPages);

	//connect(mActionPasteInPlace, &QAction::triggered, this, &GwmLayoutDesigner::pasteInPlace);
	//connect(mActionAtlasPreview, &QAction::triggered, this, &GwmLayoutDesigner::atlasPreviewTriggered);
	//connect(mActionAtlasNext, &QAction::triggered, this, &GwmLayoutDesigner::atlasNext);
	//connect(mActionAtlasPrev, &QAction::triggered, this, &GwmLayoutDesigner::atlasPrevious);
	//connect(mActionAtlasFirst, &QAction::triggered, this, &GwmLayoutDesigner::atlasFirst);
	//connect(mActionAtlasLast, &QAction::triggered, this, &GwmLayoutDesigner::atlasLast);
	//connect(mActionPrintAtlas, &QAction::triggered, this, &GwmLayoutDesigner::printAtlas);
	//connect(mActionExportAtlasAsImage, &QAction::triggered, this, &GwmLayoutDesigner::exportAtlasToRaster);
	//connect(mActionExportAtlasAsSVG, &QAction::triggered, this, &GwmLayoutDesigner::exportAtlasToSvg);
	//connect(mActionExportAtlasAsPDF, &QAction::triggered, this, &GwmLayoutDesigner::exportAtlasToPdf);

	//connect(mActionExportReportAsImage, &QAction::triggered, this, &GwmLayoutDesigner::exportReportToRaster);
	//connect(mActionExportReportAsSVG, &QAction::triggered, this, &GwmLayoutDesigner::exportReportToSvg);
	//connect(mActionExportReportAsPDF, &QAction::triggered, this, &GwmLayoutDesigner::exportReportToPdf);
	//connect(mActionPrintReport, &QAction::triggered, this, &GwmLayoutDesigner::printReport);

	//connect(mActionPageSetup, &QAction::triggered, this, &GwmLayoutDesigner::pageSetup);

	//connect(mActionOptions, &QAction::triggered, this, [=]
	//{
	//	GwmApp::Instance()->showOptionsDialog(this, QStringLiteral("mOptionsPageComposer"));
	//});

	/*connect(mActionSaveAsTemplate, &QAction::triggered, this, &GwmLayoutDesigner::saveAsTemplate);
	connect(mActionLoadFromTemplate, &QAction::triggered, this, &GwmLayoutDesigner::addItemsFromTemplate);
	connect(mActionDuplicateLayout, &QAction::triggered, this, &GwmLayoutDesigner::duplicate);
	connect(mActionRenameLayout, &QAction::triggered, this, &GwmLayoutDesigner::renameLayout);

	connect(mActionZoomIn, &QAction::triggered, mView, &QgsLayoutView::zoomIn);
	connect(mActionZoomOut, &QAction::triggered, mView, &QgsLayoutView::zoomOut);
	connect(mActionZoomAll, &QAction::triggered, mView, &QgsLayoutView::zoomFull);
	connect(mActionZoomActual, &QAction::triggered, mView, &QgsLayoutView::zoomActual);
	connect(mActionZoomToWidth, &QAction::triggered, mView, &QgsLayoutView::zoomWidth);

	connect(mActionSelectAll, &QAction::triggered, mView, &QgsLayoutView::selectAll);
	connect(mActionDeselectAll, &QAction::triggered, mView, &QgsLayoutView::deselectAll);
	connect(mActionInvertSelection, &QAction::triggered, mView, &QgsLayoutView::invertSelection);
	connect(mActionSelectNextAbove, &QAction::triggered, mView, &QgsLayoutView::selectNextItemAbove);
	connect(mActionSelectNextBelow, &QAction::triggered, mView, &QgsLayoutView::selectNextItemBelow);

	connect(mActionRaiseItems, &QAction::triggered, this, &GwmLayoutDesigner::raiseSelectedItems);
	connect(mActionLowerItems, &QAction::triggered, this, &GwmLayoutDesigner::lowerSelectedItems);
	connect(mActionMoveItemsToTop, &QAction::triggered, this, &GwmLayoutDesigner::moveSelectedItemsToTop);
	connect(mActionMoveItemsToBottom, &QAction::triggered, this, &GwmLayoutDesigner::moveSelectedItemsToBottom);*/
	connect(mActionAlignLeft, &QAction::triggered, this, [=]
	{
		mView->alignSelectedItems(QgsLayoutAligner::AlignLeft);
	});
	connect(mActionAlignHCenter, &QAction::triggered, this, [=]
	{
		mView->alignSelectedItems(QgsLayoutAligner::AlignHCenter);
	});
	connect(mActionAlignRight, &QAction::triggered, this, [=]
	{
		mView->alignSelectedItems(QgsLayoutAligner::AlignRight);
	});
	connect(mActionAlignTop, &QAction::triggered, this, [=]
	{
		mView->alignSelectedItems(QgsLayoutAligner::AlignTop);
	});
	connect(mActionAlignVCenter, &QAction::triggered, this, [=]
	{
		mView->alignSelectedItems(QgsLayoutAligner::AlignVCenter);
	});
	connect(mActionAlignBottom, &QAction::triggered, this, [=]
	{
		mView->alignSelectedItems(QgsLayoutAligner::AlignBottom);
	});
	connect(mActionDistributeLeft, &QAction::triggered, this, [=]
	{
		mView->distributeSelectedItems(QgsLayoutAligner::DistributeLeft);
	});
	connect(mActionDistributeHCenter, &QAction::triggered, this, [=]
	{
		mView->distributeSelectedItems(QgsLayoutAligner::DistributeHCenter);
	});
	connect(mActionDistributeHSpace, &QAction::triggered, this, [=]
	{
		mView->distributeSelectedItems(QgsLayoutAligner::DistributeHSpace);
	});
	connect(mActionDistributeRight, &QAction::triggered, this, [=]
	{
		mView->distributeSelectedItems(QgsLayoutAligner::DistributeRight);
	});
	connect(mActionDistributeTop, &QAction::triggered, this, [=]
	{
		mView->distributeSelectedItems(QgsLayoutAligner::DistributeTop);
	});
	connect(mActionDistributeVCenter, &QAction::triggered, this, [=]
	{
		mView->distributeSelectedItems(QgsLayoutAligner::DistributeVCenter);
	});
	connect(mActionDistributeVSpace, &QAction::triggered, this, [=]
	{
		mView->distributeSelectedItems(QgsLayoutAligner::DistributeVSpace);
	});
	connect(mActionDistributeBottom, &QAction::triggered, this, [=]
	{
		mView->distributeSelectedItems(QgsLayoutAligner::DistributeBottom);
	});
	connect(mActionResizeNarrowest, &QAction::triggered, this, [=]
	{
		mView->resizeSelectedItems(QgsLayoutAligner::ResizeNarrowest);
	});
	connect(mActionResizeWidest, &QAction::triggered, this, [=]
	{
		mView->resizeSelectedItems(QgsLayoutAligner::ResizeWidest);
	});
	connect(mActionResizeShortest, &QAction::triggered, this, [=]
	{
		mView->resizeSelectedItems(QgsLayoutAligner::ResizeShortest);
	});
	connect(mActionResizeTallest, &QAction::triggered, this, [=]
	{
		mView->resizeSelectedItems(QgsLayoutAligner::ResizeTallest);
	});
	connect(mActionResizeToSquare, &QAction::triggered, this, [=]
	{
		mView->resizeSelectedItems(QgsLayoutAligner::ResizeToSquare);
	});

	/*connect(mActionAddPages, &QAction::triggered, this, &GwmLayoutDesigner::addPages);

	connect(mActionUnlockAll, &QAction::triggered, this, &GwmLayoutDesigner::unlockAllItems);
	connect(mActionLockItems, &QAction::triggered, this, &GwmLayoutDesigner::lockSelectedItems);

	connect(mActionHidePanels, &QAction::toggled, this, &GwmLayoutDesigner::setPanelVisibility);*/

	connect(mActionDeleteSelection, &QAction::triggered, this, [=]
	{
		if (mView->tool() == mNodesTool)
			mNodesTool->deleteSelectedNode();
		else
			mView->deleteSelectedItems();
	});
	connect(mActionGroupItems, &QAction::triggered, this, [=]
	{
		mView->groupSelectedItems();
	});
	connect(mActionUngroupItems, &QAction::triggered, this, [=]
	{
		mView->ungroupSelectedItems();
	});
}

void GwmLayoutDesigner::setupPreviewActions()
{
	mActionPreviewModeOff->setChecked(true);
	connect(mActionPreviewModeOff, &QAction::triggered, this, [=]
	{
		mView->setPreviewModeEnabled(false);
	});
	connect(mActionPreviewModeGrayscale, &QAction::triggered, this, [=]
	{
		mView->setPreviewMode(QgsPreviewEffect::PreviewGrayscale);
		mView->setPreviewModeEnabled(true);
	});
	connect(mActionPreviewModeMono, &QAction::triggered, this, [=]
	{
		mView->setPreviewMode(QgsPreviewEffect::PreviewMono);
		mView->setPreviewModeEnabled(true);
	});
	connect(mActionPreviewProtanope, &QAction::triggered, this, [=]
	{
		mView->setPreviewMode(QgsPreviewEffect::PreviewProtanope);
		mView->setPreviewModeEnabled(true);
	});
	connect(mActionPreviewDeuteranope, &QAction::triggered, this, [=]
	{
		mView->setPreviewMode(QgsPreviewEffect::PreviewDeuteranope);
		mView->setPreviewModeEnabled(true);
	});
	QActionGroup *previewGroup = new QActionGroup(this);
	previewGroup->setExclusive(true);
	mActionPreviewModeOff->setActionGroup(previewGroup);
	mActionPreviewModeGrayscale->setActionGroup(previewGroup);
	mActionPreviewModeMono->setActionGroup(previewGroup);
	mActionPreviewProtanope->setActionGroup(previewGroup);
	mActionPreviewDeuteranope->setActionGroup(previewGroup);
}

void GwmLayoutDesigner::setupStatusBar()
{
	// Add a progress bar to the status bar for indicating rendering in progress
	mStatusProgressBar = new QProgressBar(mStatusBar);
	mStatusProgressBar->setObjectName(QStringLiteral("mProgressBar"));
	mStatusProgressBar->setMaximumWidth(100);
	mStatusProgressBar->setMaximumHeight(18);
	mStatusProgressBar->hide();
	mStatusBar->addPermanentWidget(mStatusProgressBar, 1);

	//create status bar labels
	mStatusCursorXLabel = new QLabel(mStatusBar);
	mStatusCursorXLabel->setMinimumWidth(100);
	mStatusCursorYLabel = new QLabel(mStatusBar);
	mStatusCursorYLabel->setMinimumWidth(100);
	mStatusCursorPageLabel = new QLabel(mStatusBar);
	mStatusCursorPageLabel->setMinimumWidth(100);

	mStatusBar->addPermanentWidget(mStatusCursorXLabel);
	mStatusBar->addPermanentWidget(mStatusCursorXLabel);
	mStatusBar->addPermanentWidget(mStatusCursorYLabel);
	mStatusBar->addPermanentWidget(mStatusCursorPageLabel);

	mStatusZoomCombo = new QComboBox();
	mStatusZoomCombo->setEditable(true);
	mStatusZoomCombo->setInsertPolicy(QComboBox::NoInsert);
	mStatusZoomCombo->setCompleter(nullptr);
	mStatusZoomCombo->setMinimumWidth(100);
	//zoom combo box accepts decimals in the range 1-9999, with an optional decimal point and "%" sign
	QRegularExpression zoomRx(QStringLiteral("\\s*\\d{1,4}(\\.\\d?)?\\s*%?"));
	QValidator *zoomValidator = new QRegularExpressionValidator(zoomRx, mStatusZoomCombo);
	mStatusZoomCombo->lineEdit()->setValidator(zoomValidator);

	for (double level : { 0.125, 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 })
	{
		mStatusZoomCombo->insertItem(0, tr("%1%").arg(level * 100.0, 0, 'f', 1), level);
	}
	mStatusZoomCombo->insertItem(0, tr("Fit Layout"), FIT_LAYOUT);
	mStatusZoomCombo->insertItem(0, tr("Fit Layout Width"), FIT_LAYOUT_WIDTH);
	connect(mStatusZoomCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &GwmLayoutDesigner::statusZoomCombo_currentIndexChanged);
	connect(mStatusZoomCombo->lineEdit(), &QLineEdit::returnPressed, this, &GwmLayoutDesigner::statusZoomCombo_zoomEntered);

	mStatusZoomSlider = new QSlider();
	mStatusZoomSlider->setFixedWidth(mStatusZoomCombo->width());
	mStatusZoomSlider->setOrientation(Qt::Horizontal);
	mStatusZoomSlider->setMinimum(20);
	mStatusZoomSlider->setMaximum(800);
	connect(mStatusZoomSlider, &QSlider::valueChanged, this, &GwmLayoutDesigner::sliderZoomChanged);

	mStatusZoomCombo->setToolTip(tr("Zoom level"));
	mStatusZoomSlider->setToolTip(tr("Zoom level"));

	mStatusBar->addPermanentWidget(mStatusZoomCombo);
	mStatusBar->addPermanentWidget(mStatusZoomSlider);

	mView->setTool(mSelectTool);
	mView->setFocus();
	connect(mView, &QgsLayoutView::zoomLevelChanged, this, &GwmLayoutDesigner::updateStatusZoom);
	connect(mView, &QgsLayoutView::cursorPosChanged, this, &GwmLayoutDesigner::updateStatusCursorPos);
	//also listen out for position updates from the horizontal/vertical rulers
	connect(mHorizontalRuler, &QgsLayoutRuler::cursorPosChanged, this, &GwmLayoutDesigner::updateStatusCursorPos);
	connect(mVerticalRuler, &QgsLayoutRuler::cursorPosChanged, this, &GwmLayoutDesigner::updateStatusCursorPos);
}

void GwmLayoutDesigner::itemTypeAdded(int id)
{
	if (QgsGui::layoutItemGuiRegistry()->itemMetadata(id)->flags() & QgsLayoutItemAbstractGuiMetadata::FlagNoCreationTools)
		return;


	QString name = QgsGui::layoutItemGuiRegistry()->itemMetadata(id)->visibleName();
	QString groupId = QgsGui::layoutItemGuiRegistry()->itemMetadata(id)->groupId();
	bool nodeBased = QgsGui::layoutItemGuiRegistry()->itemMetadata(id)->isNodeBased();
	QToolButton *groupButton = nullptr;
	QMenu *itemSubmenu = nullptr;
	if (!groupId.isEmpty())
	{
		// find existing group toolbutton and submenu, or create new ones if this is the first time the group has been encountered
		const QgsLayoutItemGuiGroup &group = QgsGui::layoutItemGuiRegistry()->itemGroup(groupId);
		QIcon groupIcon = group.icon.isNull() ? QgsApplication::getThemeIcon(QStringLiteral("/mActionAddBasicShape.svg")) : group.icon;
		QString groupText = tr("Add %1").arg(group.name);
		if (mItemGroupToolButtons.contains(groupId))
		{
			groupButton = mItemGroupToolButtons.value(groupId);
		}
		else
		{
			QToolButton *groupToolButton = new QToolButton(mToolsToolbar);
			groupToolButton->setIcon(groupIcon);
			groupToolButton->setCheckable(true);
			groupToolButton->setPopupMode(QToolButton::InstantPopup);
			groupToolButton->setAutoRaise(true);
			groupToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
			groupToolButton->setToolTip(groupText);
			mToolsToolbar->addWidget(groupToolButton);
			mItemGroupToolButtons.insert(groupId, groupToolButton);
			groupButton = groupToolButton;
		}

		if (mItemGroupSubmenus.contains(groupId))
		{
			itemSubmenu = mItemGroupSubmenus.value(groupId);
		}
		else
		{
			QMenu *groupSubmenu = mItemMenu->addMenu(groupText);
			groupSubmenu->setIcon(groupIcon);
			mItemMenu->addMenu(groupSubmenu);
			mItemGroupSubmenus.insert(groupId, groupSubmenu);
			itemSubmenu = groupSubmenu;
		}
	}

	// update UI for new item type
	QAction *action = new QAction(tr("Add %1").arg(name), this);
	action->setToolTip(tr("Add %1").arg(name));
	action->setCheckable(true);
	action->setData(id);
	action->setIcon(QgsGui::layoutItemGuiRegistry()->itemMetadata(id)->creationIcon());

	mToolsActionGroup->addAction(action);
	if (itemSubmenu)
		itemSubmenu->addAction(action);
	else
		mItemMenu->addAction(action);

	if (groupButton)
		groupButton->addAction(action);
	else
		mToolsToolbar->addAction(action);

	connect(action, &QAction::triggered, this, [this, id, nodeBased]()
	{
		activateNewItemCreationTool(id, nodeBased);
	});
}

void GwmLayoutDesigner::activateNewItemCreationTool(int id, bool nodeBasedItem)
{
	if (!nodeBasedItem)
	{
		mAddItemTool->setItemMetadataId(id);
		if (mView)
			mView->setTool(mAddItemTool);
	}
	else
	{
		mAddNodeItemTool->setItemMetadataId(id);
		if (mView)
			mView->setTool(mAddNodeItemTool);
	}
}

void GwmLayoutDesigner::selectItems(const QList<QgsLayoutItem*>& items)
{
	for (QGraphicsItem *item : items)
	{
		if (item)
		{
			item->setSelected(true);
		}
	}

	//update item panel
	const QList<QgsLayoutItem *> selectedItemList = currentLayout()->selectedLayoutItems();
	if (!selectedItemList.isEmpty())
	{
		showItemOptions(selectedItemList.at(0));
	}
	else
	{
		showItemOptions(nullptr);
	}
}

void GwmLayoutDesigner::showItemOptions(QgsLayoutItem *item, bool bringPanelToFront)
{
	if (mBlockItemOptions)
		return;

	if (!item)
	{
		delete mItemPropertiesStack->takeMainPanel();
		return;
	}

	if (auto widget = qobject_cast<QgsLayoutItemBaseWidget *>(mItemPropertiesStack->mainPanel()))
	{
		if (widget->layoutObject() == item)
		{
			// already showing properties for this item - we don't want to create a new panel
			if (bringPanelToFront)
				mItemDock->setUserVisible(true);

			return;
		}
		else
		{
			// try to reuse
			if (widget->setItem(item))
			{
				if (bringPanelToFront)
					mItemDock->setUserVisible(true);

				return;
			}
		}
	}

	std::unique_ptr< QgsLayoutItemBaseWidget > widget(QgsGui::layoutItemGuiRegistry()->createItemWidget(item));
	delete mItemPropertiesStack->takeMainPanel();

	if (!widget)
		return;

	//widget->setDesignerInterface(iface());
	//widget->setReportTypeString(reportTypeString());
	widget->setMasterLayout(mMasterLayout);

	if (QgsLayoutPagePropertiesWidget *ppWidget = qobject_cast<QgsLayoutPagePropertiesWidget *>(widget.get()))
		connect(ppWidget, &QgsLayoutPagePropertiesWidget::pageOrientationChanged, this, &GwmLayoutDesigner::pageOrientationChanged);

	widget->setDockMode(true);
	connect(item, &QgsLayoutItem::destroyed, widget.get(), [this]
	{
		delete mItemPropertiesStack->takeMainPanel();
	});

	mItemPropertiesStack->setMainPanel(widget.release());
	if (bringPanelToFront)
		mItemDock->setUserVisible(true);
}

void GwmLayoutDesigner::loadPredefinedScalesFromProject()
{
	if (mLayout)
		mLayout->renderContext().setPredefinedScales(predefinedScales());
}

QVector<double> GwmLayoutDesigner::predefinedScales() const
{
	QgsProject *project = mMasterLayout->layoutProject();
	// first look at project's scales
	QVector< double > projectMapScales = project->viewSettings()->mapScales();
	bool hasProjectScales(project->viewSettings()->useProjectScales());
	if (!hasProjectScales || projectMapScales.isEmpty())
	{
		// default to global map tool scales
		QgsSettings settings;
		QString scalesStr(settings.value(QStringLiteral("Map/scales"), Qgis::defaultProjectScales()).toString());
		const QStringList scales = scalesStr.split(',');

		for (const QString &scale : scales)
		{
			QStringList parts(scale.split(':'));
			if (parts.size() == 2)
			{
				projectMapScales.push_back(parts[1].toDouble());
			}
		}
	}
	return projectMapScales;
}

void GwmLayoutDesigner::updateWindowTitle()
{
	QString title;
	if (mSectionTitle.isEmpty())
		title = mTitle;
	else
		title = QStringLiteral("%1 - %2").arg(mTitle, mSectionTitle);

	if (QgsProject::instance()->isDirty())
		title.prepend('*');

	setWindowTitle(title);
}

void GwmLayoutDesigner::activate()
{
	show();
	raise();
	setWindowState(windowState() & ~Qt::WindowMinimized);
	activateWindow();
}

void GwmLayoutDesigner::open()
{
	show();
	activate();
	if (mView)
	{
		mView->zoomFull(); // zoomFull() does not work properly until we have called show()
	}

	if (mMasterLayout && mMasterLayout->layoutType() == QgsMasterLayoutInterface::Report)
	{
		mReportDock->show();
		mReportDock->raise();
		mReportDock->setUserVisible(true);
	}
}

void GwmLayoutDesigner::paste()
{
	QPointF pt = mView->mapFromGlobal(QCursor::pos());
	//TODO - use a better way of determining whether paste was triggered by keystroke
	//or menu item
	QList< QgsLayoutItem * > items;
	if ((pt.x() < 0) || (pt.y() < 0))
	{
		//action likely triggered by menu, paste items in center of screen
		items = mView->pasteItems(QgsLayoutView::PasteModeCenter);
	}
	else
	{
		//action likely triggered by keystroke, paste items at cursor position
		items = mView->pasteItems(QgsLayoutView::PasteModeCursor);
	}

	whileBlocking(currentLayout())->deselectAll();
	selectItems(items);

	//switch back to select tool so that pasted items can be moved/resized (#8958)
	mView->setTool(mSelectTool);
}

void GwmLayoutDesigner::atlasPreviewTriggered(bool checked)
{
	QgsPrintLayout *printLayout = qobject_cast<QgsPrintLayout *>(mLayout);
	if (!printLayout)
		return;
	QgsLayoutAtlas *atlas = printLayout->atlas();

	//check if composition has an atlas map enabled
	if (checked && !atlas->enabled())
	{
		//no atlas current enabled
		mMessageBar->pushWarning(tr("Atlas"),
			tr("Atlas is not enabled for this layout!"));
		whileBlocking(mActionAtlasPreview)->setChecked(false);
		return;
	}

	//toggle other controls depending on whether atlas preview is active
	mActionAtlasFirst->setEnabled(checked);
	mActionAtlasLast->setEnabled(checked);
	mActionAtlasNext->setEnabled(checked);
	mActionAtlasPrev->setEnabled(checked);
	mAtlasPageComboBox->setEnabled(checked);

	if (checked)
	{
		loadPredefinedScalesFromProject();
	}

	if (checked)
	{
		if (!atlas->beginRender())
		{
			atlas->endRender();
			//something went wrong, e.g., no matching features
			mMessageBar->pushWarning(tr("Atlas"), tr("No matching atlas features found!"));
			mActionAtlasPreview->blockSignals(true);
			mActionAtlasPreview->setChecked(false);
			mActionAtlasFirst->setEnabled(false);
			mActionAtlasLast->setEnabled(false);
			mActionAtlasNext->setEnabled(false);
			mActionAtlasPrev->setEnabled(false);
			mAtlasPageComboBox->setEnabled(false);
			mActionAtlasPreview->blockSignals(false);
		}
		else
		{
			GwmApp::Instance()->mapCanvas()->stopRendering();
			atlas->first();
		}
	}
	else
	{
		atlas->endRender();
		mView->setSectionLabel(QString());
	}
}

void GwmLayoutDesigner::atlasPageComboEditingFinished()
{
	QString text = mAtlasPageComboBox->lineEdit()->text();

	//find matching record in combo box
	int page = -1; //note - first page starts at 1, not 0
	for (int i = 0; i < mAtlasPageComboBox->count(); ++i)
	{
		if (text.compare(mAtlasPageComboBox->itemData(i, Qt::UserRole + 1).toString(), Qt::CaseInsensitive) == 0
			|| text.compare(mAtlasPageComboBox->itemData(i, Qt::UserRole + 2).toString(), Qt::CaseInsensitive) == 0
			|| QString::number(i + 1) == text)
		{
			page = i + 1;
			break;
		}
	}
	bool ok = (page > 0);

	QgsPrintLayout *printLayout = qobject_cast<QgsPrintLayout *>(mLayout);
	if (!printLayout)
		return;
	QgsLayoutAtlas *atlas = printLayout->atlas();

	if (!ok || page > atlas->count() || page < 1)
	{
		whileBlocking(mAtlasPageComboBox)->setCurrentIndex(atlas->currentFeatureNumber());
	}
	else if (page != atlas->currentFeatureNumber() + 1)
	{
		GwmApp::Instance()->mapCanvas()->stopRendering();
		loadPredefinedScalesFromProject();
		atlas->seekTo(page - 1);
	}
}

void GwmLayoutDesigner::refreshLayout()
{
	if (!currentLayout())
	{
		return;
	}

	//refresh atlas feature first, to force an update of feature
	//in case feature attributes or geometry has changed
	if (QgsLayoutAtlas *printAtlas = atlas())
	{
		if (printAtlas->enabled() && mActionAtlasPreview->isChecked())
		{
			//block signals from atlas, since the later call to mComposition->refreshItems() will
			//also trigger items to refresh atlas dependent properties
			whileBlocking(printAtlas)->refreshCurrentFeature();
		}
	}

	currentLayout()->refresh();
}

void GwmLayoutDesigner::statusZoomCombo_currentIndexChanged(int index)
{
	QVariant data = mStatusZoomCombo->itemData(index);
	if (data.toInt() == FIT_LAYOUT)
	{
		mView->zoomFull();
	}
	else if (data.toInt() == FIT_LAYOUT_WIDTH)
	{
		mView->zoomWidth();
	}
	else
	{
		double selectedZoom = data.toDouble();
		if (mView)
		{
			mView->setZoomLevel(selectedZoom);
			//update zoom combobox text for correct format (one decimal place, trailing % sign)
			whileBlocking(mStatusZoomCombo)->lineEdit()->setText(tr("%1%").arg(selectedZoom * 100.0, 0, 'f', 1));
		}
	}
}

void GwmLayoutDesigner::statusZoomCombo_zoomEntered()
{
	if (!mView)
	{
		return;
	}

	//need to remove spaces and "%" characters from input text
	QString zoom = mStatusZoomCombo->currentText().remove(QChar('%')).trimmed();
	mView->setZoomLevel(zoom.toDouble() / 100);
}

void GwmLayoutDesigner::sliderZoomChanged(int value)
{
	mView->setZoomLevel(value / 100.0);
}

void GwmLayoutDesigner::updateStatusZoom()
{
	if (!currentLayout())
		return;

	double zoomLevel = 0;
	if (currentLayout()->units() == QgsUnitTypes::LayoutPixels)
	{
		zoomLevel = mView->transform().m11() * 100;
	}
	else
	{
		double dpi = QgsApplication::desktop()->logicalDpiX();
		//monitor dpi is not always correct - so make sure the value is sane
		if ((dpi < 60) || (dpi > 1200))
			dpi = 72;

		//pixel width for 1mm on screen
		double scale100 = dpi / 25.4;
		scale100 = currentLayout()->convertFromLayoutUnits(scale100, QgsUnitTypes::LayoutMillimeters).length();
		//current zoomLevel
		zoomLevel = mView->transform().m11() * 100 / scale100;
	}
	whileBlocking(mStatusZoomCombo)->lineEdit()->setText(tr("%1%").arg(zoomLevel, 0, 'f', 1));
	whileBlocking(mStatusZoomSlider)->setValue(static_cast<int>(zoomLevel));
}

void GwmLayoutDesigner::updateStatusCursorPos(QPointF position)
{
	if (!mView->currentLayout())
	{
		return;
	}

	//convert cursor position to position on current page
	QPointF pagePosition = mLayout->pageCollection()->positionOnPage(position);
	int currentPage = mLayout->pageCollection()->pageNumberForPoint(position);

	QString unit = QgsUnitTypes::toAbbreviatedString(mLayout->units());
	mStatusCursorXLabel->setText(tr("x: %1 %2").arg(pagePosition.x()).arg(unit));
	mStatusCursorYLabel->setText(tr("y: %1 %2").arg(pagePosition.y()).arg(unit));
	mStatusCursorPageLabel->setText(tr("page: %1").arg(currentPage + 1));
}

void GwmLayoutDesigner::setMasterLayout(QgsMasterLayoutInterface * layout)
{
	mMasterLayout = layout;

	QObject *obj = dynamic_cast<QObject *>(mMasterLayout);
	if (obj)
		connect(obj, &QObject::destroyed, this, [=]
	{
		this->close();
	});

	setTitle(mMasterLayout->name());

	if (QgsPrintLayout *l = dynamic_cast<QgsPrintLayout *>(layout))
	{
		connect(l, &QgsPrintLayout::nameChanged, this, &GwmLayoutDesigner::setTitle);
		setCurrentLayout(l);
	}
	else if (QgsReport *r = dynamic_cast<QgsReport *>(layout))
	{
		connect(r, &QgsReport::nameChanged, this, &GwmLayoutDesigner::setTitle);
	}

	if (dynamic_cast<QgsPrintLayout *>(layout))
	{
		createAtlasWidget();
	}
	else
	{
		// ideally we'd only create mAtlasDock in createAtlasWidget() -
		// but if we do that, then it's always brought to the focus
		// in tab widgets
		mAtlasDock->hide();
		mPanelsMenu->removeAction(mAtlasDock->toggleViewAction());
		delete mMenuAtlas;
		mMenuAtlas = nullptr;
		mAtlasToolbar->hide();
		mToolbarMenu->removeAction(mAtlasToolbar->toggleViewAction());
	}

	if (dynamic_cast<QgsReport *>(layout))
	{
		createReportWidget();
		mLayoutMenu->removeAction(mActionExportAsPDF);
		mLayoutMenu->removeAction(mActionExportAsSVG);
		mLayoutMenu->removeAction(mActionExportAsImage);
		mLayoutMenu->removeAction(mActionPrint);
		mLayoutToolbar->removeAction(mActionExportAsPDF);
		mLayoutToolbar->removeAction(mActionExportAsSVG);
		mLayoutToolbar->removeAction(mActionExportAsImage);
		mLayoutToolbar->removeAction(mActionPrint);
		// remove useless dangling separator
		mLayoutToolbar->removeAction(mLayoutToolbar->actions().constLast());
	}
	else
	{
		// ideally we'd only create mReportDock in createReportWidget() -
		// but if we do that, then it's always brought to the focus
		// in tab widgets
		mReportDock->hide();
		mPanelsMenu->removeAction(mReportDock->toggleViewAction());
		delete mMenuReport;
		mMenuReport = nullptr;
		mReportToolbar->hide();
		mToolbarMenu->removeAction(mReportToolbar->toggleViewAction());
	}

	updateActionNames(mMasterLayout->layoutType());
}

void GwmLayoutDesigner::createAtlasWidget()
{
	QgsPrintLayout *printLayout = dynamic_cast<QgsPrintLayout *>(mMasterLayout);
	if (!printLayout)
		return;

	QgsLayoutAtlas *atlas = printLayout->atlas();
	QgsLayoutAtlasWidget *atlasWidget = new QgsLayoutAtlasWidget(mAtlasDock, printLayout);
	//atlasWidget->setMessageBar(mMessageBar);
	mAtlasDock->setWidget(atlasWidget);

	mAtlasToolbar->show();
	mPanelsMenu->addAction(mAtlasDock->toggleViewAction());

	//connect(atlas, &QgsLayoutAtlas::messagePushed, mStatusBar, [=](const QString & message)
	//{
	//	mStatusBar->showMessage(message);
	//});
	//connect(atlas, &QgsLayoutAtlas::toggled, this, &GwmLayoutDesigner::toggleAtlasControls);
	//connect(atlas, &QgsLayoutAtlas::toggled, this, &GwmLayoutDesigner::refreshLayout);
	//connect(atlas, &QgsLayoutAtlas::numberFeaturesChanged, this, &GwmLayoutDesigner::updateAtlasPageComboBox);
	//connect(atlas, &QgsLayoutAtlas::featureChanged, this, &GwmLayoutDesigner::atlasFeatureChanged);
	//toggleAtlasControls(atlas->enabled() && atlas->coverageLayer());
}

void GwmLayoutDesigner::createReportWidget()
{
	QgsReport *report = dynamic_cast<QgsReport *>(mMasterLayout);
	//QgsReportOrganizerWidget *reportWidget = new QgsReportOrganizerWidget(mReportDock, this, report);
	//reportWidget->setMessageBar(mMessageBar);
	//mReportDock->setWidget(reportWidget);
	mReportToolbar->show();
	mPanelsMenu->addAction(mReportDock->toggleViewAction());
}

void GwmLayoutDesigner::updateActionNames(QgsMasterLayoutInterface::Type type)
{
	switch (type)
	{
		case QgsMasterLayoutInterface::PrintLayout:
			mActionDuplicateLayout->setText(tr("&Duplicate Layout¡­"));
			mActionDuplicateLayout->setToolTip(tr("Duplicate layout"));
			mActionDuplicateLayout->setIcon(QgsApplication::getThemeIcon(QStringLiteral("mActionDuplicateLayout.svg")));
			mActionRemoveLayout->setText(tr("Delete Layout¡­"));
			mActionRemoveLayout->setToolTip(tr("Delete layout"));
			mActionRenameLayout->setText(tr("Rename Layout¡­"));
			mActionRenameLayout->setToolTip(tr("Rename layout"));
			mActionNewLayout->setText(tr("New Layout¡­"));
			mActionNewLayout->setToolTip(tr("New layout"));
			mActionNewLayout->setIcon(QgsApplication::getThemeIcon(QStringLiteral("mActionNewLayout.svg")));
			break;

		case QgsMasterLayoutInterface::Report:
			mActionDuplicateLayout->setText(tr("&Duplicate Report¡­"));
			mActionDuplicateLayout->setToolTip(tr("Duplicate report"));
			mActionDuplicateLayout->setIcon(QgsApplication::getThemeIcon(QStringLiteral("mActionDuplicateLayout.svg")));
			mActionRemoveLayout->setText(tr("Delete Report¡­"));
			mActionRemoveLayout->setToolTip(tr("Delete report"));
			mActionRenameLayout->setText(tr("Rename Report¡­"));
			mActionRenameLayout->setToolTip(tr("Rename report"));
			mActionNewLayout->setText(tr("New Report¡­"));
			mActionNewLayout->setToolTip(tr("New report"));
			mActionNewLayout->setIcon(QgsApplication::getThemeIcon(QStringLiteral("mActionNewReport.svg")));
			break;
	}
}

QgsLayoutAtlas * GwmLayoutDesigner::atlas()
{
	QgsPrintLayout *layout = qobject_cast<QgsPrintLayout *>(mLayout);
	if (!layout)
		return nullptr;
	return layout->atlas();
}

void GwmLayoutDesigner::setCurrentLayout(QgsLayout * layout)
{
	if (!layout)
	{
		toggleActions(false);
		mLayout = nullptr;
	}
	else
	{
		if (mLayout)
		{
			//disconnect(mLayout, &QgsLayout::backgroundTaskCountChanged, this, &QgsLayoutDesignerDialog::backgroundTaskCountChanged);
		}

		layout->deselectAll();
		mLayout = layout;

		mView->setCurrentLayout(layout);

		// add undo/redo actions which apply to the correct layout undo stack
		delete mUndoAction;
		delete mRedoAction;
		mUndoAction = layout->undoStack()->stack()->createUndoAction(this);
		mUndoAction->setIcon(QgsApplication::getThemeIcon(QStringLiteral("/mActionUndo.svg")));
		mUndoAction->setShortcuts(QKeySequence::Undo);
		mRedoAction = layout->undoStack()->stack()->createRedoAction(this);
		mRedoAction->setIcon(QgsApplication::getThemeIcon(QStringLiteral("/mActionRedo.svg")));
		mRedoAction->setShortcuts(QKeySequence::Redo);
		menuEdit->insertAction(menuEdit->actions().at(0), mRedoAction);
		menuEdit->insertAction(mRedoAction, mUndoAction);
		mLayoutToolbar->addAction(mUndoAction);
		mLayoutToolbar->addAction(mRedoAction);

		//connect(mLayout->undoStack(), &QgsLayoutUndoStack::undoRedoOccurredForItems, this, &GwmLayoutDesigner::undoRedoOccurredForItems);
		connect(mActionClearGuides, &QAction::triggered, &mLayout->guides(), [=]
		{
			mLayout->guides().clear();
		});

		mActionShowGrid->setChecked(mLayout->renderContext().gridVisible());
		mActionSnapGrid->setChecked(mLayout->snapper().snapToGrid());
		mActionShowGuides->setChecked(mLayout->guides().visible());
		mActionSnapGuides->setChecked(mLayout->snapper().snapToGuides());
		mActionSmartGuides->setChecked(mLayout->snapper().snapToItems());
		mActionShowBoxes->setChecked(mLayout->renderContext().boundingBoxesVisible());
		mActionShowPage->setChecked(mLayout->renderContext().pagesVisible());

		mUndoView->setStack(mLayout->undoStack()->stack());

		mSelectTool->setLayout(layout);
		mItemsTreeView->setCurrentLayout(mLayout);
#ifdef ENABLE_MODELTEST
		new ModelTest(mLayout->itemsModel(), this);
#endif

		//connect(mLayout, &QgsLayout::backgroundTaskCountChanged, this, &GwmLayoutDesigner::backgroundTaskCountChanged);

		//createLayoutPropertiesWidget();
		toggleActions(true);
	}
}

void GwmLayoutDesigner::toggleActions(bool layoutAvailable)
{
	mActionPan->setEnabled(layoutAvailable);
	mActionZoomTool->setEnabled(layoutAvailable);
	mActionSelectMoveItem->setEnabled(layoutAvailable);
	mActionZoomAll->setEnabled(layoutAvailable);
	mActionZoomIn->setEnabled(layoutAvailable);
	mActionZoomOut->setEnabled(layoutAvailable);
	mActionZoomActual->setEnabled(layoutAvailable);
	mActionZoomToWidth->setEnabled(layoutAvailable);
	mActionAddPages->setEnabled(layoutAvailable);
	mActionShowGrid->setEnabled(layoutAvailable);
	mActionSnapGrid->setEnabled(layoutAvailable);
	mActionShowGuides->setEnabled(layoutAvailable);
	mActionSnapGuides->setEnabled(layoutAvailable);
	mActionClearGuides->setEnabled(layoutAvailable);
	mActionLayoutProperties->setEnabled(layoutAvailable);
	mActionShowBoxes->setEnabled(layoutAvailable);
	mActionSmartGuides->setEnabled(layoutAvailable);
	mActionDeselectAll->setEnabled(layoutAvailable);
	mActionSelectAll->setEnabled(layoutAvailable);
	mActionInvertSelection->setEnabled(layoutAvailable);
	mActionSelectNextBelow->setEnabled(layoutAvailable);
	mActionSelectNextAbove->setEnabled(layoutAvailable);
	mActionLockItems->setEnabled(layoutAvailable);
	mActionUnlockAll->setEnabled(layoutAvailable);
	mActionRaiseItems->setEnabled(layoutAvailable);
	mActionLowerItems->setEnabled(layoutAvailable);
	mActionMoveItemsToTop->setEnabled(layoutAvailable);
	mActionMoveItemsToBottom->setEnabled(layoutAvailable);
	mActionAlignLeft->setEnabled(layoutAvailable);
	mActionAlignHCenter->setEnabled(layoutAvailable);
	mActionAlignRight->setEnabled(layoutAvailable);
	mActionAlignTop->setEnabled(layoutAvailable);
	mActionAlignVCenter->setEnabled(layoutAvailable);
	mActionAlignBottom->setEnabled(layoutAvailable);
	mActionDistributeLeft->setEnabled(layoutAvailable);
	mActionDistributeHCenter->setEnabled(layoutAvailable);
	mActionDistributeHSpace->setEnabled(layoutAvailable);
	mActionDistributeRight->setEnabled(layoutAvailable);
	mActionDistributeTop->setEnabled(layoutAvailable);
	mActionDistributeVCenter->setEnabled(layoutAvailable);
	mActionDistributeVSpace->setEnabled(layoutAvailable);
	mActionDistributeBottom->setEnabled(layoutAvailable);
	mActionResizeNarrowest->setEnabled(layoutAvailable);
	mActionResizeWidest->setEnabled(layoutAvailable);
	mActionResizeShortest->setEnabled(layoutAvailable);
	mActionResizeTallest->setEnabled(layoutAvailable);
	mActionDeleteSelection->setEnabled(layoutAvailable);
	mActionResizeToSquare->setEnabled(layoutAvailable);
	mActionShowPage->setEnabled(layoutAvailable);
	mActionGroupItems->setEnabled(layoutAvailable);
	mActionUngroupItems->setEnabled(layoutAvailable);
	mActionRefreshView->setEnabled(layoutAvailable);
	mActionEditNodesItem->setEnabled(layoutAvailable);
	mActionMoveItemContent->setEnabled(layoutAvailable);
	mActionPasteInPlace->setEnabled(layoutAvailable);
	mActionSaveAsTemplate->setEnabled(layoutAvailable);
	mActionLoadFromTemplate->setEnabled(layoutAvailable);
	mActionExportAsImage->setEnabled(layoutAvailable);
	mActionExportAsPDF->setEnabled(layoutAvailable);
	mActionExportAsSVG->setEnabled(layoutAvailable);
	mActionPrint->setEnabled(layoutAvailable);
	mActionCut->setEnabled(layoutAvailable);
	mActionCopy->setEnabled(layoutAvailable);
	mActionPaste->setEnabled(layoutAvailable);
	menuAlign_Items->setEnabled(layoutAvailable);
	menu_Distribute_Items->setEnabled(layoutAvailable);
	menuResize->setEnabled(layoutAvailable);

	const QList<QAction *> itemActions = mToolsActionGroup->actions();
	for (QAction *action : itemActions)
	{
		action->setEnabled(layoutAvailable);
	}
	for (auto it = mItemGroupSubmenus.constBegin(); it != mItemGroupSubmenus.constEnd(); ++it)
	{
		it.value()->setEnabled(layoutAvailable);
	}
	for (auto it = mItemGroupToolButtons.constBegin(); it != mItemGroupToolButtons.constEnd(); ++it)
	{
		it.value()->setEnabled(layoutAvailable);
	}
}

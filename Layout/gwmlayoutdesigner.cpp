#include "gwmlayoutdesigner.h"

#include <QToolButton>

#include <qgsgui.h>
#include <qgslayoutitemguiregistry.h>
#include <qgslayoutpagepropertieswidget.h>
#include <qgslayoutviewtooladditem.h>
#include <qgslayoutviewtooladdnodeitem.h>
#include <qgslayoutview.h>
#include <qgslayoutruler.h>


bool GwmLayoutDesigner::sInitializedRegistry = false;


GwmLayoutDesigner::GwmLayoutDesigner(QWidget *parent)
	: QMainWindow(parent)
	, mToolsActionGroup(new QActionGroup(this))
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

void GwmLayoutDesigner::activate()
{
	show();
	raise();
	setWindowState(windowState() & ~Qt::WindowMinimized);
	activateWindow();
}

#ifndef GWMLAYOUTDESIGNER_H
#define GWMLAYOUTDESIGNER_H


#include <QMainWindow>
#include "ui_gwmlayoutdesigner.h"

class QgsLayoutView;
class QgsLayoutRuler;
class QgsLayoutViewToolAddItem;
class QgsLayoutViewToolAddNodeItem;

class GwmLayoutDesigner : public QMainWindow, public Ui::GwmLayoutDesigner
{
	Q_OBJECT

private:
	static bool sInitializedRegistry;

public:
	GwmLayoutDesigner(QWidget *parent = nullptr);
	~GwmLayoutDesigner();

public:
	void activate();


private:
	void initializeRegistry();

	void showRulers(bool visible);

	void createLayoutView();

	void itemTypeAdded(int id);
	void activateNewItemCreationTool(int id, bool nodeBasedItem);


private:
	QgsLayoutView* mView = nullptr;
	QgsLayoutRuler *mHorizontalRuler = nullptr;
	QgsLayoutRuler *mVerticalRuler = nullptr;
	QWidget *mRulerLayoutFix = nullptr;

	QgsLayoutViewToolAddItem *mAddItemTool = nullptr;
	QgsLayoutViewToolAddNodeItem *mAddNodeItemTool = nullptr;

	QActionGroup *mToolsActionGroup = nullptr;
	QMap< QString, QToolButton * > mItemGroupToolButtons;
	QMap< QString, QMenu * > mItemGroupSubmenus;
};


#endif // !GWMLAYOUTDESIGNER_H

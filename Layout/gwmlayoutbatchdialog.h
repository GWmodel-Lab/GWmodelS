#pragma once

#include <QDialog>
#include <QAbstractItemModel>
namespace Ui { class GwmLayoutBatchDialog; };

class QgsLayoutManagerModel;
class QgsLayoutManagerProxyModel;
class GwmLayerItemModel;
class QItemSelectionModel;



class GwmLayoutBatchDialog : public QDialog
{
	Q_OBJECT

public:
	GwmLayoutBatchDialog(QWidget *parent = Q_NULLPTR);
	~GwmLayoutBatchDialog();

private:
	Ui::GwmLayoutBatchDialog *ui;

	QgsLayoutManagerModel *mLayoutModel = nullptr;
	QgsLayoutManagerProxyModel *mLayoutProxyModel = nullptr;
	
	GwmLayerItemModel* mLayerModel = nullptr;
	QItemSelectionModel* mLayerSelectionModel = nullptr;
};

#pragma once

#include <QDialog>
#include <QAbstractItemModel>
namespace Ui { class GwmLayoutBatchDialog; };

class QgsLayoutManagerModel;
class QgsLayoutManagerProxyModel;
class GwmLayoutBatchLayerListModel;
class QItemSelectionModel;
class GwmLayoutBatchFieldListModel;


class GwmLayoutBatchDialog : public QDialog
{
	Q_OBJECT

public:
	GwmLayoutBatchDialog(QWidget *parent = Q_NULLPTR);
	~GwmLayoutBatchDialog();

private slots:
    void onLayerSelectionModelCurrentChanged(const QModelIndex& current, const QModelIndex& previous);

private:
	Ui::GwmLayoutBatchDialog *ui;

	QgsLayoutManagerModel *mLayoutModel = nullptr;
	QgsLayoutManagerProxyModel *mLayoutProxyModel = nullptr;
	
	GwmLayoutBatchLayerListModel* mLayerModel = nullptr;
	QItemSelectionModel* mLayerSelectionModel = nullptr;

    GwmLayoutBatchFieldListModel* mFieldModel = nullptr;
};

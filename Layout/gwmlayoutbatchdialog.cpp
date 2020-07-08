#include "gwmlayoutbatchdialog.h"
#include "ui_gwmlayoutbatchdialog.h"

#include <qitemselectionmodel.h>

#include <qgsproject.h>
#include <qgslayoutmanager.h>

#include "gwmapp.h"

GwmLayoutBatchDialog::GwmLayoutBatchDialog(QWidget *parent)
	: QDialog(parent)
	, mLayerModel(GwmApp::Instance()->mapModel())
{
	ui = new Ui::GwmLayoutBatchDialog();
	ui->setupUi(this);

	mLayoutModel = new QgsLayoutManagerModel(QgsProject::instance()->layoutManager(), this);
	mLayoutProxyModel = new QgsLayoutManagerProxyModel(ui->lsvLayout);
	mLayoutProxyModel->setSourceModel(mLayoutModel);
	ui->lsvLayout->setModel(mLayoutProxyModel);

	ui->lsvLayer->setModel(mLayerModel);
	mLayerSelectionModel = new QItemSelectionModel(mLayerModel);
	ui->lsvLayer->setSelectionModel(mLayerSelectionModel);


}

GwmLayoutBatchDialog::~GwmLayoutBatchDialog()
{
	delete ui;
}
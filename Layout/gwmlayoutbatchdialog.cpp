#include "gwmlayoutbatchdialog.h"
#include "ui_gwmlayoutbatchdialog.h"

#include <qitemselectionmodel.h>

#include <qgsproject.h>
#include <qgslayoutmanager.h>

#include "gwmapp.h"
#include "gwmlayoutbatchlayerlistmodel.h"
#include "gwmlayoutbatchfieldlistmodel.h"

GwmLayoutBatchDialog::GwmLayoutBatchDialog(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::GwmLayoutBatchDialog();
	ui->setupUi(this);

	mLayoutModel = new QgsLayoutManagerModel(QgsProject::instance()->layoutManager(), this);
	mLayoutProxyModel = new QgsLayoutManagerProxyModel(ui->lsvLayout);
	mLayoutProxyModel->setSourceModel(mLayoutModel);
	ui->lsvLayout->setModel(mLayoutProxyModel);

    mLayerModel = new GwmLayoutBatchLayerListModel(GwmApp::Instance()->mapModel(), this);
	ui->lsvLayer->setModel(mLayerModel);
	mLayerSelectionModel = new QItemSelectionModel(mLayerModel);
	ui->lsvLayer->setSelectionModel(mLayerSelectionModel);
    connect(mLayerSelectionModel, &QItemSelectionModel::currentChanged, this, &GwmLayoutBatchDialog::onLayerSelectionModelCurrentChanged);
}

GwmLayoutBatchDialog::~GwmLayoutBatchDialog()
{
    delete ui;
}

void GwmLayoutBatchDialog::onLayerSelectionModelCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (mFieldModel) delete mFieldModel;
    QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>(mLayerModel->layerFromIndex(current));
    if (layer)
    {
        mFieldModel = new GwmLayoutBatchFieldListModel(layer, this);
        ui->lsvField->setModel(mFieldModel);
    }
}

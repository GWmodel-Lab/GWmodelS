#include "gwmlayoutbatchdialog.h"
#include "ui_gwmlayoutbatchdialog.h"

#include <qitemselectionmodel.h>

#include <qgsproject.h>
#include <qgslayoutmanager.h>

#include "gwmapp.h"
#include "gwmlayoutbatchlayerlistmodel.h"
#include "gwmlayoutbatchfieldlistmodel.h"
#include "gwmlayoutbatchconfigurationmodel.h"
#include "gwmlayoutbatchconfigurationdelegate.h"
#include "symbolwindow/gwmsymbolwindow.h"

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
    connect(mLayerModel, &QAbstractItemModel::dataChanged, this, &GwmLayoutBatchDialog::onLayerModelDataChanged);

    mConfigurationModel = new GwmLayoutBatchConfigurationModel(this);
    mConfigurationSelectionModel = new QItemSelectionModel(mConfigurationModel);
    ui->trvSymbology->setModel(mConfigurationModel);
    ui->trvSymbology->setSelectionModel(mConfigurationSelectionModel);
    mConfiguationDelegate = new GwmLayoutBatchConfigurationDelegate();
    ui->trvSymbology->setItemDelegate(mConfiguationDelegate);

//    connect(ui->btnEditSymbol, &QToolButton::clicked, this, &GwmLayoutBatchDialog::on_btnEditSymbol_clicked);
}

GwmLayoutBatchDialog::~GwmLayoutBatchDialog()
{
    delete ui;
}

void GwmLayoutBatchDialog::onLayerSelectionModelCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (mFieldModel)
    {
        disconnect(mFieldModel, &QAbstractItemModel::dataChanged, this, &GwmLayoutBatchDialog::onFieldModelDataChanged);
        delete mFieldModel;
    }
    QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>(mLayerModel->layerFromIndex(current));
    if (layer)
    {
        mFieldModel = new GwmLayoutBatchFieldListModel(layer, this);
        ui->lsvField->setModel(mFieldModel);
        connect(mFieldModel, &QAbstractItemModel::dataChanged, this, &GwmLayoutBatchDialog::onFieldModelDataChanged);
    }
}

void GwmLayoutBatchDialog::onLayerModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    if (topLeft == bottomRight)
    {
        for (int role : roles)
        {
            if (role == Qt::CheckStateRole)
            {
                bool checked = mLayerModel->data(topLeft, Qt::CheckStateRole).toInt() == Qt::Checked;
                if (checked)
                {
                    QgsVectorLayer* layer = mLayerModel->layerFromIndex(topLeft);
                    int layerCheckedIndex = mLayerModel->checkedIndex(layer);
                    mConfigurationModel->insertLayer(layerCheckedIndex, layer);
                }
                else
                {
                    QgsVectorLayer* layer = mLayerModel->layerFromIndex(topLeft);
                    mConfigurationModel->removeLayer(layer);
                }
            }
        }
    }
}

void GwmLayoutBatchDialog::onFieldModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    if (topLeft == bottomRight)
    {
        QgsFieldCheckable* field = mFieldModel->itemFromIndex(topLeft);
        for (int role : roles)
        {
            if (role == Qt::CheckStateRole)
            {
                QModelIndex layerSelectedIndex = mLayerSelectionModel->currentIndex();
                if (layerSelectedIndex.isValid())
                {
                    QgsVectorLayer* layer = mLayerModel->layerFromIndex(layerSelectedIndex);
                    QModelIndex layerItemIndex = mConfigurationModel->indexFromLayer(layer);
                    bool checked = mFieldModel->data(topLeft, Qt::CheckStateRole).toInt() == Qt::Checked;
                    if (checked)
                    {
                        int fieldCheckedIndex = mFieldModel->checkedIndex(field);
                        mConfigurationModel->insertField(fieldCheckedIndex, *field, layerItemIndex);
                    }
                    else
                    {
                        mConfigurationModel->removeField(*field, layerItemIndex);
                    }
                }
            }
        }
    }
}

void GwmLayoutBatchDialog::on_btnEditSymbol_clicked()
{
    const QModelIndex& index = mConfigurationSelectionModel->currentIndex();
    GwmLayoutBatchConfigurationItem* item = mConfigurationModel->itemFromIndex(index);
    if (item->type() == GwmLayoutBatchConfigurationItem::Field)
    {
        GwmLayoutBatchConfigurationItemField* fieldItem = static_cast<GwmLayoutBatchConfigurationItemField*>(item);
        GwmLayoutBatchConfigurationItemLayer* layerItem = static_cast<GwmLayoutBatchConfigurationItemLayer*>(fieldItem->parent());
        GwmSymbolWindow* symbolWindow = new GwmSymbolWindow(layerItem->layer());
        symbolWindow->show();
    }
}

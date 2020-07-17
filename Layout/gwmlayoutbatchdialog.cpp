#include "gwmlayoutbatchdialog.h"
#include "ui_gwmlayoutbatchdialog.h"

#include <qitemselectionmodel.h>
#include <QDebug>

#include <qgsproject.h>
#include <qgslayoutmanager.h>
#include <qgslayoutimageexportoptionsdialog.h>
#include <qgslayoutpagecollection.h>
#include <qgsprojectviewsettings.h>
#include <qgsprintlayout.h>
#include <qgsproxyprogresstask.h>

#include "gwmapp.h"
#include "gwmlayoutbatchlayerlistmodel.h"
#include "gwmlayoutbatchfieldlistmodel.h"
#include "gwmlayoutbatchconfigurationmodel.h"
#include "gwmlayoutbatchconfigurationdelegate.h"
#include "symbolwindow/gwmsymboleditordialog.h"

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
        GwmSymbolEditorDialog* editor = new GwmSymbolEditorDialog(layerItem->layer(), this);
        if (editor->exec() == QDialog::Accepted)
        {
            QgsFeatureRenderer* renderer = editor->selectedRenderer();
            mConfigurationModel->setFieldRenderer(index, renderer);
        }
        delete editor;
    }
}

void GwmLayoutBatchDialog::on_btnBox_accepted()
{

}

void GwmLayoutBatchDialog::on_btnExportRaster_clicked()
{
    auto selectLayoutIndexList = ui->lsvLayout->selectionModel()->selectedRows();
    if (selectLayoutIndexList.isEmpty())
    {
        return;
    }

    QgsMasterLayoutInterface* currentLayout = mLayoutModel->layoutFromIndex(mLayoutProxyModel->mapToSource(selectLayoutIndexList.first()));
    if (!currentLayout)
        return;

    GwmLayoutDesigner* designer = new GwmLayoutDesigner();
    designer->setMasterLayout(currentLayout);

    QString directory = ui->txtOutputDirectory->text();
    QString filenameTemplate = ui->txtFileNameTemplate->text();

    if (QgsPrintLayout* layout = dynamic_cast<QgsPrintLayout*>(currentLayout))
    {
        QVector<double> scales = predefinedScales(currentLayout);
        QgsLayoutExporter::ImageExportSettings settings;
        QSize imageSize;
        if (getRasterExportSettings(layout, settings, imageSize, scales))
        {
            // 批量导出
            auto rootItem = static_cast<GwmLayoutBatchConfigurationItemRoot*>(mConfigurationModel->itemFromIndex(QModelIndex()));
            if (rootItem)
            {
                // 先移除所有选中的图层
                QList<GwmLayoutBatchConfigurationItemLayer*> layerItemList = rootItem->layerList();
                QList<QgsMapLayer*> currentMapList0 = GwmApp::Instance()->mapLayerList();
                QList<QgsMapLayer*> currentMapList = GwmApp::Instance()->mapLayerList();
                for (auto item : layerItemList)
                {
                    currentMapList.removeOne(item->layer());
                }
                GwmApp::Instance()->mapCanvas()->setLayers(currentMapList);
                GwmApp::Instance()->mapCanvas()->refresh();
                // 逐个添加图层
                for (auto layerItem : layerItemList)
                {
                    QgsVectorLayer* layer = layerItem->layer();
                    currentMapList.append(layer);
                    GwmApp::Instance()->mapCanvas()->setLayers(currentMapList);
                    GwmApp::Instance()->mapCanvas()->refresh();
                    for (auto fieldItem : layerItem->fieldList())
                    {
                        // 修改符号
                        QgsFeatureRenderer* renderer0 = layer->renderer()->clone();
                        QgsFeatureRenderer* renderer = fieldItem->renderer();
                        layer->setRenderer(renderer->clone());
                        GwmApp::Instance()->mapCanvas()->refresh();
                        // 导出图层
                        QString filename = filenameTemplate.replace("%layer%", layerItem->name());
                        filename = filename.replace("%field%", fieldItem->name());
                        QgsProxyProgressTask* proxyTask = new QgsProxyProgressTask(tr("Exporting %1").arg(filename));
                        QgsApplication::taskManager()->addTask(proxyTask);
                        layout->refresh();
                        QgsLayoutExporter exporter(layout);
                        QFileInfo outputFile(directory, filename);
                        QgsLayoutExporter::ExportResult result = exporter.exportToImage(outputFile.filePath(), settings);
                        proxyTask->finalize(result == QgsLayoutExporter::Success);
                        switch (result) {
                        case QgsLayoutExporter::Success:
                            qDebug() << "[Export layout] Successfully exported layout to"
                                     << outputFile.filePath();
                            break;
                        case QgsLayoutExporter::PrintError:
                        case QgsLayoutExporter::SvgLayerError:
                        case QgsLayoutExporter::IteratorError:
                        case QgsLayoutExporter::Canceled:
                            // no meaning for raster exports, will not be encountered
                            break;
                        case QgsLayoutExporter::FileError:
                            qDebug() << "[Image Export Error] Cannot write to"
                                     << outputFile.filePath()
                                     << ". This file may be open in another application.";
                            break;
                        case QgsLayoutExporter::MemoryError:
                            qDebug() << "[Image Export Error] Trying to create image"
                                     << outputFile.filePath()
                                     << "resulted in a memory overflow."
                                     << "Please try a lower resolution or a smaller paper size.";
                            break;
                        default:
                            break;
                        }
                        // 恢复符号
                        layer->setRenderer(renderer0);
                    }
                    currentMapList.removeOne(layer);
                    GwmApp::Instance()->mapCanvas()->setLayers(currentMapList0);
                    GwmApp::Instance()->mapCanvas()->refresh();
                }
                // 恢复所有图层
                GwmApp::Instance()->mapCanvas()->setLayers(currentMapList0);
                GwmApp::Instance()->mapCanvas()->refresh();
            }
        }
    }
}

bool GwmLayoutBatchDialog::getRasterExportSettings(QgsLayout* layout, QgsLayoutExporter::ImageExportSettings &settings, QSize &imageSize, QVector<double> scales)
{
    QSizeF maxPageSize;
	bool hasUniformPageSizes = false;
	double dpi = 300;
	bool cropToContents = false;
	int marginTop = 0;
	int marginRight = 0;
	int marginBottom = 0;
	int marginLeft = 0;
	bool antialias = true;

	// Image size
	if (layout)
	{
		settings.flags = layout->renderContext().flags();

		maxPageSize = layout->pageCollection()->maximumPageSize();
		hasUniformPageSizes = layout->pageCollection()->hasUniformPageSizes();
		dpi = layout->renderContext().dpi();

		//get some defaults from the composition
		cropToContents = layout->customProperty(QStringLiteral("imageCropToContents"), false).toBool();
		marginTop = layout->customProperty(QStringLiteral("imageCropMarginTop"), 0).toInt();
		marginRight = layout->customProperty(QStringLiteral("imageCropMarginRight"), 0).toInt();
		marginBottom = layout->customProperty(QStringLiteral("imageCropMarginBottom"), 0).toInt();
		marginLeft = layout->customProperty(QStringLiteral("imageCropMarginLeft"), 0).toInt();
		antialias = layout->customProperty(QStringLiteral("imageAntialias"), true).toBool();
	}

	QgsLayoutImageExportOptionsDialog imageDlg(this);
	imageDlg.setImageSize(maxPageSize);
	imageDlg.setResolution(dpi);
	imageDlg.setCropToContents(cropToContents);
	imageDlg.setCropMargins(marginTop, marginRight, marginBottom, marginLeft);
	if (layout)
		imageDlg.setGenerateWorldFile(layout->customProperty(QStringLiteral("exportWorldFile"), false).toBool());
	imageDlg.setAntialiasing(antialias);

	if (!imageDlg.exec())
		return false;

	imageSize = QSize(imageDlg.imageWidth(), imageDlg.imageHeight());
	cropToContents = imageDlg.cropToContents();
	imageDlg.getCropMargins(marginTop, marginRight, marginBottom, marginLeft);
	if (layout)
	{
		layout->setCustomProperty(QStringLiteral("imageCropToContents"), cropToContents);
		layout->setCustomProperty(QStringLiteral("imageCropMarginTop"), marginTop);
		layout->setCustomProperty(QStringLiteral("imageCropMarginRight"), marginRight);
		layout->setCustomProperty(QStringLiteral("imageCropMarginBottom"), marginBottom);
		layout->setCustomProperty(QStringLiteral("imageCropMarginLeft"), marginLeft);
		layout->setCustomProperty(QStringLiteral("imageAntialias"), imageDlg.antialiasing());
	}

	settings.cropToContents = cropToContents;
	settings.cropMargins = QgsMargins(marginLeft, marginTop, marginRight, marginBottom);
	settings.dpi = imageDlg.resolution();
	if (hasUniformPageSizes)
	{
		settings.imageSize = imageSize;
	}
	settings.generateWorldFile = imageDlg.generateWorldFile();
	settings.predefinedMapScales = scales;
	settings.flags |= QgsLayoutRenderContext::FlagUseAdvancedEffects;
	if (imageDlg.antialiasing())
		settings.flags |= QgsLayoutRenderContext::FlagAntialiasing;
	else
		settings.flags &= ~QgsLayoutRenderContext::FlagAntialiasing;

    return true;
}

QVector<double> GwmLayoutBatchDialog::predefinedScales(QgsMasterLayoutInterface* mMasterLayout) const
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

void GwmLayoutBatchDialog::on_btnSelectDirectory_clicked()
{
    QFileDialog dlg(this, tr("Select a Folder"));
    dlg.setFileMode(QFileDialog::FileMode::DirectoryOnly);
    if (dlg.exec())
    {
        auto pathList = dlg.selectedFiles();
        ui->txtOutputDirectory->setText(pathList.first());
    }
}

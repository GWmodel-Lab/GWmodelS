#include "gwmlayoutbatchdialog.h"
#include "ui_gwmlayoutbatchdialog.h"

#include <qitemselectionmodel.h>
#include <QDebug>
#include <QString>

#include <qgsproject.h>
#include <qgslayoutmanager.h>
#include <qgslayoutimageexportoptionsdialog.h>
#include <qgslayoutpagecollection.h>
#include <qgsprojectviewsettings.h>
#include <qgsprintlayout.h>
#include <qgsproxyprogresstask.h>
#include <qgsvaliditycheckcontext.h>
#include <qgsvaliditycheckresultswidget.h>
#include <qgsabstractvaliditycheck.h>
#include <qgslayoutitemmap.h>
#include <qgsmessageviewer.h>
#include <qgssinglesymbolrenderer.h>
#include <qgssymbolselectordialog.h>
#include <qgslayoutmodel.h>
#include <qgslayoutitemlegend.h>
#include <qgslayoutitemlabel.h>
#include <qgsmaplayerlegend.h>

#include "gwmapp.h"
#include "gwmlayoutbatchlayerlistmodel.h"
#include "gwmlayoutbatchfieldlistmodel.h"
#include "gwmlayoutbatchconfigurationmodel.h"
#include "gwmlayoutbatchconfigurationdelegate.h"
#include "symbolwindow/gwmsymboleditordialog.h"
#include "Layout/gwmlayoutbatchunifylayerpopupdialog.h"

GwmLayoutBatchDialog::GwmLayoutBatchDialog(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::GwmLayoutBatchDialog();
	ui->setupUi(this);

	mLayoutModel = new QgsLayoutManagerModel(QgsProject::instance()->layoutManager(), this);
	mLayoutProxyModel = new QgsLayoutManagerProxyModel(ui->lsvLayout);
	mLayoutProxyModel->setSourceModel(mLayoutModel);
	ui->lsvLayout->setModel(mLayoutProxyModel);
    connect(ui->lsvLayout->selectionModel(), &QItemSelectionModel::currentChanged, this, &GwmLayoutBatchDialog::onLayoutSelectionModelCurrentChanged);

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
    connect(mConfigurationSelectionModel, &QItemSelectionModel::currentChanged, this, [&](const QModelIndex &current, const QModelIndex &/*deselected*/)
    {
        GwmLayoutBatchConfigurationItem* item = mConfigurationModel->itemFromIndex(current);
        if (item && item->type() == GwmLayoutBatchConfigurationItem::Layer)
        {
            ui->btnEditSymbol->setEnabled(false);
            ui->btnUnifyLayer->setEnabled(true);
            ui->btnUnifyField->setEnabled(false);
        }
        else if (item && item->type() == GwmLayoutBatchConfigurationItem::Field)
        {
            ui->btnEditSymbol->setEnabled(true);
            ui->btnUnifyLayer->setEnabled(false);
            ui->btnUnifyField->setEnabled(true);
        }
    });

//    connect(ui->btnEditSymbol, &QToolButton::clicked, this, &GwmLayoutBatchDialog::on_btnEditSymbol_clicked);
    QMenu* exportToRasterMenu = new QMenu(ui->btnExportRaster);
    const auto supportedImageFormats = QImageWriter::supportedImageFormats();
    for (const auto format : supportedImageFormats)
    {
        if (format == "svg")
            continue;
        QAction* action = new QAction(exportToRasterMenu);
        action->setText(format);
        exportToRasterMenu->addAction(action);
    }
    ui->btnExportRaster->setMenu(exportToRasterMenu);
    ui->btnExportRaster->setPopupMode(QToolButton::InstantPopup);

    toggleWidgets(false);
    setWindowTitle(QString("Layout Batch Processing"));
}

GwmLayoutBatchDialog::~GwmLayoutBatchDialog()
{
    delete ui;
}

void GwmLayoutBatchDialog::onLayoutSelectionModelCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    QgsMasterLayoutInterface* currentLayout = mLayoutModel->layoutFromIndex(current);
    if (!currentLayout)
        return;

    // 如果尚未输入文件名，则直接使用此布局名称
    QString currentFilename = ui->txtFileNameTemplate->text();
    if (currentFilename.isEmpty())
    {
        ui->txtFileNameTemplate->setText(currentLayout->name());
        ui->txtFileNameTemplate->setCursorPosition(currentLayout->name().size());
    }
    //如果已经输入文件名，替换文件名中的布局名
    else
    {
        QgsMasterLayoutInterface* previousLayout = mLayoutModel->layoutFromIndex(previous);
        if (!previousLayout)
            return;

        QRegExp rx(previousLayout->name());
        if (rx.exactMatch(currentFilename))
        {
            currentFilename.replace(rx, currentLayout->name());
            ui->txtFileNameTemplate->setText(currentLayout->name());
            ui->txtFileNameTemplate->setCursorPosition(currentLayout->name().size());
        }
    }

    if (currentLayout)
    {
        ui->btnExportRaster->setEnabled(true);
    }
}

void GwmLayoutBatchDialog::onLayerSelectionModelCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (mFieldModel)
    {
        disconnect(mFieldModel, &QAbstractItemModel::dataChanged, this, &GwmLayoutBatchDialog::onFieldModelDataChanged);
    }
    QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>(mLayerModel->layerFromIndex(current));
    if (layer)
    {
        mFieldModel = mLayerModel->itemFromindex(current)->fieldModel;
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
//                    QgsProject::instance()->addMapLayer(layer);
                }
                else
                {
                    QgsVectorLayer* layer = mLayerModel->layerFromIndex(topLeft);
                    mConfigurationModel->removeLayer(layer);
//                    QgsProject::instance()->removeMapLayer(layer);
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
        editor->setSelectedRenderer(fieldItem->renderer()->clone());
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
    exportToRasterBatch("png");
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

void GwmLayoutBatchDialog::exportToRasterBatch(const QString &ext)
{
    if (ui->txtOutputDirectory->text().size() <= 0)
    {
        QMessageBox::information(this, "Batch Layout", "No directory is selected", QMessageBox::Yes);
        return ;
    }

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
        //toggleWidgets(true);

        if (!checkBeforeExport(layout))
            return;

        if (containsWmsLayers(layout))
            showWmsPrintingWarning();

        if (!showFileSizeWarning(layout))
            return;

        QVector<double> scales = predefinedScales(currentLayout);
        QgsLayoutExporter::ImageExportSettings settings;
        QSize imageSize;
        if (getRasterExportSettings(layout, settings, imageSize, scales))
        {
            toggleWidgets(true);
            // 批量导出
            auto rootItem = static_cast<GwmLayoutBatchConfigurationItemRoot*>(mConfigurationModel->itemFromIndex(QModelIndex()));
            if (rootItem)
            {
                // 先移除所有选中的图层
                QList<GwmLayoutBatchConfigurationItemLayer*> layerItemList = rootItem->layerList();
                QList<QgsMapLayer*> currentMapList0 = GwmApp::Instance()->mapLayerList();
                QList<QgsMapLayer*> currentMapList = GwmApp::Instance()->mapLayerList();
                int progressBarTotal = 0;
                for (auto item : layerItemList)
                {
                    progressBarTotal += item->fieldList().size();
                    currentMapList.removeOne(item->layer());
                    QgsProject::instance()->takeMapLayer(item->layer());
                }
                GwmApp::Instance()->mapCanvas()->setLayers(currentMapList);
                GwmApp::Instance()->mapCanvas()->refresh();
                // 保存字符串模板
                QMap<QgsLayoutItem*, QString> layoutItemTemplateMap;
                QgsLayoutModel* layoutModel = layout->itemsModel();
                for (int i = 0; i < layoutModel->rowCount(); i++)
                {
                    const QModelIndex& index = layoutModel->index(i, 0);
                    QgsLayoutItem* item = layoutModel->itemFromIndex(index);

                    if (QgsLayoutItemLegend* legend = dynamic_cast<QgsLayoutItemLegend*>(item))
                    {
                        QString titleTemplate = legend->title();
                        layoutItemTemplateMap[item] = titleTemplate;
                    }
                    else if (QgsLayoutItemLabel* label = dynamic_cast<QgsLayoutItemLabel*>(item))
                    {
                        QString titleTemplate = label->text();
                        layoutItemTemplateMap[item] = titleTemplate;
                    }
                }
                // 逐个添加图层
                for (int l = 0; l < layerItemList.size(); l++)
                {
                    auto layerItem = layerItemList[l];
                    QgsVectorLayer* layer = layerItem->layer();
                    currentMapList.append(layer);
                    QgsProject::instance()->addMapLayer(layerItem->layer());
                    GwmApp::Instance()->mapCanvas()->setLayers(currentMapList);
                    GwmApp::Instance()->mapCanvas()->refresh();
                    auto fieldItemList = layerItem->fieldList();
                    for (int f = 0; f < fieldItemList.size(); f++)
                    {
                        auto fieldItem = fieldItemList[f];
                        // 修改符号
                        QgsFeatureRenderer* renderer0 = layer->renderer()->clone();
                        QgsFeatureRenderer* renderer = fieldItem->renderer();
                        layer->setRenderer(renderer->clone());
                        QgsMapLayerLegend* legend = QgsMapLayerLegend::defaultVectorLegend(layer);
                        layer->setLegend(legend);
                        layout->refresh();
                        // 图例和其他文字
                        for (auto item : layoutItemTemplateMap.keys())
                        {
                            if (QgsLayoutItemLegend* legend = dynamic_cast<QgsLayoutItemLegend*>(item))
                            {
                                QString titleTemplate = layoutItemTemplateMap[legend];
                                QString title = titleTemplate.replace("%layer%", layerItem->name(), Qt::CaseInsensitive).replace("%field%", fieldItem->name(), Qt::CaseInsensitive);
                                legend->setTitle(title);
                                legend->updateFilterByMap();
                                legend->refresh();

                            }
                            else if (QgsLayoutItemLabel* label = dynamic_cast<QgsLayoutItemLabel*>(item))
                            {
                                QString titleTemplate = layoutItemTemplateMap[label];
                                QString title = titleTemplate.replace("%layer%", layerItem->name(), Qt::CaseInsensitive).replace("%field%", fieldItem->name(), Qt::CaseInsensitive);
                                label->setText(title);
                                label->refresh();
                            }
                        }
                        layout->refresh();
                        layout->update();
                        // 导出图层
                        QFileInfo outputFile = formatOutputFile(directory, filenameTemplate, layerItem->name(), fieldItem->name(), ext);
                        QgsProxyProgressTask* proxyTask = new QgsProxyProgressTask(tr("Exporting %1").arg(outputFile.absoluteFilePath()));
                        QgsApplication::taskManager()->addTask(proxyTask);
                        layout->refresh();
                        QgsLayoutExporter exporter(layout);
                        QgsLayoutExporter::ExportResult result = exporter.exportToImage(outputFile.absoluteFilePath(), settings);
                        proxyTask->finalize(result == QgsLayoutExporter::Success);
                        switch (result) {
                        case QgsLayoutExporter::Success:
                        {
                            int current = l * layerItemList.size() + f;
                            updateProgressBar(current, progressBarTotal, QString("%1 %2 %3").arg(layerItem->name()).arg(fieldItem->name()).arg("Success"));
//                            qDebug() << "[Export layout] Successfully exported layout to"
//                                     << outputFile.filePath();
                            break;
                        }
                        case QgsLayoutExporter::PrintError:
                        case QgsLayoutExporter::SvgLayerError:
                        case QgsLayoutExporter::IteratorError:
                        case QgsLayoutExporter::Canceled:
                        {
                            // no meaning for raster exports, will not be encountered
                            int current = l * layerItemList.size() + f;
                            updateProgressBar(current, progressBarTotal, QString("%1 %2 %3").arg(layerItem->name()).arg(fieldItem->name()).arg("Error"));
                            break;
                        }
                        case QgsLayoutExporter::FileError:
                        {
                            int current = l * layerItemList.size() + f;
                            QStringList errorMessageList = QStringList() << tr("[Image Export Error]")
                                                                         << tr(" Cannot write to")
                                                                         << outputFile.filePath()
                                                                         << tr(". This file may be open in another application.");
                            updateProgressBar(current, progressBarTotal, QString("%1 %2 %3").arg(layerItem->name()).arg(fieldItem->name()).arg(errorMessageList.join(" ")));
                            break;
                        }
                        case QgsLayoutExporter::MemoryError:
                        {
                            int current = l * layerItemList.size() + f;
                            QStringList errorMessageList = QStringList() << tr("[Image Export Error]")
                                                                         << tr("Trying to create image")
                                                                         << outputFile.filePath()
                                                                         << tr("resulted in a memory overflow.")
                                                                         << tr("Please try a lower resolution or a smaller paper size.");
                            updateProgressBar(current, progressBarTotal, QString("%1 %2 %3").arg(layerItem->name()).arg(fieldItem->name()).arg(errorMessageList.join(" ")));
                            break;
                        }
                        default:
                            break;
                        }
                        // 恢复符号
                        layer->setRenderer(renderer0);
                    }
                    currentMapList.removeOne(layer);
                    QgsProject::instance()->takeMapLayer(layerItem->layer());
                    GwmApp::Instance()->mapCanvas()->setLayers(currentMapList0);
                    GwmApp::Instance()->mapCanvas()->refresh();
                }
                // 恢复所有图层
                for (auto item : rootItem->layerList())
                {
                    QgsProject::instance()->addMapLayer(item->layer());
                }
                GwmApp::Instance()->mapCanvas()->setLayers(currentMapList0);
                GwmApp::Instance()->mapCanvas()->refresh();
                // 恢复图例和文字标题
                for (auto item : layoutItemTemplateMap.keys())
                {
                    if (QgsLayoutItemLegend* legend = dynamic_cast<QgsLayoutItemLegend*>(item))
                    {
                        legend->updateLegend();
                        QString titleTemplate = layoutItemTemplateMap[legend];
                        legend->setTitle(titleTemplate);
                    }
                    else if (QgsLayoutItemLabel* label = dynamic_cast<QgsLayoutItemLabel*>(item))
                    {
                        QString titleTemplate = layoutItemTemplateMap[label];
                        label->setText(titleTemplate);
                    }
                }
            }
        }
        toggleWidgets(false);
    }
}

QFileInfo GwmLayoutBatchDialog::formatOutputFile(QString directory, QString filenameTemplate, QString layerName, QString fieldName, QString ext)
{
    QString filename = filenameTemplate.replace("%layer%", layerName).replace("%field%", fieldName);
    QFileInfo outputFile(directory, filename);
    if (outputFile.suffix() != ext)
    {
        filename = (QList<QString>() << outputFile.completeBaseName() << ext).join(".");
        outputFile.setFile(directory, filename);
    }
    return outputFile;
}

void GwmLayoutBatchDialog::updateProgressBar(int current, int total, const QString &text)
{
    ui->pgbExport->setValue(current);
    ui->pgbExport->setMaximum(total);
    ui->pgbExport->setFormat(QStringLiteral("%p% ") + text);
}

void GwmLayoutBatchDialog::toggleWidgets(bool started)
{
    if (started)
    {
        ui->btnExportRaster->setEnabled(false);
        ui->btnEditSymbol->setEnabled(false);
        ui->pgbExport->show();
    }
    else
    {
        if (ui->lsvLayout->selectionModel()->selectedIndexes().size() > 0 && mLayerModel->checkedLayers().size() > 0)
        {
            ui->btnExportRaster->setEnabled(true);
            ui->btnEditSymbol->setEnabled(true);
        }
        ui->pgbExport->hide();
    }
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

void GwmLayoutBatchDialog::on_btnExportRaster_triggered(QAction *action)
{
    exportToRasterBatch(action->text());
}

void GwmLayoutBatchDialog::on_btnAddLayerNamePlace_clicked()
{
    int cursor = ui->txtFileNameTemplate->cursorPosition();
    QString filenameTemplate = ui->txtFileNameTemplate->text();
    filenameTemplate.insert(cursor, "%layer%");
    ui->txtFileNameTemplate->setText(filenameTemplate);
    ui->txtFileNameTemplate->setCursorPosition(cursor + 7);
}

void GwmLayoutBatchDialog::on_txtFileAddFieldNamePlace_clicked()
{
    int cursor = ui->txtFileNameTemplate->cursorPosition();
    QString filenameTemplate = ui->txtFileNameTemplate->text();
    filenameTemplate.insert(cursor, "%field%");
    ui->txtFileNameTemplate->setText(filenameTemplate);
    ui->txtFileNameTemplate->setCursorPosition(cursor + 7);
}

bool GwmLayoutBatchDialog::checkBeforeExport(QgsLayout *layout)
{
    if (layout)
	{
		QgsLayoutValidityCheckContext context(layout);
		return QgsValidityCheckResultsWidget::runChecks(QgsAbstractValidityCheck::TypeLayoutCheck, &context, tr("Checking Layout"),
			tr("The layout generated the following warnings. Please review and address these before proceeding with the layout export."), this);
	}
	else
	{
		return true;
    }
}

bool GwmLayoutBatchDialog::containsWmsLayers(QgsLayout *layout)
{
    QList< QgsLayoutItemMap *> maps;
	layout->layoutItems(maps);

	for (QgsLayoutItemMap *map : std::as_const(maps))
	{
		if (map->containsWmsLayer())
			return true;
	}
    return false;
}

bool GwmLayoutBatchDialog::showFileSizeWarning(QgsLayout *layout)
{
    // Image size
	double oneInchInLayoutUnits = layout->convertToLayoutUnits(QgsLayoutMeasurement(1, QgsUnitTypes::LayoutInches));
	QSizeF maxPageSize = layout->pageCollection()->maximumPageSize();
	int width = static_cast<int>(layout->renderContext().dpi() * maxPageSize.width() / oneInchInLayoutUnits);
	int height = static_cast<int>(layout->renderContext().dpi() * maxPageSize.height() / oneInchInLayoutUnits);
	int memuse = width * height * 3 / 1000000;  // pixmap + image
	QgsDebugMsg(QStringLiteral("Image %1x%2").arg(width).arg(height));
	QgsDebugMsg(QStringLiteral("memuse = %1").arg(memuse));

	if (memuse > 400)   // about 4500x4500
	{
		int answer = QMessageBox::warning(this, tr("Export Layout"),
			tr("To create an image of %1x%2 requires about %3 MB of memory. Proceed?")
			.arg(width).arg(height).arg(memuse),
			QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);

		raise();
		if (answer == QMessageBox::Cancel)
			return false;
	}
    return true;
}

void GwmLayoutBatchDialog::showWmsPrintingWarning()
{
    QgsSettings settings;
	bool displayWMSWarning = settings.value(QStringLiteral("/UI/displayComposerWMSWarning"), true).toBool();
	if (displayWMSWarning)
	{
		QgsMessageViewer *m = new QgsMessageViewer(this);
		m->setWindowTitle(tr("Project Contains WMS Layers"));
		m->setMessage(tr("Some WMS servers (e.g. UMN mapserver) have a limit for the WIDTH and HEIGHT parameter. Printing layers from such servers may exceed this limit. If this is the case, the WMS layer will not be printed"), QgsMessageOutput::MessageText);
		m->setCheckBoxText(tr("Don't show this message again"));
		m->setCheckBoxState(Qt::Unchecked);
		m->setCheckBoxVisible(true);
		m->setCheckBoxQgsSettingsLabel(QStringLiteral("/UI/displayComposerWMSWarning"));
		m->exec(); //deleted on close
	}
}

void GwmLayoutBatchDialog::on_btnUnifyLayer_clicked()
{
    const QModelIndex& currentLayerIndex = mConfigurationSelectionModel->currentIndex();
    GwmLayoutBatchConfigurationItem* item = mConfigurationModel->itemFromIndex(currentLayerIndex);
    if (item && item->type() == GwmLayoutBatchConfigurationItem::Layer)
    {
        QgsVectorLayer* currentLayer = static_cast<GwmLayoutBatchConfigurationItemLayer*>(item)->layer();
        if (currentLayer)
        {
            GwmSymbolEditorDialog* editor = new GwmSymbolEditorDialog(currentLayer);
            if (editor->exec())
            {
                QgsFeatureRenderer* renderer = editor->selectedRenderer();
                if (renderer->type() == "singleSymbol")
                {
                    unifyLayerRendererSimpleSymbol(currentLayer, renderer);
                }
                else if (renderer->type() == "graduatedSymbol")
                {
                    unifyLayerRendererGraduatedSymbol(currentLayer, renderer);
                }
            }
            delete editor;
        }
    }
}

void GwmLayoutBatchDialog::unifyLayerRendererSimpleSymbol(QgsVectorLayer *layer, QgsFeatureRenderer* renderer)
{
    QModelIndex layerIndex = mConfigurationModel->indexFromLayer(layer);
    GwmLayoutBatchConfigurationItem* item = mConfigurationModel->itemFromIndex(layerIndex);
    if (item && item->type() == GwmLayoutBatchConfigurationItem::Layer)
    {
        auto layerItem = static_cast<GwmLayoutBatchConfigurationItemLayer*>(item);
        auto fieldList = layerItem->fieldList();
        for (auto fieldItem : fieldList)
        {
            fieldItem->setRenderer(renderer);
        }
    }
}

void GwmLayoutBatchDialog::unifyLayerRendererGraduatedSymbol(QgsVectorLayer *layer, QgsFeatureRenderer *renderer)
{
    QgsGraduatedSymbolRenderer* graduated0 = static_cast<QgsGraduatedSymbolRenderer*>(renderer->clone());
    QModelIndex layerIndex = mConfigurationModel->indexFromLayer(layer);
    GwmLayoutBatchConfigurationItem* item = mConfigurationModel->itemFromIndex(layerIndex);
    if (item && item->type() == GwmLayoutBatchConfigurationItem::Layer)
    {
        auto layerItem = static_cast<GwmLayoutBatchConfigurationItemLayer*>(item);
        auto fieldList = layerItem->fieldList();
        for (auto fieldItem : fieldList)
        {
            QgsGraduatedSymbolRenderer* graduated = graduated0->clone();
            graduated->setClassAttribute(fieldItem->name());
            int nclasses = graduated0->ranges().size();
            graduated->updateClasses(layer, nclasses);
            graduated->updateColorRamp(graduated0->sourceColorRamp()->clone());
            fieldItem->setRenderer(graduated);
            delete graduated;
        }
    }
    delete graduated0;
}

void GwmLayoutBatchDialog::on_btnUnifyField_clicked()
{
    QModelIndex currentIndex = mConfigurationSelectionModel->currentIndex();
    GwmLayoutBatchConfigurationItem* item = mConfigurationModel->itemFromIndex(currentIndex);
    if (item && item->type() == GwmLayoutBatchConfigurationItem::Field)
    {
        auto fieldItem0 = static_cast<GwmLayoutBatchConfigurationItemField*>(item);
        QgsFeatureRenderer* renderer = fieldItem0->renderer();
        auto rootItem = mConfigurationModel->itemRoot();
        for (auto layerItem : rootItem->layerList())
        {
            for (auto fieldItem : layerItem->fieldList())
            {
                if (fieldItem != fieldItem0 && fieldItem->name() == fieldItem0->name())
                {
                    fieldItem->setRenderer(renderer->clone());
                }
            }
        }
    }
}

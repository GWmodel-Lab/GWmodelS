#pragma once

#include <QDialog>
#include <QAbstractItemModel>
#include <qgslayoutexporter.h>

namespace Ui { class GwmLayoutBatchDialog; };

class QgsLayoutManagerModel;
class QgsLayoutManagerProxyModel;
class GwmLayoutBatchLayerListModel;
class QItemSelectionModel;
class GwmLayoutBatchFieldListModel;
class GwmLayoutBatchConfigurationModel;
class GwmLayoutBatchConfigurationDelegate;


class GwmLayoutBatchDialog : public QDialog
{
	Q_OBJECT

public:
	GwmLayoutBatchDialog(QWidget *parent = Q_NULLPTR);
	~GwmLayoutBatchDialog();

private slots:
    void onLayoutSelectionModelCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void onLayerSelectionModelCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void onLayerModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void onFieldModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

    void on_btnEditSymbol_clicked();
    void on_btnBox_accepted();
    void on_btnExportRaster_clicked();
    void on_btnSelectDirectory_clicked();

    void on_btnExportRaster_triggered(QAction *arg1);

    bool getRasterExportSettings(QgsLayout *mLayout, QgsLayoutExporter::ImageExportSettings &settings, QSize &imageSize, QVector<double> predefinedScales);
    QVector<double> predefinedScales(QgsMasterLayoutInterface *mMasterLayout) const;

    void on_btnAddLayerNamePlace_clicked();

    void on_txtFileAddFieldNamePlace_clicked();

private:
    bool checkBeforeExport(QgsLayout* layout);
    bool containsWmsLayers(QgsLayout* layout);
    bool showFileSizeWarning(QgsLayout* layout);
    void showWmsPrintingWarning();
    void exportToRaster(const QString& ext);
    QFileInfo formatOutputFile(QString dir, QString filenameTemplate, QString layerName, QString fieldName, QString ext);

    void updateProgressBar(int current, int total, const QString& text);
    void toggleWidgets(bool started);

private:
	Ui::GwmLayoutBatchDialog *ui;

	QgsLayoutManagerModel *mLayoutModel = nullptr;
	QgsLayoutManagerProxyModel *mLayoutProxyModel = nullptr;
	
	GwmLayoutBatchLayerListModel* mLayerModel = nullptr;
	QItemSelectionModel* mLayerSelectionModel = nullptr;

    GwmLayoutBatchFieldListModel* mFieldModel = nullptr;

    GwmLayoutBatchConfigurationModel* mConfigurationModel = nullptr;
    QItemSelectionModel* mConfigurationSelectionModel = nullptr;
    GwmLayoutBatchConfigurationDelegate* mConfiguationDelegate = nullptr;

};

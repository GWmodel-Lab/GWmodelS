#ifndef GWMSWIMOPTIONSDIALOG_H
#define GWMSWIMOPTIONSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QList>
#include <QPair>
#include <QStringList>
#include <QSet>
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include "TaskThread/gwmswimtaskthread.h"
#include "TaskThread/iparallelable.h"
#include "SpatialWeight/gwmdistance.h"
#include "SpatialWeight/gwmbandwidthweight.h"

class QListWidget;
class QListWidgetItem;
class QgsVectorLayer;

namespace Ui {
class GwmSWIMOptionsDialog;
}

class GwmSWIMOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmSWIMOptionsDialog(QWidget *parent = nullptr);
    ~GwmSWIMOptionsDialog();

    QString csvFilePath() const;
    SWIMMode swimMode() const;
    bool hasValidSwimMode() const;
    bool bandwidthType() const;  // true = adaptive, false = fixed
    double bandwidthSize() const;
    GwmBandwidthWeight::KernelFunctionType bandwidthKernelFunction() const;
    GwmDistance::DistanceType distanceSourceType() const;
    QVariant distanceSourceParameters() const;
    QVariant parallelParameters() const;
    IParallelalbe::ParallelType parallelType() const;

    void setTaskThread(GwmSWIMTaskThread* taskThread);
    void updateFieldsAndEnable();
    void updateFields();
    void enableAccept();

public slots:
    void onCsvFileOpenClicked();
    void onInputDataSourceChanged();
    void onLayerSelectionChanged(int index);
    void onSwimModeChanged(int index);
    void onFixedRadioToggled(bool checked);
    void onVariableRadioToggled(bool checked);
    void onAutomaticRadioToggled(bool checked);
    void onCustomizeRadioToggled(bool checked);
    void onNoneRadioToggled(bool checked);
    void onMultithreadingRadioToggled(bool checked);
    void onGPURadioToggled(bool checked);
    void onDistTypeCRSToggled(bool checked);
    void onDistTypeMinkowskiToggled(bool checked);
    void onDistTypeDmatToggled(bool checked);
    void onDmatFileOpenClicked();

private:
    Ui::GwmSWIMOptionsDialog *ui;
    GwmSWIMTaskThread* mTaskThread = nullptr;
    GwmBandwidthWeight* mBandwidth = nullptr;
    bool isNumeric(QVariant::Type type);

    QString crsRotateTheta() const;
    QString crsRotateP() const;

    bool loadCsvHeaders(const QString& filePath);
    bool loadLayerHeaders(QgsVectorLayer* layer);
    void populateFieldMappingCombos(const QStringList& headers);
    void clearFieldMappingControls();
    QList<QPair<QString, QComboBox*>> fieldComboPairs() const;
    void populateIndependentVariableList(const QStringList& headers, const QSet<int>& reservedIndices);
    void updateIndependentFieldStates();
    QList<int> selectedIndependentVariableColumns() const;
    QStringList selectedIndependentVariableNames() const;
    QSet<int> reservedFieldIndices() const;
    GwmSWIMFieldMapping currentFieldMapping() const;
    bool isFieldMappingComplete() const;
    QChar detectDelimiter(const QString& line) const;
    void onAddIndependentVariableClicked();
    void onRemoveIndependentVariableClicked();
    QListWidgetItem* createListItemForColumn(const QString& header, int column) const;
    void moveItems(QListWidget* from, QListWidget* to);
    int findHeaderIndex(const QString& name) const;
    bool modeNeedsOriginCoords() const;
    bool modeNeedsDestCoords() const;
    void updateCoordinateControlState();
    bool usingImportedLayerData() const;
    QgsVectorLayer* selectedImportedLayer() const;
    QString resolveInputCsvPath();
    bool exportLayerToCsv(QgsVectorLayer* layer, const QString& csvPath);
    QString csvEscaped(const QString& value) const;
private:
    QStringList mCsvHeaders;
    QChar mDetectedDelimiter = '\t';
    QString mGeneratedCsvPath;
};

#endif // GWMSWIMOPTIONSDIALOG_H


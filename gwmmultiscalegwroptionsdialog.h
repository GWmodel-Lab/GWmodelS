#ifndef GWMMULTISCALEGWROPTIONSDIALOG_H
#define GWMMULTISCALEGWROPTIONSDIALOG_H

#include <QDialog>
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include "TaskThread/gwmmultiscalegwralgorithm.h"
#include "TaskThread/iparallelable.h"
#include "Model/gwmlayerattributeitemmodel.h"
#include "Model/gwmlayergroupitem.h"
#include "Model/gwmparameterspecifiedoptionsmodel.h"

namespace Ui {
class GwmMultiscaleGWROptionsDialog;
}

class GwmMultiscaleGWROptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmMultiscaleGWROptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmMultiscaleGWRAlgorithm* thread,QWidget *parent = nullptr);
    ~GwmMultiscaleGWROptionsDialog();

private:
    Ui::GwmMultiscaleGWROptionsDialog *ui;
    QList<GwmLayerGroupItem*> mMapLayerList;
    GwmLayerGroupItem* mSelectedLayer = nullptr;
    GwmMultiscaleGWRAlgorithm* mTaskThread = nullptr;
    GwmVariableItemModel* mDepVarModel;
    bool isNumeric(QVariant::Type type);
    GwmParameterSpecifiedOptionsModel* mParameterSpecifiedOptionsModel = nullptr;
    QItemSelectionModel* mParameterSpecifiedOptionsSelectionModel = nullptr;

public slots:
    void layerChanged(const int index);
    void onDepVarChanged(const int index);
    void onFixedRadioToggled(bool checked);
    void onVariableRadioToggled(bool checked);
    void onNoneRadioToggled(bool checked);
    void onMultithreadingRadioToggled(bool checked);
    void onGPURadioToggled(bool checked);
    void onCustomizeRaidoToggled(bool checked);
    void onInitializeRadioToggled(bool checked);
    void onAutomaticRadioToggled(bool checked);
    void onDistTypeCRSToggled(bool checked);
    void onDistTypeMinkowskiToggled(bool checked);
    void onDistTypeDmatToggled(bool checked);
    void onDmatFileOpenClicked();
    void onSelectedIndenpendentVariablesChanged();
    void onSpecifiedParameterCurrentChanged(const QModelIndex& currnet, const QModelIndex& previous);
    void onBwSizeAutomaticApprochChanged(int index);
    void onBwSizeAdaptiveSizeChanged(int size);
    void onBwSizeAdaptiveUnitChanged(int index);
    void onBwSizeFixedSizeChanged(double size);
    void onBwSizeFixedUnitChanged(int index);
    void onBwKernelFunctionChanged(int index);
    void onPredictorCentralizationToggled(bool checked);

public:
    QString crsRotateTheta();
    QString crsRotateP();
    bool bandwidthType();
    IParallelalbe::ParallelType parallelType();
    double bandwidthSize();
    GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType bandwidthSelectionApproach();
    QString bandWidthUnit();
    GwmBandwidthWeight::KernelFunctionType bandwidthKernelFunction();
    GwmDistance::DistanceType distanceSourceType();
    QVariant distanceSourceParameters();
    QVariant parallelParameters();

    void setTaskThread(GwmMultiscaleGWRAlgorithm* taskThread);
    void updateFieldsAndEnable();
    void updateFields();
    void enableAccept();

    GwmLayerGroupItem *selectedLayer() const;
    void setSelectedLayer(GwmLayerGroupItem *selectedLayer);
private slots:
    void onBwSelecionThresholdSpbChanged(int arg1);
};

#endif // GWMMULTISCALEGWROPTIONSDIALOG_H

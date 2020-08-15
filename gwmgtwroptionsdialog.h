#ifndef GWMGTWROPTIONSDIALOG_H
#define GWMGTWROPTIONSDIALOG_H

#include <QDialog>
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include "TaskThread/gwmgtwralgorithm.h"
#include "TaskThread/gwmgwrtaskthread.h"
#include "Model/gwmlayerattributeitemmodel.h"
#include "Model/gwmlayergroupitem.h"

#include "Model/gwmvariableitemmodel.h"

namespace Ui {
class GwmGTWROptionsDialog;
}

class GwmGTWROptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmGTWROptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmGTWRAlgorithm* thread,QWidget *parent = nullptr);
    ~GwmGTWROptionsDialog();

    GwmLayerGroupItem *selectedLayer() const;
    void setSelectedLayer(GwmLayerGroupItem *selectedLayer);

    bool hasRegressionLayer() const;

    GwmLayerGroupItem* selectedRegressionLayer() const;

public slots:
    void layerChanged(const int index);
    void updatePredict();
    void onDepVarChanged(const int index);
    void onFixedRadioToggled(bool checked);
    void onVariableRadioToggled(bool checked);
    void onNoneRadioToggled(bool checked);
    void onMultithreadingRadioToggled(bool checked);
    void onGPURadioToggled(bool checked);
    void onCustomizeRaidoToggled(bool checked);
    void onAutomaticRadioToggled(bool checked);
    void onDistTypeCRSToggled(bool checked);
    void onDistTypeMinkowskiToggled(bool checked);
    void onDistTypeDmatToggled(bool checked);
    void onDmatFileOpenClicked();


public:
    QString crsRotateTheta();
    QString crsRotateP();
    bool bandwidthType();
    GwmGWRTaskThread::ParallelMethod approachType();
    double bandwidthSize();
    GwmGTWRAlgorithm::BandwidthSelectionCriterionType bandwidthSelectionApproach();
    QString bandWidthUnit();
    GwmBandwidthWeight::KernelFunctionType bandwidthKernelFunction();
    GwmGWRTaskThread::DistanceSourceType distanceSourceType();
    QVariant distanceSourceParameters();
    GwmGWRTaskThread::ParallelMethod parallelMethod();
    QVariant parallelParameters();

    void setTaskThread(GwmGWRTaskThread* taskThread);
    void updateFieldsAndEnable();
    void updateFields();
    void enableAccept();

private slots:
    void on_cbkRegressionPoints_toggled(bool checked);

private:
    Ui::GwmGTWROptionsDialog *ui;
    QList<GwmLayerGroupItem*> mMapLayerList;
    GwmLayerGroupItem* mSelectedLayer = nullptr;
    GwmGTWRAlgorithm* mTaskThread = nullptr;
    GwmVariableItemModel* mDepVarModel;
    GwmVariableItemModel* mTimeVarModel;
    bool isNumeric(QVariant::Type type);
};

#endif // GWMGTWROPTIONSDIALOG_H

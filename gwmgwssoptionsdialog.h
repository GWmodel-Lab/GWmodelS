#ifndef GWMGWSSOPTIONSDIALOG_H
#define GWMGWSSOPTIONSDIALOG_H

#include <QDialog>
#include "Model/gwmlayergroupitem.h"
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include "Model/gwmvariableitemmodel.h"
#include "TaskThread/gwmgwsstaskthread.h"
#include "TaskThread/gwmgwrtaskthread.h"

namespace Ui {
class GwmGWSSOptionsDialog;
}

class GwmGWSSOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmGWSSOptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmGWSSTaskThread* thread,QWidget *parent = nullptr);
    ~GwmGWSSOptionsDialog();

private:
    Ui::GwmGWSSOptionsDialog *ui;
    QList<GwmLayerGroupItem*> mMapLayerList;
    GwmLayerGroupItem* mSelectedLayer = nullptr;
    GwmGWSSTaskThread* mTaskThread = nullptr;
    GwmVariableItemModel* mDepVarModel;
    bool isNumeric(QVariant::Type type);
    GwmBandwidthWeight* mBandwidth;

public slots:
    void layerChanged(const int index);
//    void onDepVarChanged(const int index);
    void onFixedRadioToggled(bool checked);
    void onVariableRadioToggled(bool checked);
    void onNoneRadioToggled(bool checked);
    void onMultithreadingRadioToggled(bool checked);
    void onGPURadioToggled(bool checked);
//    void onCustomizeRaidoToggled(bool checked);
//    void onAutomaticRadioToggled(bool checked);
    void onDistTypeCRSToggled(bool checked);
    void onDistTypeMinkowskiToggled(bool checked);
    void onDistTypeDmatToggled(bool checked);
    void onDmatFileOpenClicked();
//    void onVariableAutoSelectionToggled(bool checked);

public:
    QString crsRotateTheta();
    QString crsRotateP();
    GwmGWRTaskThread::BandwidthType bandwidthType();
    GwmGWRTaskThread::ParallelMethod approachType();
//    double bandwidthSize();
//    GwmGWRTaskThread::BandwidthSelectionApproach bandwidthSelectionApproach();
//    QString bandWidthUnit();
    GwmBandwidthWeight::KernelFunctionType bandwidthKernelFunction();
    GwmGWRTaskThread::DistanceSourceType distanceSourceType();
    QVariant distanceSourceParameters();
    GwmGWRTaskThread::ParallelMethod parallelMethod();
    QVariant parallelParameters();

    void setTaskThread(GwmGWSSTaskThread* taskThread);
    void updateFieldsAndEnable();
    void updateFields();
    void enableAccept();

    GwmLayerGroupItem *selectedLayer() const;
    void setSelectedLayer(GwmLayerGroupItem *selectedLayer);
private slots:
//    void on_cbxHatmatrix_toggled(bool checked);
    void on_cbkRegressionPoints_toggled(bool checked);
    void on_cmbRegressionLayerSelect_currentIndexChanged(int index);



};

#endif // GWMGWSSOPTIONSDIALOG_H

#ifndef GWMROBUSTGWROPTIONSDIALOG_H
#define GWMROBUSTGWROPTIONSDIALOG_H

#include <QDialog>
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include "TaskThread/gwmrobustgwralgorithm.h"
#include "Model/gwmlayerattributeitemmodel.h"
#include "Model/gwmlayergroupitem.h"
#include "Model/gwmvariableitemmodel.h"

namespace Ui {
class GwmRobustGWROptionsDialog;
}

class GwmRobustGWROptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmRobustGWROptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmRobustGWRAlgorithm* thread,QWidget *parent = nullptr);
    ~GwmRobustGWROptionsDialog();

private:
    Ui::GwmRobustGWROptionsDialog *ui;
    QList<GwmLayerGroupItem*> mMapLayerList;
    GwmLayerGroupItem* mSelectedLayer = nullptr;
    GwmRobustGWRAlgorithm* mTaskThread = nullptr;
    GwmVariableItemModel* mDepVarModel;
    bool isNumeric(QVariant::Type type);

public slots:
    void layerChanged(const int index);
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
    void onVariableAutoSelectionToggled(bool checked);

    void onFilteredRadioToggled(bool checked);
    void onNotFilteredRadioToggled(bool checked);
public:
    QString crsRotateTheta();
    QString crsRotateP();
    //GwmRobustGWRTaskThread::BandwidthType bandwidthType();
    //GwmRobustGWRTaskThread::ParallelMethod approachType();
    double bandwidthSize();
    //GwmRobustGWRTaskThread::BandwidthSelectionApproach bandwidthSelectionApproach();
    QString bandWidthUnit();
    //GwmRobustGWRTaskThread::KernelFunction bandwidthKernelFunction();
    //GwmRobustGWRTaskThread::DistanceSourceType distanceSourceType();
    QVariant distanceSourceParameters();
    //GwmRobustGWRTaskThread::ParallelMethod parallelMethod();
    QVariant parallelParameters();

    void setTaskThread(GwmRobustGWRAlgorithm* taskThread);
    void updateFieldsAndEnable();
    void updateFields();
    void enableAccept();

    GwmLayerGroupItem *selectedLayer() const;
    void setSelectedLayer(GwmLayerGroupItem *selectedLayer);

    bool bandwidthType();
    GwmBandwidthWeight::KernelFunctionType bandwidthKernelFunction();
private slots:
    void on_cbxHatmatrix_toggled(bool checked);
    void on_cbkRegressionPoints_toggled(bool checked);
    void on_cmbRegressionLayerSelect_currentIndexChanged(int index);

    //void on_cbxFilter_toggled(bool checked);
};

#endif // GWMROBUSTGWROPTIONSDIALOG_H

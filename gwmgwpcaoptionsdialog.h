#ifndef GWMGWPCAOPTIONSDIALOG_H
#define GWMGWPCAOPTIONSDIALOG_H

#include <QDialog>
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include <TaskThread/gwmgwpcataskthread.h>
//#include "TaskThread/gwmgwrtaskthread.h"
#include "Model/gwmlayerattributeitemmodel.h"
#include "Model/gwmlayergroupitem.h"

#include "Model/gwmvariableitemmodel.h"

namespace Ui {
class GwmGWPCAOptionsDialog;
}

class GwmGWPCAOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmGWPCAOptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmGWPCATaskThread * thread,QWidget *parent = nullptr);
    ~GwmGWPCAOptionsDialog();

private:
    Ui::GwmGWPCAOptionsDialog *ui;
    QList<GwmLayerGroupItem*> mMapLayerList;
    GwmLayerGroupItem* mSelectedLayer = nullptr;
    GwmGWPCATaskThread* mTaskThread = nullptr;
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
    void onSelectedIndenpendentVariablesChanged();

public:
    QString crsRotateTheta();
    QString crsRotateP();
    bool bandwidthType();
    //GwmGWRTaskThread::ParallelMethod approachType();
    double bandwidthSize();
    //GwmGWPCATaskThread::BandwidthSelectionCriterionType bandwidthSelectionApproach();
    QString bandWidthUnit();
    GwmBandwidthWeight::KernelFunctionType bandwidthKernelFunction();
    //GwmGWRTaskThread::DistanceSourceType distanceSourceType();
    QVariant distanceSourceParameters();
    //GwmGWRTaskThread::ParallelMethod parallelMethod();
    QVariant parallelParameters();

    //void setTaskThread(GwmGWRTaskThread* taskThread);
    void updateFieldsAndEnable();
    void updateFields();
    void enableAccept();
    void onRobust();
    GwmLayerGroupItem *selectedLayer() const;
    void setSelectedLayer(GwmLayerGroupItem *selectedLayer);
private slots:
    void on_cbxHatmatrix_toggled(bool checked);
    void on_cbkRegressionPoints_toggled(bool checked);

};

#endif // GWMGWROPTIONSDIALOG_H

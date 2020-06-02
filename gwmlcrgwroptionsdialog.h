#ifndef GwmLcrGWROptionsDialog_H
#define GwmLcrGWROptionsDialog_H

#include <QDialog>
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include "TaskThread/GwmLcrGWRtaskthread.h"
#include "Model/gwmlayerattributeitemmodel.h"
#include "Model/gwmlayergroupitem.h"

namespace Ui {
class GwmLcrGWROptionsDialog;
}

class GwmLcrGWROptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmLcrGWROptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmLcrGWRTaskThread* thread,QWidget *parent = nullptr);
    ~GwmLcrGWROptionsDialog();

private:
    Ui::GwmLcrGWROptionsDialog *ui;
    QList<GwmLayerGroupItem*> mMapLayerList;
    GwmLayerGroupItem* mSelectedLayer = nullptr;
    GwmLcrGWRTaskThread* mTaskThread = nullptr;
    GwmLayerAttributeItemModel* mDepVarModel;
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


public:
    QString crsRotateTheta();
    QString crsRotateP();
    GwmLcrGWRTaskThread::BandwidthType bandwidthType();
    GwmLcrGWRTaskThread::ParallelMethod approachType();
    double bandwidthSize();
    GwmLcrGWRTaskThread::BandwidthSelectionApproach bandwidthSelectionApproach();
    QString bandWidthUnit();
    GwmLcrGWRTaskThread::KernelFunction bandwidthKernelFunction();
    GwmLcrGWRTaskThread::DistanceSourceType distanceSourceType();
    QVariant distanceSourceParameters();
    GwmLcrGWRTaskThread::ParallelMethod parallelMethod();
    QVariant parallelParameters();

    void setTaskThread(GwmLcrGWRTaskThread* taskThread);
    void updateFieldsAndEnable();
    void updateFields();
    void enableAccept();

    GwmLayerGroupItem *selectedLayer() const;
    void setSelectedLayer(GwmLayerGroupItem *selectedLayer);
private slots:
    void on_cbxHatmatrix_toggled(bool checked);
    void on_cbkRegressionPoints_toggled(bool checked);
    void on_cmbRegressionLayerSelect_currentIndexChanged(int index);
};

#endif // GwmLcrGWROptionsDialog_H

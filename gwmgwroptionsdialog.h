#ifndef GWMGWROPTIONSDIALOG_H
#define GWMGWROPTIONSDIALOG_H

#include <QDialog>
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include "TaskThread/gwmgwrtaskthread.h"
#include "Model/gwmlayerattributeitemmodel.h"

namespace Ui {
class GwmGWROptionsDialog;
}

class GwmGWROptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmGWROptionsDialog(QList<QgsMapLayer*> vectorLayerList, GwmGWRTaskThread* thread,QWidget *parent = nullptr);
    ~GwmGWROptionsDialog();

private:
    Ui::GwmGWROptionsDialog *ui;
    QList<QgsMapLayer*> mMapLayerList;
    QgsVectorLayer* mLayer = nullptr;
    GwmGWRTaskThread* mTaskThread = nullptr;
    GwmLayerAttributeItemModel* mDepVarModel;

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


public:
    QString crsRotateTheta();
    QString crsRotateP();
    GwmGWRTaskThread::BandwidthType bandwidthType();
    GwmGWRTaskThread::ParallelMethod approachType();
    double bandwidthSize();
    QString bandWidthUnit();
    GwmGWRTaskThread::KernelFunction bandwidthKernelFunction();
    GwmGWRTaskThread::DistanceSourceType distanceSourceType();
    QVariant distanceSourceParameters();
    GwmGWRTaskThread::ParallelMethod parallelMethod();
    QVariant parallelParameters();

    void setTaskThread(GwmGWRTaskThread* taskThread);
    void updateFieldsAndEnable();
    void updateFields();
    void enableAccept();
};

#endif // GWMGWROPTIONSDIALOG_H

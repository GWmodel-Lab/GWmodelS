#ifndef GWMGWROPTIONSDIALOG_H
#define GWMGWROPTIONSDIALOG_H

#include <QDialog>
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include "TaskThread/gwmgwrtaskthread.h"

namespace Ui {
class GwmGWROptionsDialog;
}

class GwmGWROptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmGWROptionsDialog(QList<QgsMapLayer*> vectorLayerList,QWidget *parent = nullptr);
    ~GwmGWROptionsDialog();

private:
    Ui::GwmGWROptionsDialog *ui;
    QList<QgsMapLayer*> mapLayerList;
    QgsVectorLayer* mLayer = nullptr;

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


public:
    QString crsRotateTheta();
    QString crsRotateP();
    GwmGWRTaskThread::BandwidthType bandwidthType();
    GwmGWRTaskThread::ParallelMethod approachType();
    double bandwidthSize();
    QString bandWidthUnit();
    GwmGWRTaskThread::KernelFunction bandwidthKernelFunction();
    QString parallelGPUBatchSize();
    QString parallelMultiThreadNum();
};

#endif // GWMGWROPTIONSDIALOG_H

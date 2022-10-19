#ifndef GWMGWAVERAGEOPTIONSDIALOG_H
#define GWMGWAVERAGEOPTIONSDIALOG_H

#include <QDialog>
#include "Model/gwmlayergroupitem.h"
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include "Model/gwmvariableitemmodel.h"
#include "TaskThread/gwmgwaveragetaskthread.h"
#include "TaskThread/iparallelable.h"
#include "SpatialWeight/gwmdistance.h"

namespace Ui {
class GwmGWaverageOptionsDialog;
}

class GwmGWaverageOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmGWaverageOptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmGWaverageTaskThread* thread,QWidget *parent = nullptr);
    ~GwmGWaverageOptionsDialog();

private:
    Ui::GwmGWaverageOptionsDialog *ui;
    QList<GwmLayerGroupItem*> mMapLayerList;
    GwmLayerGroupItem* mSelectedLayer = nullptr;
    GwmGWaverageTaskThread* mTaskThread = nullptr;
    bool isNumeric(QVariant::Type type);
    GwmBandwidthWeight* mBandwidth;

public slots:
    void layerChanged(const int index);

    void onFixedRadioToggled(bool checked);
    void onVariableRadioToggled(bool checked);
    void onNoneRadioToggled(bool checked);
    void onMultithreadingRadioToggled(bool checked);
    void onGPURadioToggled(bool checked);

    void onDistTypeCRSToggled(bool checked);
    void onDistTypeMinkowskiToggled(bool checked);
    void onDistTypeDmatToggled(bool checked);
    void onDmatFileOpenClicked();


public:
    QString crsRotateTheta();
    QString crsRotateP();
    bool bandwidthType();
    IParallelalbe::ParallelType approachType();
    double bandwidthSize();
//    GwmGWRTaskThread::BandwidthSelectionApproach bandwidthSelectionApproach();
//    QString bandWidthUnit();
    GwmBandwidthWeight::KernelFunctionType bandwidthKernelFunction();
    GwmDistance::DistanceType distanceSourceType();
    QVariant distanceSourceParameters();
    QVariant parallelParameters();

    void setTaskThread(GwmGWaverageTaskThread* taskThread);
    void updateFieldsAndEnable();
    void updateFields();
    void enableAccept();

    GwmLayerGroupItem *selectedLayer() const;
    void setSelectedLayer(GwmLayerGroupItem *selectedLayer);



};

#endif // GWMGWAVERAGEOPTIONSDIALOG_H

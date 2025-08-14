#ifndef GWMGTDROPTIONSDIALOG_H
#define GWMGTDROPTIONSDIALOG_H

#include <QDialog>
#include "Model/gwmlayergroupitem.h"
#include "Model/gwmalgorithmmetagwss.h"
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include "Model/gwmvariableitemmodel.h"
#include "TaskThread/gwmgwsstaskthread.h"
#include "TaskThread/iparallelable.h"
#include "SpatialWeight/gwmdistance.h"

namespace Ui {
class GwmGTDROptionsDialog;
}

class GwmGTDROptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmGTDROptionsDialog(QList<GwmLayerGroupItem*> originItemList, QWidget *parent = nullptr);
    ~GwmGTDROptionsDialog();

    GwmAlgorithmMetaGWSS meta() const { return mAlgorithmMeta; }

private:
    Ui::GwmGTDROptionsDialog *ui;
    QList<GwmLayerGroupItem*> mMapLayerList;
    GwmLayerGroupItem* mSelectedLayer = nullptr;
    bool isNumeric(QVariant::Type type);
    GwmBandwidthWeight* mBandwidth;
    GwmAlgorithmMetaGWSS mAlgorithmMeta;

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
    void onDepVarChanged(const int index);
    GwmVariableItemModel* mDepVarModel;
    
    QString crsRotateTheta();
    QString crsRotateP();
    bool bandwidthType();
    IParallelalbe::ParallelType approachType();
    double bandwidthSize();
    gwm::BandwidthWeight::KernelFunctionType bandwidthKernelFunction();
    GwmDistance::DistanceType distanceSourceType();
    QVariant distanceSourceParameters();
    QVariant parallelParameters();

    void updateFieldsAndEnable();
    void updateFields();
    void enableAccept();

    GwmLayerGroupItem *selectedLayer() const;
    void setSelectedLayer(GwmLayerGroupItem *selectedLayer);
};


#endif // GWMGTDROPTIONSDIALOG_H
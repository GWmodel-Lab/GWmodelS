#ifndef GWMGTDROPTIONSDIALOG_H
#define GWMGTDROPTIONSDIALOG_H

#include <QDialog>
#include <QItemSelectionModel>
#include "Model/gwmlayergroupitem.h"
#include "Model/gwmalgorithmmetagtdr.h"
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include "Model/gwmvariableitemmodel.h"
#include "TaskThread/gwmgtdrtaskthread.h"
#include "TaskThread/iparallelable.h"
#include "SpatialWeight/gwmdistance.h"
#include "Model/gwmgtdrparameterspecifiedoptionsmodel.h"

namespace Ui {
class GwmGTDROptionsDialog;
}

class GwmGTDROptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmGTDROptionsDialog(QList<GwmLayerGroupItem*> originItemList, QWidget *parent = nullptr);
    ~GwmGTDROptionsDialog();

    GwmAlgorithmMetaGTDR meta() const { return mAlgorithmMeta; }

private:
    Ui::GwmGTDROptionsDialog *ui;
    QList<GwmLayerGroupItem*> mMapLayerList;
    GwmLayerGroupItem* mSelectedLayer = nullptr;
    bool isNumeric(QVariant::Type type);
    GwmBandwidthWeight* mBandwidth;
    GwmAlgorithmMetaGTDR mAlgorithmMeta;

public slots:
    void layerChanged(const int index);

    void onFixedRadioToggled(bool checked);
    void onVariableRadioToggled(bool checked);
    void onBwSizeAutomaticToggled(bool checked);
    void onBwSizeCustomizeToggled(bool checked);
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
private:
    GwmGTDRParameterSpecifiedOptionsModel* mParameterSpecifiedOptionsModel = nullptr;
    QItemSelectionModel* mParameterSpecifiedOptionsSelectionModel = nullptr;

public slots:
    // ... 现有槽函数 ...
    void onSelectedIndenpendentVariablesChanged();
    void onSpecifiedParameterCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void onBwSizeAdaptiveSizeChanged(int size);
    void onBwSizeFixedSizeChanged(double size);
    void onBwKernelFunctionChanged(int index);

};


#endif // GWMGTDROPTIONSDIALOG_H

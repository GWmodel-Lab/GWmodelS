#ifndef GWMSCALABLEGWROPTIONSDIALOG_H
#define GWMSCALABLEGWROPTIONSDIALOG_H

#include <QDialog>
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include <SpatialWeight/gwmbandwidthweight.h>
#include "TaskThread/gwmscalablegwralgorithm.h"
#include "Model/gwmlayerattributeitemmodel.h"
#include "Model/gwmlayergroupitem.h"

namespace Ui {
class GwmScalableGWROptionsDialog;
}

class GwmScalableGWROptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmScalableGWROptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmScalableGWRAlgorithm* thread,QWidget *parent = nullptr);
    ~GwmScalableGWROptionsDialog();

    bool hasRegressionLayer() const;
    GwmLayerGroupItem *selectedRegressionLayer() const;

private:
    Ui::GwmScalableGWROptionsDialog *ui;
    QList<GwmLayerGroupItem*> mMapLayerList;
    GwmLayerGroupItem* mSelectedLayer = nullptr;
    GwmScalableGWRAlgorithm* mTaskThread = nullptr;
    GwmVariableItemModel* mDepVarModel;
    bool isNumeric(QVariant::Type type);

public slots:
    void layerChanged(const int index);
    void onDepVarChanged(const int index);
    void onFixedRadioToggled(bool checked);
    void onVariableRadioToggled(bool checked);
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
    double bandwidthSize();
    QString bandWidthUnit();
    GwmBandwidthWeight::KernelFunctionType bandwidthKernelFunction();
    QVariant distanceSourceParameters();

    void setTaskThread(GwmScalableGWRAlgorithm* taskThread);
    void updateFieldsAndEnable();
    void updateFields();
    void enableAccept();

    GwmLayerGroupItem *selectedLayer() const;
    void setSelectedLayer(GwmLayerGroupItem *selectedLayer);
    void updatePredict();
    void on_cbkRegressionPoints_toggled(bool checked);
};

#endif // GWMSCALABLEGWROPTIONSDIALOG_H

#ifndef GWMGGWROPTIONSDIALOG_H
#define GWMGGWROPTIONSDIALOG_H

#include <QDialog>
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include "TaskThread/gwmgeneralizedgwralgorithm.h"
#include "Model/gwmvariableitemmodel.h"
#include "Model/gwmlayergroupitem.h"

namespace Ui {
class GwmGGWROptionsDialog;
}

class GwmGGWROptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmGGWROptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmGeneralizedGWRAlgorithm* thread,QWidget *parent = nullptr);
    ~GwmGGWROptionsDialog();

private:
    Ui::GwmGGWROptionsDialog *ui;
    QList<GwmLayerGroupItem*> mMapLayerList;
    GwmLayerGroupItem* mSelectedLayer = nullptr;
    GwmGeneralizedGWRAlgorithm* mTaskThread = nullptr;
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


public:
    QString crsRotateTheta();
    QString crsRotateP();
    bool bandwidthType();
    double bandwidthSize();
    GwmGeneralizedGWRAlgorithm::BandwidthSelectionCriterionType bandwidthSelectionApproach();
    QString bandWidthUnit();
    GwmBandwidthWeight::KernelFunctionType bandwidthKernelFunction();
    QVariant distanceSourceParameters();
    QVariant parallelParameters();
    double epsilonSize();
    QString epsilonUnit();
    GwmGeneralizedGWRAlgorithm::Family distributionFunction();
    int maxiter();

    void setTaskThread(GwmGeneralizedGWRAlgorithm* taskThread);
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

#endif // GWMGGWROPTIONSDIALOG_H

#ifndef GWMSCALABLEGWROPTIONSDIALOG_H
#define GWMSCALABLEGWROPTIONSDIALOG_H

#include <QDialog>
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include "TaskThread/gwmscalablegwrtaskthread.h"
#include "Model/gwmlayerattributeitemmodel.h"
#include "Model/gwmlayergroupitem.h"

namespace Ui {
class GwmScalableGWROptionsDialog;
}

class GwmScalableGWROptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmScalableGWROptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmScalableGWRTaskThread* thread,QWidget *parent = nullptr);
    ~GwmScalableGWROptionsDialog();

private:
    Ui::GwmScalableGWROptionsDialog *ui;
    QList<GwmLayerGroupItem*> mMapLayerList;
    GwmLayerGroupItem* mSelectedLayer = nullptr;
    GwmScalableGWRTaskThread* mTaskThread = nullptr;
    GwmLayerAttributeItemModel* mDepVarModel;
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
    GwmGWRTaskThread::BandwidthType bandwidthType();
    double bandwidthSize();
    QString bandWidthUnit();
    GwmGWRTaskThread::KernelFunction bandwidthKernelFunction();
    GwmGWRTaskThread::DistanceSourceType distanceSourceType();
    QVariant distanceSourceParameters();

    void setTaskThread(GwmGWRTaskThread* taskThread);
    void updateFieldsAndEnable();
    void updateFields();
    void enableAccept();

    GwmLayerGroupItem *selectedLayer() const;
    void setSelectedLayer(GwmLayerGroupItem *selectedLayer);
};

#endif // GWMSCALABLEGWROPTIONSDIALOG_H

#ifndef GWMGWROPTIONSDIALOG_H
#define GWMGWROPTIONSDIALOG_H

#include <QDialog>
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>

namespace Ui {
class GwmGWROptionsDialog;
}

class GwmGWROptionsDialog : public QDialog
{
    Q_OBJECT

public:
    enum GwmBandWidthType{
        Fixed,
        Variable
    };
    enum GwmApproachType{
        None,
        Multithreading,
        GPU
    };

public:
    explicit GwmGWROptionsDialog(QList<QgsMapLayer*> vectorLayerList,QWidget *parent = nullptr);
    ~GwmGWROptionsDialog();

private:
    Ui::GwmGWROptionsDialog *ui;
    QList<QgsMapLayer*> mapLayerList;
    QgsVectorLayer* mLayer = nullptr;
    QStandardItemModel *mAttributeModel = nullptr;
    QStandardItemModel *mSelectedAttributeModel = nullptr;

public slots:
    void layerChanged(const int index);
    void onAddAttributeBtn();
    void onDelAttributeBtn();
    void onFixedRadioToggled(bool checked);
    void onVariableRadioToggled(bool checked);
    void onNoneRadioToggled(bool checked);
    void onMultithreadingRadioToggled(bool checked);
    void onGPURadioToggled(bool checked);
    void onCustomizeRaidoToggled(bool checked);
    void onAutomaticRadioToggled(bool checked);

public:
    QString thetaValue();
    QString pValue();
    GwmBandWidthType bandwidthType();
    GwmApproachType approachType();
    QString bandwidthSize();
    QString bandWidthUnit();
    QString sampleGroupSize();
    QString threadNum();
};

#endif // GWMGWROPTIONSDIALOG_H

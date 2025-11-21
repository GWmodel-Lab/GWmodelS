#ifndef GWMSWIMOPTIONSDIALOG_H
#define GWMSWIMOPTIONSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QList>
#include <QStringList>
#include <qgsvectorlayer.h>
#include <qstandarditemmodel.h>
#include "TaskThread/gwmswimtaskthread.h"
#include "TaskThread/iparallelable.h"
#include "SpatialWeight/gwmdistance.h"
#include "SpatialWeight/gwmbandwidthweight.h"

namespace Ui {
class GwmSWIMOptionsDialog;
}

class GwmSWIMOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmSWIMOptionsDialog(QWidget *parent = nullptr);
    ~GwmSWIMOptionsDialog();

    // 获取参数
    QString csvFilePath() const;
    SWIMMode swimMode() const;
    bool bandwidthType() const;  // true = adaptive, false = fixed
    double bandwidthSize() const;
    GwmBandwidthWeight::KernelFunctionType bandwidthKernelFunction() const;
    GwmDistance::DistanceType distanceSourceType() const;
    QVariant distanceSourceParameters() const;
    QVariant parallelParameters() const;
    IParallelalbe::ParallelType parallelType() const;

    // 设置TaskThread
    void setTaskThread(GwmSWIMTaskThread* taskThread);

    // 更新和验证
    void updateFieldsAndEnable();
    void updateFields();
    void enableAccept();

public slots:
    void onCsvFileOpenClicked();
    void onSwimModeChanged(int index);
    void onFixedRadioToggled(bool checked);
    void onVariableRadioToggled(bool checked);
    void onNoneRadioToggled(bool checked);
    void onMultithreadingRadioToggled(bool checked);
    void onDistTypeCRSToggled(bool checked);
    void onDistTypeMinkowskiToggled(bool checked);
    void onDistTypeDmatToggled(bool checked);
    void onDmatFileOpenClicked();

private:
    Ui::GwmSWIMOptionsDialog *ui;
    GwmSWIMTaskThread* mTaskThread = nullptr;
    GwmBandwidthWeight* mBandwidth = nullptr;
    bool isNumeric(QVariant::Type type);

    QString crsRotateTheta() const;
    QString crsRotateP() const;

    bool loadCsvHeaders(const QString& filePath);
    void populateFieldMappingCombos(const QStringList& headers);
    void clearFieldMappingControls();
    QList<QPair<QString, QComboBox*>> fieldComboPairs() const;
    GwmSWIMFieldMapping currentFieldMapping() const;
    bool isFieldMappingComplete() const;
    QChar detectDelimiter(const QString& line) const;

private:
    QStringList mCsvHeaders;
    QChar mDetectedDelimiter = '\t';
};

#endif // GWMSWIMOPTIONSDIALOG_H


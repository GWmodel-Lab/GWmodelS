#include "gwmswimoptionsdialog.h"
#include "ui_gwmswimoptionsdialog.h"
#include <QFileDialog>
#include <QButtonGroup>
#include <QComboBox>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QMessageBox>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>
#ifdef ENABLE_OpenMP
#include <omp.h>
#endif

#include "SpatialWeight/gwmbandwidthweight.h"
#include "SpatialWeight/gwmcrsdistance.h"
#include "SpatialWeight/gwmdmatdistance.h"
#include "SpatialWeight/gwmminkwoskidistance.h"
#include "SpatialWeight/gwmspatialweight.h"

GwmSWIMOptionsDialog::GwmSWIMOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmSWIMOptionsDialog)
{
    ui->setupUi(this);

    ui->mSwimModeComboBox->addItem(tr("-- Select focus --"), QVariant());
    ui->mSwimModeComboBox->addItem(tr("Origin-based Distance"), static_cast<int>(SWIMMode::OriginFocused));
    ui->mSwimModeComboBox->addItem(tr("Destination-based Distance"), static_cast<int>(SWIMMode::DestinationFocused));
    ui->mSwimModeComboBox->addItem(tr("Flow-based Distance (Euclidean)"), static_cast<int>(SWIMMode::FlowFocusedEuclidean));
    ui->mSwimModeComboBox->addItem(tr("Flow-based Distance (Trajectory)"), static_cast<int>(SWIMMode::FlowFocusedSOP));
    ui->mSwimModeComboBox->setCurrentIndex(0);
    connect(ui->mSwimModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &GwmSWIMOptionsDialog::onSwimModeChanged);

    connect(ui->mCsvFileOpenBtn, &QAbstractButton::clicked, this, &GwmSWIMOptionsDialog::onCsvFileOpenClicked);

    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onVariableRadioToggled);

    QButtonGroup* bwSizeBtnGroup = new QButtonGroup(this);
    bwSizeBtnGroup->addButton(ui->mBwSizeAutomaticRadio);
    bwSizeBtnGroup->addButton(ui->mBwSizeCustomizeRadio);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onAutomaticRadioToggled);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onCustomizeRadioToggled);

    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmSWIMOptionsDialog::onDmatFileOpenClicked);

    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelGPURadio);
    ui->mCalcParallelNoneRadio->setChecked(true);
#ifdef ENABLE_OpenMP
    int cores = omp_get_num_procs();
    ui->mThreadNum->setValue(cores);
    ui->mThreadNum->setMaximum(cores);
#else
    ui->mCalcParallelMultithreadRadio->setEnabled(false);
#endif
    ui->mCalcParallelGPURadio->setEnabled(false);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onMultithreadingRadioToggled);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onNoneRadioToggled);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onGPURadioToggled);
    connect(ui->mCsvFilePathEdit, &QLineEdit::textChanged, this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticApprochCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);

    ui->mBwSizeAdaptiveSize->setMaximum(INT_MAX);
    ui->mBwSizeFixedSize->setMaximum(DBL_MAX);
    ui->mBwTypeAdaptiveRadio->setChecked(true);
    ui->mBwSizeAutomaticRadio->setChecked(true);
    ui->mDistTypeCRSRadio->setChecked(true);
    ui->stackedWidget->setCurrentIndex(0);

    onCustomizeRadioToggled(ui->mBwSizeCustomizeRadio->isChecked());

    clearFieldMappingControls();
    for (const auto& pair : fieldComboPairs())
    {
        connect(pair.second, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    }
    connect(ui->btnAddIndependentVar, &QPushButton::clicked, this, &GwmSWIMOptionsDialog::onAddIndependentVariableClicked);
    connect(ui->btnRemoveIndependentVar, &QPushButton::clicked, this, &GwmSWIMOptionsDialog::onRemoveIndependentVariableClicked);

    updateFieldsAndEnable();
}

GwmSWIMOptionsDialog::~GwmSWIMOptionsDialog()
{
    delete ui;
}

void GwmSWIMOptionsDialog::onCsvFileOpenClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择CSV文件"), "", tr("CSV文件 (*.csv *.txt)"));
    if (!fileName.isEmpty())
    {
        ui->mCsvFilePathEdit->setText(fileName);
        if (!loadCsvHeaders(fileName))
        {
            QMessageBox::warning(this, tr("error"), tr("NO CSV, please check!"));
            clearFieldMappingControls();
        }
        updateFieldsAndEnable();
    }
}

void GwmSWIMOptionsDialog::onSwimModeChanged(int index)
{
    Q_UNUSED(index);
    updateFieldsAndEnable();
}

void GwmSWIMOptionsDialog::onFixedRadioToggled(bool checked)
{
    if (checked)
    {
        ui->mBwSizeSettingStack->setCurrentIndex(1);
        if (ui->mBwSizeCustomizeRadio->isChecked())
        {
            ui->mBwSizeFixedSize->setEnabled(true);
            ui->mBwSizeFixedUnit->setEnabled(true);
            ui->mBwSizeAdaptiveSize->setEnabled(false);
            ui->mBwSizeAdaptiveUnit->setEnabled(false);
        }
    }
    updateFieldsAndEnable();
}

void GwmSWIMOptionsDialog::onVariableRadioToggled(bool checked)
{
    if (checked)
    {
        ui->mBwSizeSettingStack->setCurrentIndex(0);
        if (ui->mBwSizeCustomizeRadio->isChecked())
        {
            ui->mBwSizeFixedSize->setEnabled(false);
            ui->mBwSizeFixedUnit->setEnabled(false);
            ui->mBwSizeAdaptiveSize->setEnabled(true);
            ui->mBwSizeAdaptiveUnit->setEnabled(true);
        }
    }
    updateFieldsAndEnable();
}

void GwmSWIMOptionsDialog::onAutomaticRadioToggled(bool checked)
{
    if (checked)
    {
        ui->mBwSizeAdaptiveSize->setEnabled(false);
        ui->mBwSizeAdaptiveUnit->setEnabled(false);
        ui->mBwSizeFixedSize->setEnabled(false);
        ui->mBwSizeFixedUnit->setEnabled(false);
        ui->mBwSizeAutomaticApprochCombo->setEnabled(true);
        ui->mBwSizeSettingStack->setEnabled(false);
    }
    else
    {
        ui->mBwSizeAutomaticApprochCombo->setEnabled(false);
    }
}

void GwmSWIMOptionsDialog::onCustomizeRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setEnabled(checked);
    if (checked)
    {
        ui->mBwSizeAutomaticApprochCombo->setEnabled(false);
        if (ui->mBwTypeAdaptiveRadio->isChecked())
        {
            ui->mBwSizeAdaptiveSize->setEnabled(true);
            ui->mBwSizeAdaptiveUnit->setEnabled(true);
            ui->mBwSizeFixedSize->setEnabled(false);
            ui->mBwSizeFixedUnit->setEnabled(false);
        }
        else
        {
            ui->mBwSizeAdaptiveSize->setEnabled(false);
            ui->mBwSizeAdaptiveUnit->setEnabled(false);
            ui->mBwSizeFixedSize->setEnabled(true);
            ui->mBwSizeFixedUnit->setEnabled(true);
        }
    }
    else
    {
        ui->mBwSizeAutomaticApprochCombo->setEnabled(ui->mBwSizeAutomaticRadio->isChecked());
        ui->mBwSizeAdaptiveSize->setEnabled(false);
        ui->mBwSizeAdaptiveUnit->setEnabled(false);
        ui->mBwSizeFixedSize->setEnabled(false);
        ui->mBwSizeFixedUnit->setEnabled(false);
        ui->mBwSizeSettingStack->setEnabled(false);
    }
}

void GwmSWIMOptionsDialog::onNoneRadioToggled(bool checked)
{
    if (checked)
    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->mThreadNum->setEnabled(false);
    }
}

void GwmSWIMOptionsDialog::onMultithreadingRadioToggled(bool checked)
{
    if (checked)
    {
        ui->stackedWidget->setCurrentIndex(1);
        ui->mThreadNum->setEnabled(true);
    }
}

void GwmSWIMOptionsDialog::onGPURadioToggled(bool checked)
{
    if (checked)
    {
        ui->stackedWidget->setCurrentIndex(2);
        ui->mThreadNum->setEnabled(false);
    }
}

void GwmSWIMOptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
    {
        ui->mDistParamSettingStack->setCurrentIndex(0);
        ui->mCalcParallelGroup->setEnabled(true);
    }
}

void GwmSWIMOptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
    {
        ui->mDistParamSettingStack->setCurrentIndex(1);
        ui->mCalcParallelGroup->setEnabled(true);
    }
}

void GwmSWIMOptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
    {
        ui->mDistParamSettingStack->setCurrentIndex(2);
        ui->mCalcParallelGroup->setEnabled(false);
    }
}

void GwmSWIMOptionsDialog::onDmatFileOpenClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择距离矩阵文件"), "", tr("所有文件 (*.*)"));
    if (!fileName.isEmpty())
    {
        ui->mDistMatrixFileNameEdit->setText(fileName);
    }
}

QString GwmSWIMOptionsDialog::csvFilePath() const
{
    return ui->mCsvFilePathEdit->text();
}

SWIMMode GwmSWIMOptionsDialog::swimMode() const
{
    int index = ui->mSwimModeComboBox->currentIndex();
    QVariant data = ui->mSwimModeComboBox->itemData(index);
    bool ok = false;
    int value = data.toInt(&ok);
    if (!ok)
    {
        return SWIMMode::OriginFocused;
    }
    return static_cast<SWIMMode>(value);
}

bool GwmSWIMOptionsDialog::hasValidSwimMode() const
{
    int index = ui->mSwimModeComboBox->currentIndex();
    if (index < 0) return false;
    QVariant data = ui->mSwimModeComboBox->itemData(index);
    bool ok = false;
    data.toInt(&ok);
    return ok;
}

bool GwmSWIMOptionsDialog::bandwidthType() const
{
    return ui->mBwTypeAdaptiveRadio->isChecked();
}

double GwmSWIMOptionsDialog::bandwidthSize() const
{
    if (bandwidthType())
    {
        QList<double> units = { 1.0, 10.0, 100.0, 1000.0 };
        int idx = ui->mBwSizeAdaptiveUnit->currentIndex();
        if (idx < 0 || idx >= units.size()) idx = 0;
        return ui->mBwSizeAdaptiveSize->value() * units[idx];
    }
    else
    {
        QList<double> units = { 1.0, 1000.0, 1609.344 };
        int idx = ui->mBwSizeFixedUnit->currentIndex();
        if (idx < 0 || idx >= units.size()) idx = 0;
        return ui->mBwSizeFixedSize->value() * units[idx];
    }
}

GwmBandwidthWeight::KernelFunctionType GwmSWIMOptionsDialog::bandwidthKernelFunction() const
{
    int index = ui->mBwKernelFunctionCombo->currentIndex();
    return static_cast<GwmBandwidthWeight::KernelFunctionType>(index);
}

GwmDistance::DistanceType GwmSWIMOptionsDialog::distanceSourceType() const
{
    if (ui->mDistTypeCRSRadio->isChecked())
        return GwmDistance::DistanceType::CRSDistance;
    else if (ui->mDistTypeMinkowskiRadio->isChecked())
        return GwmDistance::DistanceType::MinkwoskiDistance;
    else if (ui->mDistTypeDmatRadio->isChecked())
        return GwmDistance::DistanceType::DMatDistance;
    else
        return GwmDistance::DistanceType::CRSDistance;
}

QVariant GwmSWIMOptionsDialog::distanceSourceParameters() const
{
    QVariantMap params;
    if (ui->mDistTypeCRSRadio->isChecked() || ui->mDistTypeMinkowskiRadio->isChecked())
    {
        params["theta"] = crsRotateTheta().toDouble();
        params["p"] = crsRotateP().toDouble();
    }
    else if (ui->mDistTypeDmatRadio->isChecked())
    {
        params["file"] = ui->mDistMatrixFileNameEdit->text();
    }
    return params;
}

QVariant GwmSWIMOptionsDialog::parallelParameters() const
{
    QVariantMap params;
    if (ui->mCalcParallelMultithreadRadio->isChecked())
    {
        params["threadNum"] = ui->mThreadNum->value();
    }
    return params;
}

IParallelalbe::ParallelType GwmSWIMOptionsDialog::parallelType() const
{
    if (ui->mCalcParallelMultithreadRadio->isChecked())
        return IParallelalbe::ParallelType::OpenMP;
    else if (ui->mCalcParallelGPURadio->isChecked())
        return IParallelalbe::ParallelType::CUDA;
    else
        return IParallelalbe::ParallelType::SerialOnly;
}

void GwmSWIMOptionsDialog::setTaskThread(GwmSWIMTaskThread* taskThread)
{
    mTaskThread = taskThread;
    if (taskThread)
    {
        // Configure task thread immediately
        taskThread->setCsvFilePath(csvFilePath());
        taskThread->setSWIMMode(swimMode());

        auto bandwidth = new GwmBandwidthWeight(
            bandwidthSize(),
            bandwidthType(),
            bandwidthKernelFunction());

        // SWIM handles real distance computation internally, so build lightweight objects.
        GwmDistance* distanceObj = nullptr;
        QVariant distParams = distanceSourceParameters();
        int tempTotal = 1000;
        switch (distanceSourceType())
        {
        case GwmDistance::DistanceType::CRSDistance:
            distanceObj = new GwmCRSDistance(tempTotal, distParams.toMap().value("theta").toDouble() != 0.0);
            break;
        case GwmDistance::DistanceType::MinkwoskiDistance:
            distanceObj = new GwmMinkwoskiDistance(
                tempTotal,
                distParams.toMap().value("p").toDouble(),
                distParams.toMap().value("theta").toDouble()
                );
            break;
        case GwmDistance::DistanceType::DMatDistance:
            distanceObj = new GwmDMatDistance(tempTotal, distParams.toMap().value("file").toString());
            break;
        default:
            distanceObj = new GwmCRSDistance(tempTotal, false);
            break;
        }

        GwmSpatialWeight spatialWeight(bandwidth, distanceObj);
        taskThread->setSpatialWeight(spatialWeight);
        taskThread->setFieldMapping(currentFieldMapping());
        taskThread->setFieldDelimiter(mDetectedDelimiter);

        bool useAutoBandwidth = ui->mBwSizeAutomaticRadio->isChecked();
        taskThread->setUseBandwidthAuto(useAutoBandwidth);
        if (useAutoBandwidth)
        {
            GwmSWIMTaskThread::BandwidthSelectionCriterionType criterionType =
                ui->mBwSizeAutomaticApprochCombo->currentIndex() == 0
                ? GwmSWIMTaskThread::BandwidthSelectionCriterionType::CV
                : GwmSWIMTaskThread::BandwidthSelectionCriterionType::AICc;
            taskThread->setBandwidthSelectionCriterion(criterionType);
        }

        taskThread->setParallelType(parallelType());
        if (parallelType() == IParallelalbe::ParallelType::OpenMP)
        {
            QVariant parallelParams = parallelParameters();
            taskThread->setOmpThreadNum(parallelParams.toMap().value("threadNum").toInt());
        }
    }
}

void GwmSWIMOptionsDialog::updateFieldsAndEnable()
{
    updateFields();
    enableAccept();
}

void GwmSWIMOptionsDialog::updateFields()
{
    updateCoordinateControlState();
    updateIndependentFieldStates();
}

void GwmSWIMOptionsDialog::enableAccept()
{
    bool enabled = true;

    if (!hasValidSwimMode())
    {
        enabled = false;
    }

    if (ui->mCsvFilePathEdit->text().isEmpty())
    {
        enabled = false;
    }

    if (ui->mBwSizeCustomizeRadio->isChecked())
    {
        if (ui->mBwTypeFixedRadio->isChecked())
        {
            if (ui->mBwSizeFixedSize->value() <= 0)
                enabled = false;
        }
        else if (ui->mBwTypeAdaptiveRadio->isChecked())
        {
            if (ui->mBwSizeAdaptiveSize->value() <= 0)
                enabled = false;
        }
    }

    if (ui->mDistTypeDmatRadio->isChecked())
    {
        if (ui->mDistMatrixFileNameEdit->text().isEmpty())
            enabled = false;
    }

    if (!isFieldMappingComplete())
    {
        enabled = false;
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

bool GwmSWIMOptionsDialog::isNumeric(QVariant::Type type)
{
    switch (type)
    {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::UInt:
    case QVariant::Double:
        return true;
    default:
        return false;
    }
}

QString GwmSWIMOptionsDialog::crsRotateTheta() const
{
    return ui->mThetaValue->text();
}

QString GwmSWIMOptionsDialog::crsRotateP() const
{
    return ui->mPValue->text();
}

bool GwmSWIMOptionsDialog::loadCsvHeaders(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    QTextStream in(&file);
    QString headerLine = in.readLine();
    if (headerLine.isNull())
    {
        return false;
    }
    mDetectedDelimiter = detectDelimiter(headerLine);
    QStringList headers = headerLine.trimmed().split(mDetectedDelimiter, Qt::KeepEmptyParts);
    if (headers.isEmpty())
    {
        return false;
    }
    mCsvHeaders = headers;
    populateFieldMappingCombos(headers);
    return true;
}

void GwmSWIMOptionsDialog::populateFieldMappingCombos(const QStringList& headers)
{
    QSet<int> reservedIndices;
    for (const auto& binding : fieldComboPairs())
    {
        QComboBox* combo = binding.second;
        combo->setEnabled(true);
        combo->clear();
        combo->addItem(tr("-- Select --"), -1);
        for (int i = 0; i < headers.size(); ++i)
        {
            combo->addItem(headers.at(i).trimmed(), i);
        }
        int matchedIndex = -1;
        for (int i = 0; i < headers.size(); ++i)
        {
            if (headers.at(i).trimmed().compare(binding.first, Qt::CaseInsensitive) == 0)
            {
                matchedIndex = i;
                break;
            }
        }
        int selectedIndex = -1;
        if (matchedIndex >= 0)
        {
            int comboIndex = combo->findData(matchedIndex);
            if (comboIndex >= 0)
            {
                combo->setCurrentIndex(comboIndex);
                selectedIndex = matchedIndex;
            }
        }
        if (selectedIndex < 0)
        {
            combo->setCurrentIndex(0);
        }
        else
        {
            reservedIndices.insert(selectedIndex);
        }
    }
    ui->mSwimModeComboBox->setEnabled(true);
    populateIndependentVariableList(headers, reservedIndices);
}

void GwmSWIMOptionsDialog::clearFieldMappingControls()
{
    mCsvHeaders.clear();
    mDetectedDelimiter = '\t';
    ui->mSwimModeComboBox->setEnabled(false);
    ui->cbOriginXField->setEnabled(false);
    ui->cbOriginYField->setEnabled(false);
    ui->cbDestXField->setEnabled(false);
    ui->cbDestYField->setEnabled(false);
    ui->cbFlowVolumeField->setEnabled(false);
    for (const auto& binding : fieldComboPairs())
    {
        QComboBox* combo = binding.second;
        combo->clear();
        combo->addItem(tr("-- Select --"), -1);
        combo->setCurrentIndex(0);
        combo->setEnabled(false);
    }
    if (ui->lwCandidateIndepVars)
    {
        ui->lwCandidateIndepVars->clear();
        ui->lwCandidateIndepVars->setEnabled(false);
    }
    if (ui->lwSelectedIndepVars)
    {
        ui->lwSelectedIndepVars->clear();
        ui->lwSelectedIndepVars->setEnabled(false);
    }
}

QList<QPair<QString, QComboBox*>> GwmSWIMOptionsDialog::fieldComboPairs() const
{
    QList<QPair<QString, QComboBox*>> pairs;
    pairs.append(QPair<QString, QComboBox*>(QStringLiteral("flow_volume"), ui->cbFlowVolumeField));
    pairs.append(QPair<QString, QComboBox*>(QStringLiteral("origin_x"), ui->cbOriginXField));
    pairs.append(QPair<QString, QComboBox*>(QStringLiteral("origin_y"), ui->cbOriginYField));
    pairs.append(QPair<QString, QComboBox*>(QStringLiteral("dest_x"), ui->cbDestXField));
    pairs.append(QPair<QString, QComboBox*>(QStringLiteral("dest_y"), ui->cbDestYField));
    return pairs;
}

void GwmSWIMOptionsDialog::populateIndependentVariableList(const QStringList& headers, const QSet<int>& reservedIndices)
{
    if (!ui->lwCandidateIndepVars || !ui->lwSelectedIndepVars) return;
    ui->lwCandidateIndepVars->clear();
    ui->lwSelectedIndepVars->clear();
    for (int i = 0; i < headers.size(); ++i)
    {
        if (reservedIndices.contains(i)) continue;
        QString header = headers.at(i).trimmed();
        QListWidgetItem* item = createListItemForColumn(header, i);
        ui->lwCandidateIndepVars->addItem(item);
    }
    ui->lwCandidateIndepVars->setEnabled(true);
    ui->lwSelectedIndepVars->setEnabled(true);
}

QList<int> GwmSWIMOptionsDialog::selectedIndependentVariableColumns() const
{
    QList<int> columns;
    if (!ui->lwSelectedIndepVars) return columns;
    for (int i = 0; i < ui->lwSelectedIndepVars->count(); ++i)
    {
        QListWidgetItem* item = ui->lwSelectedIndepVars->item(i);
        if (!item) continue;
        bool ok = false;
        int col = item->data(Qt::UserRole).toInt(&ok);
        if (ok) columns.append(col);
    }
    return columns;
}

QStringList GwmSWIMOptionsDialog::selectedIndependentVariableNames() const
{
    QStringList names;
    if (!ui->lwSelectedIndepVars) return names;
    for (int i = 0; i < ui->lwSelectedIndepVars->count(); ++i)
    {
        QListWidgetItem* item = ui->lwSelectedIndepVars->item(i);
        if (!item) continue;
        names.append(item->text());
    }
    return names;
}

QSet<int> GwmSWIMOptionsDialog::reservedFieldIndices() const
{
    QSet<int> indices;
    for (const auto& binding : fieldComboPairs())
    {
        QComboBox* combo = binding.second;
        if (!combo) continue;
        bool ok = false;
        int idx = combo->currentData().toInt(&ok);
        if (ok && idx >= 0)
        {
            indices.insert(idx);
        }
    }
    return indices;
}

void GwmSWIMOptionsDialog::updateIndependentFieldStates()
{
    if (!ui->lwCandidateIndepVars || !ui->lwSelectedIndepVars) return;
    if (mCsvHeaders.isEmpty()) return;

    QSet<int> reserved = reservedFieldIndices();

    auto removeReservedFromList = [&](QListWidget* list)
    {
        if (!list) return;
        for (int i = list->count() - 1; i >= 0; --i)
        {
            QListWidgetItem* item = list->item(i);
            if (!item) continue;
            bool ok = false;
            int col = item->data(Qt::UserRole).toInt(&ok);
            if (!ok) continue;
            if (reserved.contains(col))
            {
                delete list->takeItem(i);
            }
        }
    };

    removeReservedFromList(ui->lwSelectedIndepVars);
    removeReservedFromList(ui->lwCandidateIndepVars);

    QSet<int> existing;
    auto collectExisting = [&](QListWidget* list)
    {
        if (!list) return;
        for (int i = 0; i < list->count(); ++i)
        {
            QListWidgetItem* item = list->item(i);
            if (!item) continue;
            bool ok = false;
            int col = item->data(Qt::UserRole).toInt(&ok);
            if (ok) existing.insert(col);
        }
    };

    collectExisting(ui->lwSelectedIndepVars);
    collectExisting(ui->lwCandidateIndepVars);

    for (int i = 0; i < mCsvHeaders.size(); ++i)
    {
        if (reserved.contains(i)) continue;
        if (existing.contains(i)) continue;
        QListWidgetItem* item = createListItemForColumn(mCsvHeaders.at(i), i);
        ui->lwCandidateIndepVars->addItem(item);
    }
}

GwmSWIMFieldMapping GwmSWIMOptionsDialog::currentFieldMapping() const
{
    GwmSWIMFieldMapping mapping;
    auto columnFromCombo = [](QComboBox* combo) -> int
    {
        if (!combo) return -1;
        QVariant data = combo->currentData();
        bool ok = false;
        int index = data.toInt(&ok);
        return ok ? index : -1;
    };

    mapping.flowVolume = columnFromCombo(ui->cbFlowVolumeField);
    mapping.originX = columnFromCombo(ui->cbOriginXField);
    mapping.originY = columnFromCombo(ui->cbOriginYField);
    mapping.destX = columnFromCombo(ui->cbDestXField);
    mapping.destY = columnFromCombo(ui->cbDestYField);
    mapping.requireOriginCoords = true;
    mapping.requireDestCoords = true;
    mapping.originValue = findHeaderIndex(QStringLiteral("origin_value"));
    mapping.destValue = findHeaderIndex(QStringLiteral("dest_value"));
    mapping.independentVars = selectedIndependentVariableColumns();
    mapping.independentVarNames = selectedIndependentVariableNames();

    return mapping;
}

bool GwmSWIMOptionsDialog::isFieldMappingComplete() const
{
    return currentFieldMapping().isComplete();
}

QChar GwmSWIMOptionsDialog::detectDelimiter(const QString& line) const
{
    if (line.contains('\t')) return '\t';
    if (line.contains(',')) return ',';
    return '\t';
}

void GwmSWIMOptionsDialog::onAddIndependentVariableClicked()
{
    moveItems(ui->lwCandidateIndepVars, ui->lwSelectedIndepVars);
    updateFieldsAndEnable();
}

void GwmSWIMOptionsDialog::onRemoveIndependentVariableClicked()
{
    moveItems(ui->lwSelectedIndepVars, ui->lwCandidateIndepVars);
    updateFieldsAndEnable();
}

QListWidgetItem* GwmSWIMOptionsDialog::createListItemForColumn(const QString& header, int column) const
{
    QListWidgetItem* item = new QListWidgetItem(header.trimmed());
    item->setData(Qt::UserRole, column);
    return item;
}

void GwmSWIMOptionsDialog::moveItems(QListWidget* from, QListWidget* to)
{
    if (!from || !to) return;
    QList<QListWidgetItem*> selected = from->selectedItems();
    if (selected.isEmpty()) return;
    for (QListWidgetItem* item : selected)
    {
        bool ok = false;
        int col = item->data(Qt::UserRole).toInt(&ok);
        if (!ok) continue;
        QListWidgetItem* newItem = createListItemForColumn(item->text(), col);
        to->addItem(newItem);
        delete from->takeItem(from->row(item));
    }
}

int GwmSWIMOptionsDialog::findHeaderIndex(const QString& name) const
{
    for (int i = 0; i < mCsvHeaders.size(); ++i)
    {
        if (mCsvHeaders.at(i).trimmed().compare(name, Qt::CaseInsensitive) == 0)
        {
            return i;
        }
    }
    return -1;
}

bool GwmSWIMOptionsDialog::modeNeedsOriginCoords() const
{
    Q_UNUSED(this);
    return true;
}

bool GwmSWIMOptionsDialog::modeNeedsDestCoords() const
{
    Q_UNUSED(this);
    return true;
}

void GwmSWIMOptionsDialog::updateCoordinateControlState()
{
    auto enableCombo = [](QComboBox* combo)
    {
        if (combo)
            combo->setEnabled(true);
    };
    enableCombo(ui->cbOriginXField);
    enableCombo(ui->cbOriginYField);
    enableCombo(ui->cbDestXField);
    enableCombo(ui->cbDestYField);
}



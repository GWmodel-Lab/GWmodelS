#include "gwmswimoptionsdialog.h"
#include "ui_gwmswimoptionsdialog.h"
#include <QFileDialog>
#include <QButtonGroup>
#include <QComboBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
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

    // 初始化SWIM模式选择
    ui->mSwimModeComboBox->addItem(tr("Origin-Focused SWIM"), static_cast<int>(SWIMMode::OriginFocused));
    ui->mSwimModeComboBox->addItem(tr("Destination-Focused SWIM"), static_cast<int>(SWIMMode::DestinationFocused));
    ui->mSwimModeComboBox->addItem(tr("Flow-Focused SWIM - Euclidean"), static_cast<int>(SWIMMode::FlowFocusedEuclidean));
    ui->mSwimModeComboBox->addItem(tr("Flow-Focused SWIM - SOP"), static_cast<int>(SWIMMode::FlowFocusedSOP));
    ui->mSwimModeComboBox->setCurrentIndex(0);
    connect(ui->mSwimModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), 
            this, &GwmSWIMOptionsDialog::onSwimModeChanged);

    // CSV文件选择
    connect(ui->mCsvFileOpenBtn, &QAbstractButton::clicked, this, &GwmSWIMOptionsDialog::onCsvFileOpenClicked);

    // 带宽类型选择
    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onVariableRadioToggled);

    // 距离计算部分
    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmSWIMOptionsDialog::onDmatFileOpenClicked);

    // 并行参数设置
    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
    ui->mCalcParallelNoneRadio->setChecked(true);
#ifdef ENABLE_OpenMP
    int cores = omp_get_num_procs();
    ui->mThreadNum->setValue(cores);
    ui->mThreadNum->setMaximum(cores);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onMultithreadingRadioToggled);
#else
    ui->mCalcParallelMultithreadRadio->setEnabled(false);
#endif
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::onNoneRadioToggled);

    // 更新字段和启用状态
    connect(ui->mCsvFilePathEdit, &QLineEdit::textChanged, this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), 
            this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), 
            this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), 
            this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);

    ui->mBwSizeAdaptiveSize->setMaximum(INT_MAX);
    ui->mBwSizeFixedSize->setMaximum(DBL_MAX);
    ui->mDistTypeCRSRadio->setChecked(true);

    clearFieldMappingControls();
    for (const auto& pair : fieldComboPairs())
    {
        connect(pair.second, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &GwmSWIMOptionsDialog::updateFieldsAndEnable);
    }
    
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
            QMessageBox::warning(this, tr("tips"), tr("No csv header!Please check!"));
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
        ui->mBwSizeFixedSize->setEnabled(true);
        ui->mBwSizeFixedUnit->setEnabled(true);
        ui->mBwSizeAdaptiveSize->setEnabled(false);
    }
    updateFieldsAndEnable();
}

void GwmSWIMOptionsDialog::onVariableRadioToggled(bool checked)
{
    if (checked)
    {
        ui->mBwSizeFixedSize->setEnabled(false);
        ui->mBwSizeFixedUnit->setEnabled(false);
        ui->mBwSizeAdaptiveSize->setEnabled(true);
    }
    updateFieldsAndEnable();
}

void GwmSWIMOptionsDialog::onNoneRadioToggled(bool checked)
{
    if (checked)
    {
        ui->mThreadNum->setEnabled(false);
    }
}

void GwmSWIMOptionsDialog::onMultithreadingRadioToggled(bool checked)
{
    if (checked)
    {
        ui->mThreadNum->setEnabled(true);
    }
}

void GwmSWIMOptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
    {
        ui->mThetaValue->setEnabled(true);
        ui->mPValue->setEnabled(true);
        ui->mDistMatrixFileNameEdit->setEnabled(false);
        ui->mDistMatrixFileOpenBtn->setEnabled(false);
    }
}

void GwmSWIMOptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
    {
        ui->mThetaValue->setEnabled(true);
        ui->mPValue->setEnabled(true);
        ui->mDistMatrixFileNameEdit->setEnabled(false);
        ui->mDistMatrixFileOpenBtn->setEnabled(false);
    }
}

void GwmSWIMOptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
    {
        ui->mThetaValue->setEnabled(false);
        ui->mPValue->setEnabled(false);
        ui->mDistMatrixFileNameEdit->setEnabled(true);
        ui->mDistMatrixFileOpenBtn->setEnabled(true);
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
    return static_cast<SWIMMode>(ui->mSwimModeComboBox->itemData(index).toInt());
}

bool GwmSWIMOptionsDialog::bandwidthType() const
{
    return ui->mBwTypeAdaptiveRadio->isChecked();
}

double GwmSWIMOptionsDialog::bandwidthSize() const
{
    if (bandwidthType())
    {
        return ui->mBwSizeAdaptiveSize->value();
    }
    else
    {
        return ui->mBwSizeFixedSize->value();
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
    else
        return IParallelalbe::ParallelType::SerialOnly;
}

void GwmSWIMOptionsDialog::setTaskThread(GwmSWIMTaskThread* taskThread)
{
    mTaskThread = taskThread;
    if (taskThread)
    {
        // 设置参数到TaskThread
        taskThread->setCsvFilePath(csvFilePath());
        taskThread->setSWIMMode(swimMode());
        
        // 创建空间权重
        GwmBandwidthWeight* bandwidth = new GwmBandwidthWeight(
            bandwidthSize(),
            bandwidthType(),
            bandwidthKernelFunction()
        );
        
        // 注意：对于SWIM，我们使用自己的距离计算，这里创建一个默认的距离对象
        // 实际的距离计算在TaskThread中实现
        GwmDistance* distance = nullptr;
        QVariant distParams = distanceSourceParameters();
        // 使用一个临时值作为total，实际计算中不会使用这个距离对象
        int tempTotal = 1000;  // 临时值，SWIM使用自己的距离计算
        switch (distanceSourceType())
        {
        case GwmDistance::DistanceType::CRSDistance:
            distance = new GwmCRSDistance(tempTotal, distParams.toMap().value("theta").toDouble() != 0.0);
            break;
        case GwmDistance::DistanceType::MinkwoskiDistance:
            distance = new GwmMinkwoskiDistance(
                tempTotal,
                distParams.toMap().value("p").toDouble(),
                distParams.toMap().value("theta").toDouble()
            );
            break;
        case GwmDistance::DistanceType::DMatDistance:
            distance = new GwmDMatDistance(tempTotal, distParams.toMap().value("file").toString());
            break;
        default:
            distance = new GwmCRSDistance(tempTotal, false);
            break;
        }
        
        GwmSpatialWeight spatialWeight(bandwidth, distance);
        taskThread->setSpatialWeight(spatialWeight);
        taskThread->setFieldMapping(currentFieldMapping());
        taskThread->setFieldDelimiter(mDetectedDelimiter);
        
        // 设置并行参数
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
    // 更新字段显示
}

void GwmSWIMOptionsDialog::enableAccept()
{
    bool enabled = true;
    
    // 检查CSV文件路径
    if (ui->mCsvFilePathEdit->text().isEmpty())
    {
        enabled = false;
    }
    
    // 检查带宽参数
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
    
    // 检查距离矩阵文件（如果使用DMat）
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
        if (matchedIndex >= 0)
        {
            int comboIndex = combo->findData(matchedIndex);
            if (comboIndex >= 0)
            {
                combo->setCurrentIndex(comboIndex);
            }
        }
        else
        {
            combo->setCurrentIndex(0);
        }
    }
}

void GwmSWIMOptionsDialog::clearFieldMappingControls()
{
    mCsvHeaders.clear();
    mDetectedDelimiter = '\t';
    for (const auto& binding : fieldComboPairs())
    {
        QComboBox* combo = binding.second;
        combo->clear();
        combo->addItem(tr("-- Select --"), -1);
        combo->setCurrentIndex(0);
        combo->setEnabled(false);
    }
}

QList<QPair<QString, QComboBox*>> GwmSWIMOptionsDialog::fieldComboPairs() const
{
    return {
        {QStringLiteral("flow_id"), ui->cbFlowIdField},
        {QStringLiteral("origin_id"), ui->cbOriginIdField},
        {QStringLiteral("dest_id"), ui->cbDestIdField},
        {QStringLiteral("flow_volume"), ui->cbFlowVolumeField},
        {QStringLiteral("origin_value"), ui->cbOriginValueField},
        {QStringLiteral("dest_value"), ui->cbDestValueField},
        {QStringLiteral("origin_x"), ui->cbOriginXField},
        {QStringLiteral("origin_y"), ui->cbOriginYField},
        {QStringLiteral("dest_x"), ui->cbDestXField},
        {QStringLiteral("dest_y"), ui->cbDestYField}
    };
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

    mapping.flowId = columnFromCombo(ui->cbFlowIdField);
    mapping.originId = columnFromCombo(ui->cbOriginIdField);
    mapping.destId = columnFromCombo(ui->cbDestIdField);
    mapping.flowVolume = columnFromCombo(ui->cbFlowVolumeField);
    mapping.originValue = columnFromCombo(ui->cbOriginValueField);
    mapping.destValue = columnFromCombo(ui->cbDestValueField);
    mapping.originX = columnFromCombo(ui->cbOriginXField);
    mapping.originY = columnFromCombo(ui->cbOriginYField);
    mapping.destX = columnFromCombo(ui->cbDestXField);
    mapping.destY = columnFromCombo(ui->cbDestYField);

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


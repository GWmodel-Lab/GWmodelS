#include "gwmgtdroptionsdialog.h"
#include "ui_gwmgtdroptionsdialog.h"
#ifdef ENABLE_OpenMP
#include <omp.h>
#endif
#include <QComboBox>
#include <QButtonGroup>
#include <QFileDialog>
#include <QDialogButtonBox>

#include <SpatialWeight/gwmcrsdistance.h>
#include <SpatialWeight/gwmdmatdistance.h>
#include <SpatialWeight/gwmminkwoskidistance.h>


GwmGTDROptionsDialog::GwmGTDROptionsDialog(QList<GwmLayerGroupItem*> originItemList, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmGTDROptionsDialog),
    mMapLayerList(originItemList),
    mDepVarModel(new GwmVariableItemModel)
{
    ui->setupUi(this);

    //图层选择部分
    for (GwmLayerGroupItem* item : mMapLayerList){
        ui->mLayerComboBox->addItem(item->originChild()->layer()->name());
    }
    ui->mLayerComboBox->setCurrentIndex(-1);


    //连接图层选择部分信号
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::layerChanged);

    //因变量选择部分
    ui->mDepVarComboBox->setCurrentIndex(-1);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::onDepVarChanged);

    //自变量选择部分
    mParameterSpecifiedOptionsModel = new GwmGTDRParameterSpecifiedOptionsModel(this);
    mParameterSpecifiedOptionsSelectionModel = new QItemSelectionModel(mParameterSpecifiedOptionsModel, this);
    ui->lsvParameterSpecifiedParameterList->setModel(mParameterSpecifiedOptionsModel);
    ui->lsvParameterSpecifiedParameterList->setSelectionModel(mParameterSpecifiedOptionsSelectionModel);
    // 连接信号
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGTDROptionsDialog::onSelectedIndenpendentVariablesChanged);
    connect(mParameterSpecifiedOptionsSelectionModel, &QItemSelectionModel::currentChanged, this, &GwmGTDROptionsDialog::onSpecifiedParameterCurrentChanged);

    //带宽类型选择部分
    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onVariableRadioToggled);
    
    QButtonGroup* bwSizeTypeBtnGroup = new QButtonGroup(this);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeAutomaticRadio);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeCustomizeRadio);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onBwSizeAutomaticToggled);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onBwSizeCustomizeToggled);


    //距离计算部分
    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGTDROptionsDialog::onDmatFileOpenClicked);

    //并行参数设置部分
    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
//    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelGPURadio);
#ifdef ENABLE_OpenMP
    int cores = omp_get_num_procs();
    ui->mThreadNum->setValue(cores);
    ui->mThreadNum->setMaximum(cores);    
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onMultithreadingRadioToggled);
#else
    ui->mCalcParallelMultithreadRadio->setEnabled(false);
#endif
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onNoneRadioToggled);
    //    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onGPURadioToggled);
    ui->mCalcParallelGPURadio->hide();

    //更新线程参数信息
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    // connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    // connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    // connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    // connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    // connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGTDROptionsDialog::onBwSizeFixedSizeChanged);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::onBwSizeFixedSizeChanged);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGTDROptionsDialog::onBwSizeAdaptiveSizeChanged);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::onBwSizeAdaptiveSizeChanged);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::onBwKernelFunctionChanged);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThetaValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mPValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileNameEdit, &QLineEdit::textChanged, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThreadNum, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mSampleGroupSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mHatmatrixCheckBox, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);

    ui->mBwSizeAdaptiveSize->setMaximum(INT_MAX);
    ui->mBwSizeFixedSize->setMaximum(DBL_MAX);
    ui->mDistTypeCRSRadio->setChecked(true);
    ui->mBwTypeAdaptiveRadio->setChecked(true);
    ui->mBwSizeAutomaticRadio->setChecked(true);
    ui->mBwSizeSettingStack->setEnabled(ui->mBwSizeCustomizeRadio->isChecked());
    updateFieldsAndEnable();
}

GwmGTDROptionsDialog::~GwmGTDROptionsDialog()
{
    delete ui;
}

bool GwmGTDROptionsDialog::isNumeric(QVariant::Type type)
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

GwmLayerGroupItem *GwmGTDROptionsDialog::selectedLayer() const
{
    return mSelectedLayer;
}

void GwmGTDROptionsDialog::setSelectedLayer(GwmLayerGroupItem *selectedLayer)
{
    mSelectedLayer = selectedLayer;
}

void GwmGTDROptionsDialog::layerChanged(int index)
{
    ui->mIndepVarSelector->layerChanged(mMapLayerList[index]->originChild()->layer());
    if (mSelectedLayer)
    {
        mSelectedLayer = nullptr;
    }
    mSelectedLayer =  mMapLayerList[index];
    QgsFields fieldList = mSelectedLayer->originChild()->layer()->fields();
    ui->mDepVarComboBox->clear();
    ui->mIndepVarSelector->onDepVarChanged("");
    mDepVarModel->clear();
    for (int i = 0; i < fieldList.size(); i++)
    {
        QgsField field = fieldList[i];
        if (isNumeric(field.type()))
        {
            GwmVariable item;
            item.name = field.name();
            item.type = field.type();
            item.index = i;
            item.isNumeric = field.isNumeric();
            mDepVarModel->append(item);
            ui->mDepVarComboBox->addItem(field.name());
        }
    }
}

void GwmGTDROptionsDialog::onDepVarChanged(const int index)
{
    ui->mIndepVarSelector->onDepVarChanged(ui->mDepVarComboBox->itemText(index));
}

QString GwmGTDROptionsDialog::crsRotateTheta()
{
    return ui->mThetaValue->text();
}

QString GwmGTDROptionsDialog::crsRotateP()
{
    return ui->mPValue->text();
}

bool GwmGTDROptionsDialog::bandwidthType()
{
    if(ui->mBwTypeFixedRadio->isChecked()){
        return false;
    }
    else if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return true;
    }
    else return false;
}

IParallelalbe::ParallelType GwmGTDROptionsDialog::approachType()
{
    if(ui->mCalcParallelNoneRadio->isChecked()){
        return IParallelalbe::ParallelType::SerialOnly;
    }
    else if(ui->mCalcParallelMultithreadRadio->isChecked()){
        return IParallelalbe::ParallelType::OpenMP;
    }
    else if(ui->mCalcParallelGPURadio->isChecked()){
        return IParallelalbe::ParallelType::CUDA;
    }
    else return IParallelalbe::ParallelType::SerialOnly;
}


void GwmGTDROptionsDialog::onNoneRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void GwmGTDROptionsDialog::onMultithreadingRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void GwmGTDROptionsDialog::onGPURadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(2);
    }
}


GwmDistance::DistanceType GwmGTDROptionsDialog::distanceSourceType()
{
    if (ui->mDistTypeCRSRadio->isChecked())
        return GwmDistance::DistanceType::OneDimDistance;
    else if (ui->mDistTypeDmatRadio->isChecked())
        return GwmDistance::DistanceType::DMatDistance;
    else if (ui->mDistTypeMinkowskiRadio->isChecked())
        return GwmDistance::DistanceType::MinkwoskiDistance;
    else
        return GwmDistance::DistanceType::OneDimDistance;
}

QVariant GwmGTDROptionsDialog::distanceSourceParameters()
{
    if (ui->mDistTypeDmatRadio->isChecked())
    {
        return ui->mDistMatrixFileNameEdit->text();
    }
    else if (ui->mDistTypeMinkowskiRadio->isChecked())
    {
        QMap<QString, QVariant> parameters;
        parameters["theta"] = ui->mThetaValue->value();
        parameters["p"] = ui->mPValue->value();
        return parameters;
    }

    else return QVariant();
}
void GwmGTDROptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(0);
}

void GwmGTDROptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(1);
}

void GwmGTDROptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(2);
    ui->mCalcParallelGroup->setEnabled(!checked);
}

void GwmGTDROptionsDialog::onDmatFileOpenClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Dmat File"), tr(""), tr("Dmat File (*.dmat)"));
    ui->mDistMatrixFileNameEdit->setText(filePath);
}

void GwmGTDROptionsDialog::onFixedRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(1);
}

void GwmGTDROptionsDialog::onBwSizeAutomaticToggled(bool checked)
{
    if (checked)
        ui->mBwSizeSettingStack->setEnabled(false);
}

void GwmGTDROptionsDialog::onBwSizeCustomizeToggled(bool checked)
{
    ui->mBwSizeSettingStack->setEnabled(checked);
}

void GwmGTDROptionsDialog::onVariableRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(0);
}

void GwmGTDROptionsDialog::onSelectedIndenpendentVariablesChanged()
{
    // 同步独立变量到列表（类似 MultiscaleGWR）
    mParameterSpecifiedOptionsModel->syncWithAttributes(ui->mIndepVarSelector->selectedIndepVarModel());

    // 如果有项目，选中第一个
    if (mParameterSpecifiedOptionsModel->rowCount() > 0)
    {
        QModelIndex firstIndex = mParameterSpecifiedOptionsModel->index(0, 0);
        mParameterSpecifiedOptionsSelectionModel->setCurrentIndex(firstIndex, QItemSelectionModel::SelectCurrent);
    }
}

void GwmGTDROptionsDialog::onSpecifiedParameterCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    // 可以在这里更新右侧编辑控件（如果需要）
    // GTDR 可能不需要，因为可以直接在列表中编辑
    // Q_UNUSED(current);
    // Q_UNUSED(previous);

    // 启用/禁用编辑控件（只有当选中有效项时才启用）
    bool isValid = current.isValid();
    ui->mBwSizeSettingStack->setEnabled(isValid && ui->mBwSizeCustomizeRadio->isChecked());
    ui->mBwKernelFunctionCombo->setEnabled(isValid);

    if (!isValid)
        return;

    // 获取当前选中变量对应的参数选项
    GwmGTDRParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(current);
    if (!option)
        return;

    // 更新右侧编辑控件，显示当前选中变量的参数
    // 注意：带宽值的单位转换
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        // Adaptive 模式：直接显示数值（不需要单位转换，因为 adaptive 是数量）
        ui->mBwSizeAdaptiveSize->setValue(int(option->initialBandwidthSize));
    }
    else if (ui->mBwTypeFixedRadio->isChecked())
    {
        // Fixed 模式：需要根据单位转换
        // 假设默认单位是米，需要根据实际单位转换
        double value = option->initialBandwidthSize;
        QList<double> units = { 1.0, 1000.0, 1609.344 };  // 米、千米、英里
        // 尝试找到合适的单位和值
        int unitIndex = 0;
        if (value >= 1000.0 && value < 1000000.0)
        {
            unitIndex = 1;  // 千米
            value = value / 1000.0;
        }
        else if (value >= 1609.344)
        {
            unitIndex = 2;  // 英里
            value = value / 1609.344;
        }
        ui->mBwSizeFixedSize->setValue(value);
        ui->mBwSizeFixedUnit->setCurrentIndex(unitIndex);
    }

    // 更新核函数类型
    ui->mBwKernelFunctionCombo->setCurrentIndex(static_cast<int>(option->kernel));
}

void GwmGTDROptionsDialog::onBwSizeAdaptiveSizeChanged(int size)
{
    // 获取当前选中的变量
    QModelIndex currentIndex = mParameterSpecifiedOptionsSelectionModel->currentIndex();
    if (!currentIndex.isValid())
        return;

    GwmGTDRParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(currentIndex);
    if (!option)
        return;

    // 更新该变量的初始带宽值
    // Adaptive 模式：需要考虑单位
    QList<double> units = { 1, 10, 100, 1000 };
    double bandwidthValue = size * units[ui->mBwSizeAdaptiveUnit->currentIndex()];
    option->initialBandwidthSize = bandwidthValue;
}

void GwmGTDROptionsDialog::onBwSizeFixedSizeChanged(double size)
{
    // 获取当前选中的变量
    QModelIndex currentIndex = mParameterSpecifiedOptionsSelectionModel->currentIndex();
    if (!currentIndex.isValid())
        return;

    GwmGTDRParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(currentIndex);
    if (!option)
        return;

    // 更新该变量的初始带宽值
    // Fixed 模式：需要考虑单位
    QList<double> units = { 1.0, 1000.0, 1609.344 };  // 米、千米、英里
    double bandwidthValue = size * units[ui->mBwSizeFixedUnit->currentIndex()];
    option->initialBandwidthSize = bandwidthValue;
}

void GwmGTDROptionsDialog::onBwKernelFunctionChanged(int index)
{
    // 获取当前选中的变量
    QModelIndex currentIndex = mParameterSpecifiedOptionsSelectionModel->currentIndex();
    if (!currentIndex.isValid())
        return;

    GwmGTDRParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(currentIndex);
    if (!option)
        return;

    // 更新该变量的核函数类型
    option->kernel = static_cast<gwm::BandwidthWeight::KernelFunctionType>(index);
}

double GwmGTDROptionsDialog::bandwidthSize(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        QList<double> unit = { 1, 10, 100, 1000 };
        return (double)ui->mBwSizeAdaptiveSize->value() * unit[ui->mBwSizeAdaptiveUnit->currentIndex()];
    }
    else
    {
        QList<double> unit = { 1.0, 1000.0, 1609.344 };
        return ui->mBwSizeFixedSize->value() * unit[ui->mBwSizeFixedUnit->currentIndex()];
    }
}


gwm::BandwidthWeight::KernelFunctionType GwmGTDROptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return gwm::BandwidthWeight::KernelFunctionType(kernelSelected);
}

QVariant GwmGTDROptionsDialog::parallelParameters()
{
    if (ui->mCalcParallelGPURadio->isChecked())
    {
        return ui->mSampleGroupSize->value();
    }
    else if (ui->mCalcParallelMultithreadRadio->isChecked())
    {
        return ui->mThreadNum->value();
    }
    else
    {
        return QVariant();
    }
}

void GwmGTDROptionsDialog::updateFieldsAndEnable()
{
    this->updateFields();
    this->enableAccept();
}

void GwmGTDROptionsDialog::updateFields()
{
    QgsVectorLayer* dataLayer;
    // 图层设置
    if (ui->mLayerComboBox->currentIndex() > -1)
    {
        dataLayer = mSelectedLayer->originChild()->layer();
        mAlgorithmMeta.layer = dataLayer;
    }
    else
    {
        return;
    }
    

    if (ui->mDepVarComboBox->currentIndex() > -1)
    {
        mAlgorithmMeta.dependentVariable = mDepVarModel->item(ui->mDepVarComboBox->currentIndex());
    }

    GwmVariableItemModel* selectedIndepVarModel = ui->mIndepVarSelector->selectedIndepVarModel();
    if (selectedIndepVarModel)
    {
        if (selectedIndepVarModel->rowCount() > 0)
        {
            mAlgorithmMeta.independentVariables = selectedIndepVarModel->attributeItemList();
        }
    }

    mAlgorithmMeta.weightType = gwm::Weight::BandwidthWeight;
    mAlgorithmMeta.weightBandwidthSize = bandwidthSize();
    mAlgorithmMeta.weightBandwidthAdaptive = bandwidthType();
    mAlgorithmMeta.weightBandwidthKernel = bandwidthKernelFunction();

    // 距离设置
    if (ui->mDistTypeDmatRadio->isChecked())
    {
        mAlgorithmMeta.distanceType = gwm::Distance::DistanceType::DMatDistance;
        QString filename = ui->mDistMatrixFileNameEdit->text();
        mAlgorithmMeta.distanceDmatFilename = filename.toStdString();
    }
    else if (ui->mDistTypeMinkowskiRadio->isChecked())
    {
        mAlgorithmMeta.distanceType = gwm::Distance::DistanceType::MinkwoskiDistance;
        mAlgorithmMeta.distanceMinkowskiTheta = ui->mThetaValue->value();
        mAlgorithmMeta.distanceMinkowskiPower = ui->mPValue->value();
    }
    else
    {
        mAlgorithmMeta.distanceType = gwm::Distance::DistanceType::OneDimDistance;
        // mAlgorithmMeta.distanceCrsGeographic = dataLayer->crs().isGeographic();
    }

    // 并行设置
    if (ui->mCalcParallelNoneRadio->isChecked())
    {
        mAlgorithmMeta.parallelType = gwm::ParallelType::SerialOnly;
    }
    else if (ui->mCalcParallelMultithreadRadio->isChecked())
    {
        mAlgorithmMeta.parallelType = gwm::ParallelType::OpenMP;
        mAlgorithmMeta.parallelOmpThreads = ui->mThreadNum->value();
    }
    else if (ui->mCalcParallelGPURadio->isChecked() && !ui->mDistTypeDmatRadio->isChecked())
    {
        mAlgorithmMeta.parallelType = gwm::ParallelType::SerialOnly;
    }
    else
    {
        mAlgorithmMeta.parallelType = gwm::ParallelType::SerialOnly;
    }

    // Bandwidth Autoselection Settings
    mAlgorithmMeta.bandwidthAuto = ui->mBwSizeAutomaticRadio->isChecked();
    if (mAlgorithmMeta.bandwidthAuto)
    {
        mAlgorithmMeta.bandwidthCriterionType =
            ui->mBwSizeAutomaticApprochCombo->currentIndex() == 0
                ? gwm::GTDR::BandwidthCriterionType::AIC
                : gwm::GTDR::BandwidthCriterionType::CV;
        mAlgorithmMeta.weightBandwidthSize = bandwidthSize(); // 作为初始值使用，可保留
    }
    else
    {
        mAlgorithmMeta.weightBandwidthSize = bandwidthSize();
    }

    // 读取每个维度的初始带宽值和核函数类型
    mAlgorithmMeta.weightBandwidthSizes.clear();
    mAlgorithmMeta.weightBandwidthKernels.clear();

    for (int i = 0; i < mParameterSpecifiedOptionsModel->rowCount(); ++i)
    {
        GwmGTDRParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(i);
        if (option)
        {
            mAlgorithmMeta.weightBandwidthSizes.append(option->initialBandwidthSize);
            mAlgorithmMeta.weightBandwidthKernels.append(option->kernel);
        }
    }

    // 向后兼容：如果列表为空，使用单个默认值
    if (mAlgorithmMeta.weightBandwidthSizes.isEmpty())
    {
        // 使用当前 UI 中的值作为默认值
        mAlgorithmMeta.weightBandwidthSize = bandwidthSize();
        mAlgorithmMeta.weightBandwidthKernel = bandwidthKernelFunction();
    }
    else
    {
        // 如果列表不为空，也更新单个值（用于向后兼容或作为默认值）
        // 可以选择使用第一个值，或者保持当前 UI 中的值
        mAlgorithmMeta.weightBandwidthSize = mAlgorithmMeta.weightBandwidthSizes.first();
        mAlgorithmMeta.weightBandwidthKernel = mAlgorithmMeta.weightBandwidthKernels.first();
    }

    mAlgorithmMeta.hatmatrix = ui->mHatmatrixCheckBox->isChecked();

}

void GwmGTDROptionsDialog::enableAccept()
{
    QString error;
    if (mAlgorithmMeta.validate(error))
    {
        ui->mCheckMessage->setText(tr("Valid."));
        ui->btbOkCancle->setStandardButtons(QDialogButtonBox::Ok);
        ui->btbOkCancle->addButton(QDialogButtonBox::StandardButton::Cancel);
    }
    else
    {
        ui->mCheckMessage->setText(QString("Invalid: %1").arg(error));
        ui->btbOkCancle->setStandardButtons(QDialogButtonBox::Cancel);
    }
}




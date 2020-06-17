#include "gwmmultiscalegwroptionsdialog.h"
#include "ui_gwmmultiscalegwroptionsdialog.h"
#include <omp.h>
#include <QComboBox>
#include <QButtonGroup>
#include <QFileDialog>

#include <Model/gwmvariableitemmodel.h>

#include <SpatialWeight/gwmcrsdistance.h>
#include <SpatialWeight/gwmdmatdistance.h>
#include <SpatialWeight/gwmminkwoskidistance.h>

GwmMultiscaleGWROptionsDialog::GwmMultiscaleGWROptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmMultiscaleGWRAlgorithm* thread,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmMultiscaleGWROptionsDialog),
    mMapLayerList(originItemList),
    mTaskThread(thread),
    mDepVarModel(new GwmVariableItemModel),
    mParameterSpecifiedOptionsModel(new GwmParameterSpecifiedOptionsModel)
{
    ui->setupUi(this);
    ui->mBwSizeAdaptiveSize->setMaximum(INT_MAX);
    ui->mBwSizeFixedSize->setMaximum(DBL_MAX);

    for (GwmLayerGroupItem* item : mMapLayerList){
        ui->mLayerComboBox->addItem(item->originChild()->layer()->name());
    }
    ui->mLayerComboBox->setCurrentIndex(-1);
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmMultiscaleGWROptionsDialog::layerChanged);

    ui->mDepVarComboBox->setCurrentIndex(-1);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmMultiscaleGWROptionsDialog::onDepVarChanged);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmMultiscaleGWROptionsDialog::onSelectedIndenpendentVariablesChanged);

    ui->mMaxIterationSpb->setMaximum(INT_MAX);
    ui->mNLowerSpb->setMaximum(INT_MAX);
    ui->mBandwidthOptimizeRetrySpb->setMaximum(INT_MAX);

    // Parameter Specified 参数设置
    mParameterSpecifiedOptionsSelectionModel = new QItemSelectionModel(mParameterSpecifiedOptionsModel);
    ui->lsvParameterSpecifiedParameterList->setModel(mParameterSpecifiedOptionsModel);
    ui->lsvParameterSpecifiedParameterList->setSelectionModel(mParameterSpecifiedOptionsSelectionModel);
    connect(mParameterSpecifiedOptionsSelectionModel, &QItemSelectionModel::currentChanged, this, &GwmMultiscaleGWROptionsDialog::onSpecifiedParameterCurrentChanged);

    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::onVariableRadioToggled);


    QButtonGroup* criterionBtnGroup = new QButtonGroup(this);
    criterionBtnGroup->addButton(ui->mCriterionCVRckb);
    criterionBtnGroup->addButton(ui->mCriterionDVCRckb);
    ui->mCriterionCVRckb->setChecked(true);

    QButtonGroup* bwSizeTypeBtnGroup = new QButtonGroup(this);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeAutomaticRadio);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeInitialRadio);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeCustomizeRadio);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::onAutomaticRadioToggled);
    connect(ui->mBwSizeInitialRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::onInitializeRadioToggled);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::onCustomizeRaidoToggled);
    connect(ui->mBwSizeAutomaticApprochCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmMultiscaleGWROptionsDialog::onBwSizeAutomaticApprochChanged);
    connect(ui->mBwSelecionThresholdSpb, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmMultiscaleGWROptionsDialog::onBwSelecionThresholdSpbChanged);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmMultiscaleGWROptionsDialog::onBwSizeAdaptiveSizeChanged);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmMultiscaleGWROptionsDialog::onBwSizeAdaptiveUnitChanged);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmMultiscaleGWROptionsDialog::onBwSizeFixedSizeChanged);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmMultiscaleGWROptionsDialog::onBwSizeFixedUnitChanged);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmMultiscaleGWROptionsDialog::onBwKernelFunctionChanged);
    connect(ui->ckbPredictorCentralization, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::onPredictorCentralizationToggled);

    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmMultiscaleGWROptionsDialog::onDmatFileOpenClicked);

    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
//    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelGPURadio);
    int cores = omp_get_num_procs();
    ui->mThreadNum->setValue(cores);
    ui->mThreadNum->setMaximum(cores);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::onNoneRadioToggled);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::onMultithreadingRadioToggled);
//    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::onGPURadioToggled);
    ui->mCalcParallelGPURadio->hide();

    ui->mBwTypeAdaptiveRadio->setChecked(true);
    ui->mBwSizeAutomaticRadio->setChecked(true);
    ui->mCalcParallelNoneRadio->setChecked(true);
    ui->mDistTypeCRSRadio->setChecked(true);

    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mMaxIterationSpb, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticApprochCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSelecionThresholdSpb, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThetaValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mPValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileNameEdit, &QLineEdit::textChanged, this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThreadNum, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mSampleGroupSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->cbxHatmatrix, &QAbstractButton::toggle, this, &GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable);

    updateFieldsAndEnable();
}

GwmMultiscaleGWROptionsDialog::~GwmMultiscaleGWROptionsDialog()
{
    delete ui;
}

bool GwmMultiscaleGWROptionsDialog::isNumeric(QVariant::Type type)
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

GwmLayerGroupItem *GwmMultiscaleGWROptionsDialog::selectedLayer() const
{
    return mSelectedLayer;
}

void GwmMultiscaleGWROptionsDialog::setSelectedLayer(GwmLayerGroupItem *selectedLayer)
{
    mSelectedLayer = selectedLayer;
}

void GwmMultiscaleGWROptionsDialog::layerChanged(int index)
{
    ui->mIndepVarSelector->layerChanged(mMapLayerList[index]->originChild()->layer());
    if (mSelectedLayer)
    {
        mSelectedLayer = nullptr;
    }
    mSelectedLayer =  mMapLayerList[index];
    QgsFields fieldList = mSelectedLayer->originChild()->layer()->fields();
    ui->mDepVarComboBox->clear();
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
    // 选带宽下限最小值
    ui->mNLowerSpb->setMaximum(mSelectedLayer->originChild()->layer()->featureCount());
}

void GwmMultiscaleGWROptionsDialog::onDepVarChanged(const int index)
{
    ui->mIndepVarSelector->onDepVarChanged(ui->mDepVarComboBox->itemText(index));
}

QString GwmMultiscaleGWROptionsDialog::crsRotateTheta()
{
    return ui->mThetaValue->text();
}

QString GwmMultiscaleGWROptionsDialog::crsRotateP()
{
    return ui->mPValue->text();
}

bool GwmMultiscaleGWROptionsDialog::bandwidthType()
{
    if(ui->mBwTypeFixedRadio->isChecked()){
        return false;
    }
    else if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return true;
    }
    else return true;
}

IParallelalbe::ParallelType GwmMultiscaleGWROptionsDialog::parallelType()
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

void GwmMultiscaleGWROptionsDialog::onNoneRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void GwmMultiscaleGWROptionsDialog::onMultithreadingRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void GwmMultiscaleGWROptionsDialog::onGPURadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void GwmMultiscaleGWROptionsDialog::onAutomaticRadioToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(false);
        ui->mBwSizeAdaptiveUnit->setEnabled(false);
        ui->mBwSizeFixedSize->setEnabled(false);
        ui->mBwSizeFixedUnit->setEnabled(false);
        ui->mBwSizeSettingStack->setEnabled(false);
        ui->mBwSizeAutomaticApprochCombo->setEnabled(true);
        // 记录数据
        GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
        if (option)
        {
            option->bandwidthSeledType = GwmMultiscaleGWRAlgorithm::Null;
        }
    }
}

void GwmMultiscaleGWROptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
    {
        ui->mDistParamSettingStack->setCurrentIndex(0);
        // 记录数据
        GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
        if (option)
        {
            option->distanceType = GwmDistance::DistanceType::CRSDistance;
        }
    }
}

void GwmMultiscaleGWROptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
    {
        ui->mDistParamSettingStack->setCurrentIndex(1);
        // 记录数据
        GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
        if (option)
        {
            option->distanceType = GwmDistance::DistanceType::MinkwoskiDistance;
            option->p = ui->mPValue->value();
            option->theta = ui->mThetaValue->value();
        }
    }
}

void GwmMultiscaleGWROptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
    {
        ui->mDistParamSettingStack->setCurrentIndex(2);
        // 记录数据
        GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
        if (option)
        {
            option->distanceType = GwmDistance::DistanceType::DMatDistance;
            option->dmatFile = ui->mDistMatrixFileNameEdit->text();
        }
    }
    ui->mCalcParallelGroup->setEnabled(!checked);
    ui->mCalcParallelNoneRadio->setChecked(true);
}

void GwmMultiscaleGWROptionsDialog::onDmatFileOpenClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Dmat File"), tr(""), tr("Dmat File (*.dmat)"));
    ui->mDistMatrixFileNameEdit->setText(filePath);
}

void GwmMultiscaleGWROptionsDialog::onSelectedIndenpendentVariablesChanged()
{
    mParameterSpecifiedOptionsModel->syncWithAttributes(ui->mIndepVarSelector->selectedIndepVarModel());
    ui->mNLowerSpb->setMinimum(ui->mIndepVarSelector->selectedIndepVarModel()->rowCount() + 1);
}

void GwmMultiscaleGWROptionsDialog::onSpecifiedParameterCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    ui->mParameterSpecifiedOptionsLayout->setEnabled(current.isValid());
    ui->mPSBandwidthSettingGroup->setEnabled(current.isValid());
    ui->mPSDistanceSettingGroup->setEnabled(current.isValid());
    ui->mPSOtherSettingGroup->setEnabled(current.isValid());

    if (!current.isValid())
        return;

    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(current);
    if (option)
    {
        switch (option->bandwidthSeledType) {
        case GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType::Null:
            ui->mBwSizeAutomaticRadio->setChecked(true);
            break;
        case GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType::Initial:
            ui->mBwSizeInitialRadio->setChecked(true);
            break;
        case GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType::Specified:
            ui->mBwSizeCustomizeRadio->setChecked(true);
            break;
        default:
            break;
        }
        ui->mBwSizeAutomaticApprochCombo->setCurrentIndex(option->approach);
        ui->mBwSelecionThresholdSpb->setValue(-log10(option->threshold));
        if (ui->mBwTypeAdaptiveRadio->isChecked())
        {
            ui->mBwSizeAdaptiveSize->setValue(option->bandwidthSize);
        }
        else if (ui->mBwTypeFixedRadio->isChecked())
        {
            ui->mBwSizeFixedSize->setValue(option->bandwidthSize);
        }
        switch (option->distanceType) {
        case GwmDistance::DistanceType::CRSDistance:
            ui->mDistTypeCRSRadio->setChecked(true);
            break;
        case GwmDistance::DistanceType::MinkwoskiDistance:
            ui->mDistTypeMinkowskiRadio->setChecked(true);
            ui->mPValue->setValue(option->p);
            ui->mThetaValue->setValue(option->theta);
            break;
        case GwmDistance::DistanceType::DMatDistance:
            ui->mDistTypeDmatRadio->setChecked(true);
            ui->mDistMatrixFileNameEdit->setText(option->dmatFile);
            break;
        default:
            break;
        }
        if (current.row() == 0)
        {
            ui->ckbPredictorCentralization->setChecked(false);
            ui->ckbPredictorCentralization->setEnabled(false);
        }
        else
        {
            ui->ckbPredictorCentralization->setEnabled(true);
            ui->ckbPredictorCentralization->setChecked(option->predictorCentralization);
        }
    }
}

void GwmMultiscaleGWROptionsDialog::onBwSizeAutomaticApprochChanged(int index)
{
    // 记录数据
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    if (option)
    {
        option->approach = (GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType)index;
    }
}

void GwmMultiscaleGWROptionsDialog::onBwSizeAdaptiveSizeChanged(int size)
{
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    if (option)
    {
        option->bandwidthSize = bandwidthSize();
    }
}

void GwmMultiscaleGWROptionsDialog::onBwSizeAdaptiveUnitChanged(int index)
{
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    if (option)
    {
        option->bandwidthSize = bandwidthSize();
    }
}

void GwmMultiscaleGWROptionsDialog::onBwSizeFixedSizeChanged(double size)
{
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    if (option)
    {
        option->bandwidthSize = bandwidthSize();
    }
}

void GwmMultiscaleGWROptionsDialog::onBwSizeFixedUnitChanged(int index)
{
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    if (option)
    {
        option->bandwidthSize = bandwidthSize();
    }
}

void GwmMultiscaleGWROptionsDialog::onBwKernelFunctionChanged(int index)
{
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    if (option)
    {
        option->kernel = (GwmBandwidthWeight::KernelFunctionType)index;
    }
}

void GwmMultiscaleGWROptionsDialog::onPredictorCentralizationToggled(bool checked)
{
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    if (option)
    {
        option->predictorCentralization = checked;
    }
}

void GwmMultiscaleGWROptionsDialog::onCustomizeRaidoToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(true);
        ui->mBwSizeAdaptiveUnit->setEnabled(true);
        ui->mBwSizeFixedSize->setEnabled(true);
        ui->mBwSizeFixedUnit->setEnabled(true);
        ui->mBwSizeSettingStack->setEnabled(true);
        ui->mBwSizeAutomaticApprochCombo->setEnabled(false);
        // 记录数据
        GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
        if (option)
        {
            option->bandwidthSeledType = GwmMultiscaleGWRAlgorithm::Specified;
        }
    }
}

void GwmMultiscaleGWROptionsDialog::onInitializeRadioToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(true);
        ui->mBwSizeAdaptiveUnit->setEnabled(true);
        ui->mBwSizeFixedSize->setEnabled(true);
        ui->mBwSizeFixedUnit->setEnabled(true);
        ui->mBwSizeSettingStack->setEnabled(true);
        ui->mBwSizeAutomaticApprochCombo->setEnabled(true);
        // 记录数据
        GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
        if (option)
        {
            option->bandwidthSeledType = GwmMultiscaleGWRAlgorithm::Initial;
        }
    }
}

void GwmMultiscaleGWROptionsDialog::onFixedRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(1);
    for (int i = 0; i < mParameterSpecifiedOptionsModel->rowCount(); ++i)
    {
        mParameterSpecifiedOptionsModel->item(i)->adaptive = false;
    }
}

void GwmMultiscaleGWROptionsDialog::onVariableRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(0);
    for (int i = 0; i < mParameterSpecifiedOptionsModel->rowCount(); ++i)
    {
        mParameterSpecifiedOptionsModel->item(i)->adaptive = true;
    }
}

double GwmMultiscaleGWROptionsDialog::bandwidthSize(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        QList<double> unit = { 1, 10, 100, 1000 };
        return ui->mBwSizeAdaptiveSize->value() * unit[ui->mBwSizeAdaptiveUnit->currentIndex()];
    }
    else
    {
        QList<double> unit = { 1.0, 1000.0, 1609.344 };
        return ui->mBwSizeFixedSize->value() * unit[ui->mBwSizeAdaptiveUnit->currentIndex()];
    }
}

GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType GwmMultiscaleGWROptionsDialog::bandwidthSelectionApproach()
{
    switch (ui->mBwSizeAutomaticApprochCombo->currentIndex())
    {
    case 0:
        return GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType::CV;
    default:
        return GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType::AIC;
    }
}

void GwmMultiscaleGWROptionsDialog::onBwSelecionThresholdSpbChanged(int value)
{
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    option->threshold = pow(10.0, -value);
}

QString GwmMultiscaleGWROptionsDialog::bandWidthUnit(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        return ui->mBwSizeAdaptiveUnit->currentText();
    }
    else
    {
        return ui->mBwSizeFixedUnit->currentText();
    }
}

GwmBandwidthWeight::KernelFunctionType GwmMultiscaleGWROptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return GwmBandwidthWeight::KernelFunctionType(kernelSelected);
}

GwmDistance::DistanceType GwmMultiscaleGWROptionsDialog::distanceSourceType()
{
    if (ui->mDistTypeCRSRadio->isChecked())
        return GwmDistance::DistanceType::CRSDistance;
    else if (ui->mDistTypeDmatRadio->isChecked())
        return GwmDistance::DistanceType::DMatDistance;
    else if (ui->mDistTypeMinkowskiRadio->isChecked())
        return GwmDistance::DistanceType::MinkwoskiDistance;
    else
        return GwmDistance::DistanceType::CRSDistance;
}

QVariant GwmMultiscaleGWROptionsDialog::distanceSourceParameters()
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

QVariant GwmMultiscaleGWROptionsDialog::parallelParameters()
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

void GwmMultiscaleGWROptionsDialog::setTaskThread(GwmMultiscaleGWRAlgorithm* taskThread)
{
    mTaskThread = taskThread;
}

void GwmMultiscaleGWROptionsDialog::updateFieldsAndEnable()
{
    if (this->mTaskThread)
    {
        this->updateFields();
        this->enableAccept();
    }
    else
    {
        ui->mCheckMessage->setText(tr("Task thread is missing."));
    }
}

void GwmMultiscaleGWROptionsDialog::updateFields()
{
    QgsVectorLayer* dataLayer;
    // 图层设置
    if (ui->mLayerComboBox->currentIndex() > -1)
    {
        dataLayer = mSelectedLayer->originChild()->layer();
        mTaskThread->setDataLayer(dataLayer);
    }
    else return;
    // 因变量设置
    if (ui->mDepVarComboBox->currentIndex() > -1)
    {
        mTaskThread->setDependentVariable(mDepVarModel->item(ui->mDepVarComboBox->currentIndex()));
    }
    // 自变量设置
    GwmVariableItemModel* selectedIndepVarModel = ui->mIndepVarSelector->selectedIndepVarModel();
    if (selectedIndepVarModel)
    {
        if (selectedIndepVarModel->rowCount() > 0)
        {
            mTaskThread->setIndependentVariables(selectedIndepVarModel->attributeItemList());
        }
    }
    // 参数设置
    if (ui->mCriterionCVRckb->isChecked())
    {
        mTaskThread->setCriterionType(GwmMultiscaleGWRAlgorithm::CVR);
    }
    else
    {
        mTaskThread->setCriterionType(GwmMultiscaleGWRAlgorithm::dCVR);
    }
    mTaskThread->setMaxIteration(ui->mMaxIterationSpb->value());
    mTaskThread->setCriterionThreshold(pow(10.0, -1.0 * ui->mIterationThresholdSpb->value()));
    mTaskThread->setBandwidthSelectRetryTimes(ui->mBandwidthOptimizeRetrySpb->value());
    mTaskThread->setAdaptiveLower(ui->mNLowerSpb->value());
    // Parameter Specified 设置
    QList<GwmSpatialWeight> spatialWeights;
    QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> initilize;
    QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> approach;
    QList<bool> preditorCentered;
    QList<double> threshold;
    for (int i = 0; i < mParameterSpecifiedOptionsModel->rowCount(); ++i)
    {
        GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(i);
        initilize.append(option->bandwidthSeledType);
        approach.append(option->approach);
        preditorCentered.append(option->predictorCentralization);
        threshold.append(option->threshold);
        GwmSpatialWeight sw;
        GwmBandwidthWeight weight(option->bandwidthSize, option->adaptive, option->kernel);
        sw.setWeight(weight);
        // 距离设置
        int featureCount = dataLayer->featureCount();
        if (ui->mDistTypeDmatRadio->isChecked())
        {
            int featureCount = dataLayer->featureCount();
            GwmDMatDistance distance(featureCount, option->dmatFile);
            sw.setDistance(distance);
        }
        else if (ui->mDistTypeMinkowskiRadio->isChecked())
        {
            GwmMinkwoskiDistance distance(featureCount, option->p, option->theta);
            sw.setDistance(distance);
        }
        else
        {
            GwmCRSDistance distance(featureCount, dataLayer->crs().isGeographic());
            sw.setDistance(distance);
        }
        spatialWeights.append(sw);
    }
    mTaskThread->setSpatialWeights(spatialWeights);
    mTaskThread->setBandwidthInitilize(initilize);
    mTaskThread->setBandwidthSelectionApproach(approach);
    mTaskThread->setPreditorCentered(preditorCentered);
    mTaskThread->setBandwidthSelectThreshold(threshold);
    // 并行设置
    if (ui->mCalcParallelNoneRadio->isChecked())
    {
        mTaskThread->setParallelType(IParallelalbe::SerialOnly);
    }
    else if (ui->mCalcParallelMultithreadRadio->isChecked())
    {
        mTaskThread->setParallelType(IParallelalbe::OpenMP);
        mTaskThread->setOmpThreadNum(ui->mThreadNum->value());
    }
    else if (ui->mCalcParallelGPURadio->isChecked() && !ui->mDistTypeDmatRadio->isChecked())
    {
        mTaskThread->setParallelType(IParallelalbe::CUDA);
    }
    else
    {
        mTaskThread->setParallelType(IParallelalbe::SerialOnly);
    }
    // 其他设置
    mTaskThread->setHasHatMatrix(ui->cbxHatmatrix->isChecked());
}

void GwmMultiscaleGWROptionsDialog::enableAccept()
{
    QString message;
    if (mTaskThread->isValid())
    {
        ui->mCheckMessage->setText(tr("Valid."));
        ui->btbOkCancle->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    }
    else
    {
        ui->mCheckMessage->setText(message);
        ui->btbOkCancle->setStandardButtons(QDialogButtonBox::Cancel);
    }
}

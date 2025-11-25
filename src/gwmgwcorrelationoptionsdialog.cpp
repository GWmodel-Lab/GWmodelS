#include "gwmgwcorrelationoptionsdialog.h"
#include "ui_gwmgwcorrelationoptionsdialog.h"
#ifdef ENABLE_OpenMP
#include <omp.h>
#endif
#include <QComboBox>
#include <QButtonGroup>
#include <QFileDialog>
#include <QDebug>

#include <Model/gwmvariableitemmodel.h>

#include <SpatialWeight/gwmcrsdistance.h>
#include <SpatialWeight/gwmdmatdistance.h>
#include <SpatialWeight/gwmminkwoskidistance.h>
#include "TaskThread/gwmmultiscalegwralgorithm.h"

GwmGWCorrelationOptionsDialog::GwmGWCorrelationOptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmGWCorrelationTaskThread* thread,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmGWCorrelationOptionsDialog),
    mMapLayerList(originItemList),
    mTaskThread(thread),
    mDepVarModel(new GwmVariableItemModel),
    mParameterSpecifiedOptionsModel(new GwmParameterSpecifiedOptionsModel)
{
    mParameterSpecifiedOptionsModel->item(0)->attributeName = ""; //隐藏intercept名字
    ui->setupUi(this);
    ui->mBwSizeAdaptiveSize->setMaximum(INT_MAX);
    ui->mBwSizeFixedSize->setMaximum(DBL_MAX);
    
    //layername读入候选图层
    for (GwmLayerGroupItem* item : mMapLayerList){
        ui->mLayerComboBox->addItem(item->originChild()->layer()->name());
    }
    ui->mLayerComboBox->setCurrentIndex(-1);
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWCorrelationOptionsDialog::layerChanged);
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWCorrelationOptionsDialog::onDepVarChanged);
    //independent选择器监听候选图层的变化
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGWCorrelationOptionsDialog::onSelectedIndenpendentVariablesChanged);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGWCorrelationOptionsDialog::onXchangedToY);

    connect(ui->mIndepVarSelectorY, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGWCorrelationOptionsDialog::onSelectedIndenpendentVariablesChanged);
    // Parameter Specified 参数设置，即变量参数设置区域
    mParameterSpecifiedOptionsSelectionModel = new QItemSelectionModel(mParameterSpecifiedOptionsModel);
    ui->lsvParameterSpecifiedParameterList->setModel(mParameterSpecifiedOptionsModel);
    ui->lsvParameterSpecifiedParameterList->setSelectionModel(mParameterSpecifiedOptionsSelectionModel);
    connect(mParameterSpecifiedOptionsSelectionModel, &QItemSelectionModel::currentChanged, this, &GwmGWCorrelationOptionsDialog::onSpecifiedParameterCurrentChanged);

    //带宽类型选择
    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::onVariableRadioToggled);

    //带宽选择方式 自动选择 || 用户自定义
    QButtonGroup* bwSizeTypeBtnGroup = new QButtonGroup(this);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeAutomaticRadio);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeCustomizeRadio);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::onAutomaticRadioToggled);//自动选择
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::onCustomizeRaidoToggled);//用户自定义
    connect(ui->mBwSizeAutomaticApprochCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWCorrelationOptionsDialog::onBwSizeAutomaticApprochChanged);//自动选择带宽的方式
    
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWCorrelationOptionsDialog::onBwSizeAdaptiveSizeChanged); //带宽类型:邻居
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWCorrelationOptionsDialog::onBwSizeAdaptiveUnitChanged);//带宽单位
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWCorrelationOptionsDialog::onBwSizeFixedSizeChanged);//带宽类型:固定距离
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWCorrelationOptionsDialog::onBwSizeFixedUnitChanged);//贷款单位
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWCorrelationOptionsDialog::onBwKernelFunctionChanged);//核函数类型
    
    //距离矩阵设置
    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGWCorrelationOptionsDialog::onDmatFileOpenClicked);

    //计算方式，多线程等
    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
//    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelGPURadio);
#ifdef ENABLE_OpenMP
    int cores = omp_get_num_procs();
    ui->mThreadNum->setValue(cores);
    ui->mThreadNum->setMaximum(cores);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::onMultithreadingRadioToggled);
#else
    ui->mCalcParallelMultithreadRadio->setEnabled(false);
#endif
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::onNoneRadioToggled);

    ui->mBwTypeAdaptiveRadio->setChecked(true);
    ui->mBwSizeAutomaticRadio->setChecked(true);
    ui->mCalcParallelNoneRadio->setChecked(true);
    ui->mDistTypeCRSRadio->setChecked(true);


    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mIndepVarSelectorY, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticApprochCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
//    connect(ui->mBwSelecionThresholdSpb, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mThetaValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mPValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileNameEdit, &QLineEdit::textChanged, this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    connect(ui->mThreadNum, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
//    connect(ui->mSampleGroupSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWCorrelationOptionsDialog::updateFieldsAndEnable);
    updateFieldsAndEnable();
}


GwmGWCorrelationOptionsDialog::~GwmGWCorrelationOptionsDialog()
{
    delete ui;
}

bool GwmGWCorrelationOptionsDialog::isNumeric(QVariant::Type type)
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

GwmLayerGroupItem *GwmGWCorrelationOptionsDialog::selectedLayer() const
{
    return mSelectedLayer;
}

void GwmGWCorrelationOptionsDialog::setSelectedLayer(GwmLayerGroupItem *selectedLayer)
{
    mSelectedLayer = selectedLayer;
}

void GwmGWCorrelationOptionsDialog::layerChanged(int index)
{
    ui->mIndepVarSelector->layerChanged(mMapLayerList[index]->originChild()->layer());
    ui->mIndepVarSelectorY->layerChanged(mMapLayerList[index]->originChild()->layer());

    if (mSelectedLayer)
    {
        mSelectedLayer = nullptr;
    }
    mSelectedLayer =  mMapLayerList[index];
    QgsFields fieldList = mSelectedLayer->originChild()->layer()->fields();
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
        }
    }
}

void GwmGWCorrelationOptionsDialog::onDepVarChanged(const int index)
{
    //用于触发自变量选择框刷新
    ui->mIndepVarSelector->onDepVarChanged("");
    ui->mIndepVarSelectorY->onIndepVarChanged(ui->mIndepVarSelector->mSelectedIndepVarModel);
}

QString GwmGWCorrelationOptionsDialog::crsRotateTheta()
{
    return ui->mThetaValue->text();
}

QString GwmGWCorrelationOptionsDialog::crsRotateP()
{
    return ui->mPValue->text();
}

//带宽计算方式
bool GwmGWCorrelationOptionsDialog::bandwidthType()
{
    if(ui->mBwTypeFixedRadio->isChecked()){
        return false;
    }
    else if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return true;
    }
    else return true;
}

//多线程
IParallelalbe::ParallelType GwmGWCorrelationOptionsDialog::parallelType()
{
    if(ui->mCalcParallelNoneRadio->isChecked()){
        return IParallelalbe::ParallelType::SerialOnly;
    }
    else if(ui->mCalcParallelMultithreadRadio->isChecked()){
        return IParallelalbe::ParallelType::OpenMP;
    }
//    else if(ui->mCalcParallelGPURadio->isChecked()){
//        return IParallelalbe::ParallelType::CUDA;
//    }
    else return IParallelalbe::ParallelType::SerialOnly;
}

//切换计算方式显示区内容
void GwmGWCorrelationOptionsDialog::onNoneRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void GwmGWCorrelationOptionsDialog::onMultithreadingRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void GwmGWCorrelationOptionsDialog::onGPURadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(2);
    }
}

//记录: 自动选择带宽还是用户定义
void GwmGWCorrelationOptionsDialog::onAutomaticRadioToggled(bool checked)
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

//
void GwmGWCorrelationOptionsDialog::onDistTypeCRSToggled(bool checked)
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

void GwmGWCorrelationOptionsDialog::onDistTypeMinkowskiToggled(bool checked)
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

void GwmGWCorrelationOptionsDialog::onDistTypeDmatToggled(bool checked)
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

void GwmGWCorrelationOptionsDialog::onDmatFileOpenClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Dmat File"), tr(""), tr("Dmat File (*.dmat)"));
    ui->mDistMatrixFileNameEdit->setText(filePath);
}

void GwmGWCorrelationOptionsDialog::onSelectedIndenpendentVariablesChanged()
{
    GwmVariableItemModel *newDoubleVarsModel = new GwmVariableItemModel(ui->mIndepVarSelector->selectedIndepVarModel(), ui->mIndepVarSelectorY->selectedIndepVarModel());

    if(newDoubleVarsModel->rowCount() > 0){
        mParameterSpecifiedOptionsModel->syncWithAttributes(newDoubleVarsModel);
        //使用GWCorrelations时没有intercept， 直接把模型第一行去掉
        mParameterSpecifiedOptionsModel->mItemUnshift();
    }

}
void GwmGWCorrelationOptionsDialog::onXchangedToY()
{
    ui->mIndepVarSelectorY->onIndepVarChanged(ui->mIndepVarSelector->mSelectedIndepVarModel);
}

//重点部分:挨个设置变量的计算参数
void GwmGWCorrelationOptionsDialog::onSpecifiedParameterCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    ui->mParameterSpecifiedOptionsLayout->setEnabled(current.isValid());
    ui->mPSBandwidthSettingGroup->setEnabled(current.isValid());
    ui->mPSDistanceSettingGroup->setEnabled(current.isValid());

    if (!current.isValid())
        return;

    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(current);
    if (option)
    {
        option->adaptive ? ui->mBwTypeAdaptiveRadio->setChecked(true) : ui->mBwTypeFixedRadio->setChecked(true);
        switch (option->bandwidthSeledType) {
        case GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType::Null:
            ui->mBwSizeAutomaticRadio->setChecked(true);
            break;
        case GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType::Specified:
            ui->mBwSizeCustomizeRadio->setChecked(true);
            break;
        default:
            break;
        }
        ui->mBwSizeAutomaticApprochCombo->setCurrentIndex(option->approach);
        if (ui->mBwTypeAdaptiveRadio->isChecked())
        {
            ui->mBwSizeAdaptiveSize->setValue(option->bandwidthSize);
        }
        else if (ui->mBwTypeFixedRadio->isChecked())
        {
            ui->mBwSizeFixedSize->setValue(option->bandwidthSize);
        }
        ui->mBwKernelFunctionCombo->setCurrentIndex(int(option->kernel));
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
    }
}

//记录:带宽的的值，自动选择或手动选择的值全部在这个地方设置
void GwmGWCorrelationOptionsDialog::onBwSizeAutomaticApprochChanged(int index)
{
    // 记录数据
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    if (option)
    {
        //todo
        option->approach = (GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType)index;
    }
}

void GwmGWCorrelationOptionsDialog::onBwSizeAdaptiveSizeChanged(int size)
{
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    if (option)
    {
        option->bandwidthSize = bandwidthSize();
    }
}

void GwmGWCorrelationOptionsDialog::onBwSizeAdaptiveUnitChanged(int index)
{
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    if (option)
    {
        option->bandwidthSize = bandwidthSize();
    }
}

void GwmGWCorrelationOptionsDialog::onBwSizeFixedSizeChanged(double size)
{
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    if (option)
    {
        option->bandwidthSize = bandwidthSize();
    }
}

void GwmGWCorrelationOptionsDialog::onBwSizeFixedUnitChanged(int index)
{
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    if (option)
    {
        option->bandwidthSize = bandwidthSize();
    }
}

void GwmGWCorrelationOptionsDialog::onBwKernelFunctionChanged(int index)
{
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    if (option)
    {
        option->kernel = (GwmBandwidthWeight::KernelFunctionType)index;
    }
}

void GwmGWCorrelationOptionsDialog::onPredictorCentralizationToggled(bool checked)
{
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    if (option)
    {
        option->predictorCentralization = checked;
    }
}

void GwmGWCorrelationOptionsDialog::onCustomizeRaidoToggled(bool checked)
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
            //todo
            option->bandwidthSeledType = GwmMultiscaleGWRAlgorithm::Specified;
        }
    }
}

//这里设置各个变量的adaptive和fixed
void GwmGWCorrelationOptionsDialog::onFixedRadioToggled(bool checked)
{
   ui->mBwSizeSettingStack->setCurrentIndex(1);
   GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
   if(option)
   {
       option->adaptive =false;
   }
}

void GwmGWCorrelationOptionsDialog::onVariableRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(0);
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    if(option)
    {
        option->adaptive =true;
    }
//    for (int i = 0; i < mParameterSpecifiedOptionsModel->rowCount(); ++i)
//    {
//        mParameterSpecifiedOptionsModel->item(i)->adaptive = true;
//    }
}

double GwmGWCorrelationOptionsDialog::bandwidthSize(){
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

GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType GwmGWCorrelationOptionsDialog::bandwidthSelectionApproach()
{
    switch (ui->mBwSizeAutomaticApprochCombo->currentIndex())
    {
    case 0:
        return GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType::CV;
    default:
        return GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType::AIC;
    }
}

void GwmGWCorrelationOptionsDialog::onBwSelecionThresholdSpbChanged(int value)
{
    GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(mParameterSpecifiedOptionsSelectionModel->currentIndex());
    option->threshold = pow(10.0, -value);
}

//带宽的类型 邻居个数或距离带宽
QString GwmGWCorrelationOptionsDialog::bandWidthUnit(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        return ui->mBwSizeAdaptiveUnit->currentText();
    }
    else
    {
        return ui->mBwSizeFixedUnit->currentText();
    }
}

GwmBandwidthWeight::KernelFunctionType GwmGWCorrelationOptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return GwmBandwidthWeight::KernelFunctionType(kernelSelected);
}

GwmDistance::DistanceType GwmGWCorrelationOptionsDialog::distanceSourceType()
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

QVariant GwmGWCorrelationOptionsDialog::distanceSourceParameters()
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

QVariant GwmGWCorrelationOptionsDialog::parallelParameters()
{
/*    if (ui->mCalcParallelGPURadio->isChecked())
    {
        return ui->mSampleGroupSize->value();
    }
    else*/ if (ui->mCalcParallelMultithreadRadio->isChecked())
    {
        return ui->mThreadNum->value();
    }
    else
    {
        return QVariant();
    }
}

void GwmGWCorrelationOptionsDialog::setTaskThread(GwmGWCorrelationTaskThread* taskThread)
{
    mTaskThread = taskThread;
}

void GwmGWCorrelationOptionsDialog::updateFieldsAndEnable()
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

//将计算参数传给计算线程
void GwmGWCorrelationOptionsDialog::updateFields()
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
//    if (ui->mDepVarComboBox->currentIndex() > -1)
//    {
//        mTaskThread->setDependentVariable(mDepVarModel->item(ui->mDepVarComboBox->currentIndex()));
//    }
    // 自变量设置
    GwmVariableItemModel* selectedIndepVarModel = ui->mIndepVarSelector->selectedIndepVarModel();
    GwmVariableItemModel* selectedIndepVarModelY = ui->mIndepVarSelectorY->selectedIndepVarModel();
    if (selectedIndepVarModel && selectedIndepVarModelY)
    {
        if (selectedIndepVarModel->rowCount() > 0 && selectedIndepVarModelY->rowCount() > 0)
        {
            mTaskThread->setVariables(selectedIndepVarModel->attributeItemList());
            mTaskThread->setVariablesY(selectedIndepVarModelY->attributeItemList());
        }
    }

    // Parameter Specified 设置
    QList<GwmSpatialWeight> spatialWeights;
    QList<GwmMultiscaleGWRAlgorithm::BandwidthInitilizeType> initilize;
    QList<GwmMultiscaleGWRAlgorithm::BandwidthSelectionCriterionType> approach;
    QList<double> threshold;
    QList<bool> adaptive;

    for (int i = 0; i < mParameterSpecifiedOptionsModel->rowCount(); ++i)
    {
        GwmParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(i);
        initilize.append(option->bandwidthSeledType);
        approach.append(option->approach);
        threshold.append(option->threshold);
        adaptive.append(option->adaptive);
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


//     并行设置
    if (ui->mCalcParallelNoneRadio->isChecked())
    {
        mTaskThread->setParallelType(IParallelalbe::SerialOnly);
    }
    else if (ui->mCalcParallelMultithreadRadio->isChecked())
    {
        mTaskThread->setParallelType(IParallelalbe::OpenMP);
        mTaskThread->setOmpThreadNum(ui->mThreadNum->value());
    }
//    else if (ui->mCalcParallelGPURadio->isChecked() && !ui->mDistTypeDmatRadio->isChecked())
//    {
//        mTaskThread->setParallelType(IParallelalbe::CUDA);
//    }
    else
    {
        mTaskThread->setParallelType(IParallelalbe::SerialOnly);
    }
}

void GwmGWCorrelationOptionsDialog::enableAccept()
{
    QString message;
    if (mTaskThread->isValid())
//    if(true)
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


#include "gwmgwaverageoptionsdialog.h"
#include "ui_gwmgwaverageoptionsdialog.h"
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


GwmGWaverageOptionsDialog::GwmGWaverageOptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmGWaverageTaskThread* thread,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmGWaverageOptionsDialog),
    mMapLayerList(originItemList),
    mTaskThread(thread)
{
    ui->setupUi(this);

    //图层选择部分
    for (GwmLayerGroupItem* item : mMapLayerList){
        ui->mLayerComboBox->addItem(item->originChild()->layer()->name());
    }
    ui->mLayerComboBox->setCurrentIndex(-1);


    //连接图层选择部分信号
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWaverageOptionsDialog::layerChanged);

    //带宽类型选择部分
    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::onVariableRadioToggled);


    //距离计算部分
    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGWaverageOptionsDialog::onDmatFileOpenClicked);

    //并行参数设置部分
    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
//    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelGPURadio);
#ifdef ENABLE_OpenMP
    int cores = omp_get_num_procs();
    ui->mThreadNum->setValue(cores);
    ui->mThreadNum->setMaximum(cores);    
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::onMultithreadingRadioToggled);
#else
    ui->mCalcParallelMultithreadRadio->setEnabled(false);
#endif
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::onNoneRadioToggled);
    //    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::onGPURadioToggled);
    ui->mCalcParallelGPURadio->hide();

    //更新线程参数信息
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mThetaValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mPValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileNameEdit, &QLineEdit::textChanged, this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mThreadNum, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mSampleGroupSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mQuantileCheckBox, &QAbstractButton::toggle, this, &GwmGWaverageOptionsDialog::updateFieldsAndEnable);

    ui->mBwSizeAdaptiveSize->setMaximum(INT_MAX);
    ui->mBwSizeFixedSize->setMaximum(DBL_MAX);
    ui->mDistTypeCRSRadio->setChecked(true);
    updateFieldsAndEnable();
}

GwmGWaverageOptionsDialog::~GwmGWaverageOptionsDialog()
{
    delete ui;
}

bool GwmGWaverageOptionsDialog::isNumeric(QVariant::Type type)
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

GwmLayerGroupItem *GwmGWaverageOptionsDialog::selectedLayer() const
{
    return mSelectedLayer;
}

void GwmGWaverageOptionsDialog::setSelectedLayer(GwmLayerGroupItem *selectedLayer)
{
    mSelectedLayer = selectedLayer;
}

void GwmGWaverageOptionsDialog::layerChanged(int index)
{
    ui->mIndepVarSelector->layerChanged(mMapLayerList[index]->originChild()->layer());
    if (mSelectedLayer)
    {
        mSelectedLayer = nullptr;
    }
    mSelectedLayer =  mMapLayerList[index];
    ui->mIndepVarSelector->onDepVarChanged("");
}

QString GwmGWaverageOptionsDialog::crsRotateTheta()
{
    return ui->mThetaValue->text();
}

QString GwmGWaverageOptionsDialog::crsRotateP()
{
    return ui->mPValue->text();
}

bool GwmGWaverageOptionsDialog::bandwidthType()
{
    if(ui->mBwTypeFixedRadio->isChecked()){
        return false;
    }
    else if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return true;
    }
    else return false;
}

IParallelalbe::ParallelType GwmGWaverageOptionsDialog::approachType()
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


void GwmGWaverageOptionsDialog::onNoneRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void GwmGWaverageOptionsDialog::onMultithreadingRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void GwmGWaverageOptionsDialog::onGPURadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(2);
    }
}


GwmDistance::DistanceType GwmGWaverageOptionsDialog::distanceSourceType()
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

QVariant GwmGWaverageOptionsDialog::distanceSourceParameters()
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
void GwmGWaverageOptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(0);
}

void GwmGWaverageOptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(1);
}

void GwmGWaverageOptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(2);
    ui->mCalcParallelGroup->setEnabled(!checked);
}

void GwmGWaverageOptionsDialog::onDmatFileOpenClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Dmat File"), tr(""), tr("Dmat File (*.dmat)"));
    ui->mDistMatrixFileNameEdit->setText(filePath);
}

void GwmGWaverageOptionsDialog::onFixedRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(1);
}

void GwmGWaverageOptionsDialog::onVariableRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(0);
}

double GwmGWaverageOptionsDialog::bandwidthSize(){
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


GwmBandwidthWeight::KernelFunctionType GwmGWaverageOptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return GwmBandwidthWeight::KernelFunctionType(kernelSelected);
}

QVariant GwmGWaverageOptionsDialog::parallelParameters()
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

void GwmGWaverageOptionsDialog::setTaskThread(GwmGWaverageTaskThread *taskThread)
{
    mTaskThread = taskThread;
}

void GwmGWaverageOptionsDialog::updateFieldsAndEnable()
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

void GwmGWaverageOptionsDialog::updateFields()
{
    QgsVectorLayer* dataLayer;
    // 图层设置
    if (ui->mLayerComboBox->currentIndex() > -1)
    {
        dataLayer = mSelectedLayer->originChild()->layer();
        mTaskThread->setDataLayer(dataLayer);
    }
    else
    {
        return;
    }


    GwmVariableItemModel* selectedIndepVarModel = ui->mIndepVarSelector->selectedIndepVarModel();
    if (selectedIndepVarModel)
    {
        if (selectedIndepVarModel->rowCount() > 0)
        {
            mTaskThread->setVariables(selectedIndepVarModel->attributeItemList());
        }
    }

    GwmSpatialWeight spatialWeight;
    GwmBandwidthWeight weight(bandwidthSize(), bandwidthType(), bandwidthKernelFunction());
    mTaskThread->setBandwidth(new GwmBandwidthWeight(bandwidthSize(), bandwidthType(), bandwidthKernelFunction()));
    spatialWeight.setWeight(weight);
    // 距离设置
    int featureCount = dataLayer->featureCount();
    if (ui->mDistTypeDmatRadio->isChecked())
    {
        QString filename = ui->mDistMatrixFileNameEdit->text();
        GwmDMatDistance distance(featureCount, filename);
        spatialWeight.setDistance(distance);
    }
    else if (ui->mDistTypeMinkowskiRadio->isChecked())
    {
        double theta = ui->mThetaValue->value();
        double p = ui->mPValue->value();
        GwmMinkwoskiDistance distance(featureCount, p, theta);
        spatialWeight.setDistance(distance);
    }
    else
    {
        GwmCRSDistance distance(featureCount, dataLayer->crs().isGeographic());
        spatialWeight.setDistance(distance);
    }
    mTaskThread->setSpatialWeight(spatialWeight);

    mTaskThread->setQuantile(ui->mQuantileCheckBox->isChecked());
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

}

void GwmGWaverageOptionsDialog::enableAccept()
{
    if (mTaskThread->isValid())
    {
        ui->mCheckMessage->setText(tr("Valid."));
        ui->btbOkCancle->setStandardButtons(QDialogButtonBox::Ok);
        ui->btbOkCancle->addButton(QDialogButtonBox::StandardButton::Cancel);
    }
    else
    {
        ui->mCheckMessage->setText("Invalid.");
        ui->btbOkCancle->setStandardButtons(QDialogButtonBox::Cancel);
    }
}



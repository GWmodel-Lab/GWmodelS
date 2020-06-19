#include "gwmgwssoptionsdialog.h"
#include "ui_gwmgwssoptionsdialog.h"
#include <omp.h>
#include <QComboBox>
#include <QButtonGroup>
#include <QFileDialog>
#include <SpatialWeight/gwmcrsdistance.h>
#include <SpatialWeight/gwmdmatdistance.h>
#include <SpatialWeight/gwmminkwoskidistance.h>


GwmGWSSOptionsDialog::GwmGWSSOptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmGWSSTaskThread* thread,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmGWSSOptionsDialog),
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
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWSSOptionsDialog::layerChanged);

    //带宽类型选择部分
    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::onVariableRadioToggled);


    //距离计算部分
    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGWSSOptionsDialog::onDmatFileOpenClicked);

    //并行参数设置部分
    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelGPURadio);
    int cores = omp_get_num_procs();
    ui->mThreadNum->setValue(cores);
    ui->mThreadNum->setMaximum(cores);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::onNoneRadioToggled);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::onMultithreadingRadioToggled);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::onGPURadioToggled);

    //更新线程参数信息
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mThetaValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mPValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileNameEdit, &QLineEdit::textChanged, this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mThreadNum, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mSampleGroupSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);
    connect(ui->mQuantileCheckBox, &QAbstractButton::toggle, this, &GwmGWSSOptionsDialog::updateFieldsAndEnable);

    ui->mBwSizeAdaptiveSize->setMaximum(INT_MAX);
    ui->mBwSizeFixedSize->setMaximum(DBL_MAX);
    ui->mDistTypeCRSRadio->setChecked(true);
    updateFieldsAndEnable();
}

GwmGWSSOptionsDialog::~GwmGWSSOptionsDialog()
{
    delete ui;
}

bool GwmGWSSOptionsDialog::isNumeric(QVariant::Type type)
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

GwmLayerGroupItem *GwmGWSSOptionsDialog::selectedLayer() const
{
    return mSelectedLayer;
}

void GwmGWSSOptionsDialog::setSelectedLayer(GwmLayerGroupItem *selectedLayer)
{
    mSelectedLayer = selectedLayer;
}

void GwmGWSSOptionsDialog::layerChanged(int index)
{
    ui->mIndepVarSelector->layerChanged(mMapLayerList[index]->originChild()->layer());
    if (mSelectedLayer)
    {
        mSelectedLayer = nullptr;
    }
    mSelectedLayer =  mMapLayerList[index];
    ui->mIndepVarSelector->onDepVarChanged("");
}

QString GwmGWSSOptionsDialog::crsRotateTheta()
{
    return ui->mThetaValue->text();
}

QString GwmGWSSOptionsDialog::crsRotateP()
{
    return ui->mPValue->text();
}

bool GwmGWSSOptionsDialog::bandwidthType()
{
    if(ui->mBwTypeFixedRadio->isChecked()){
        return false;
    }
    else if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return true;
    }
    else return false;
}

GwmGWRTaskThread::ParallelMethod GwmGWSSOptionsDialog::approachType()
{
    if(ui->mCalcParallelNoneRadio->isChecked()){
        return GwmGWRTaskThread::ParallelMethod::None;
    }
    else if(ui->mCalcParallelMultithreadRadio->isChecked()){
        return GwmGWRTaskThread::ParallelMethod::Multithread;
    }
    else if(ui->mCalcParallelGPURadio->isChecked()){
        return GwmGWRTaskThread::ParallelMethod::GPU;
    }
    else return GwmGWRTaskThread::ParallelMethod::None;
}


void GwmGWSSOptionsDialog::onNoneRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void GwmGWSSOptionsDialog::onMultithreadingRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void GwmGWSSOptionsDialog::onGPURadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(2);
    }
}


GwmGWRTaskThread::DistanceSourceType GwmGWSSOptionsDialog::distanceSourceType()
{
    if (ui->mDistTypeCRSRadio->isChecked())
        return GwmGWRTaskThread::DistanceSourceType::CRS;
    else if (ui->mDistTypeDmatRadio->isChecked())
        return GwmGWRTaskThread::DistanceSourceType::DMatFile;
    else if (ui->mDistTypeMinkowskiRadio->isChecked())
        return GwmGWRTaskThread::DistanceSourceType::Minkowski;
    else
        return GwmGWRTaskThread::DistanceSourceType::CRS;
}

QVariant GwmGWSSOptionsDialog::distanceSourceParameters()
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
void GwmGWSSOptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(0);
}

void GwmGWSSOptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(1);
}

void GwmGWSSOptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(2);
    ui->mCalcParallelGroup->setEnabled(!checked);
}

void GwmGWSSOptionsDialog::onDmatFileOpenClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Dmat File"), tr(""), tr("Dmat File (*.dmat)"));
    ui->mDistMatrixFileNameEdit->setText(filePath);
}

void GwmGWSSOptionsDialog::onFixedRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(1);
}

void GwmGWSSOptionsDialog::onVariableRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(0);
}

double GwmGWSSOptionsDialog::bandwidthSize(){
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


GwmBandwidthWeight::KernelFunctionType GwmGWSSOptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return GwmBandwidthWeight::KernelFunctionType(kernelSelected);
}

GwmGWRTaskThread::ParallelMethod GwmGWSSOptionsDialog::parallelMethod()
{
    if (ui->mCalcParallelMultithreadRadio->isChecked())
    {
        return GwmGWRTaskThread::ParallelMethod::Multithread;
    }
    else if (ui->mCalcParallelGPURadio->isChecked())
    {
        return GwmGWRTaskThread::ParallelMethod::GPU;
    }
    else
    {
        return GwmGWRTaskThread::ParallelMethod::None;
    }
}

QVariant GwmGWSSOptionsDialog::parallelParameters()
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

void GwmGWSSOptionsDialog::setTaskThread(GwmGWSSTaskThread *taskThread)
{
    mTaskThread = taskThread;
}

void GwmGWSSOptionsDialog::updateFieldsAndEnable()
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

void GwmGWSSOptionsDialog::updateFields()
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
//    if (ui->mCalcParallelNoneRadio->isChecked())
//    {
//        mTaskThread->setParallelType(IParallelalbe::SerialOnly);
//    }
//    else if (ui->mCalcParallelMultithreadRadio->isChecked())
//    {
//        mTaskThread->setParallelType(IParallelalbe::OpenMP);
//        mTaskThread->setOmpThreadNum(ui->mThreadNum->value());
//    }
//    else if (ui->mCalcParallelGPURadio->isChecked() && !ui->mDistTypeDmatRadio->isChecked())
//    {
//        mTaskThread->setParallelType(IParallelalbe::CUDA);
//    }
//    else
//    {
//        mTaskThread->setParallelType(IParallelalbe::SerialOnly);
//    }

}

void GwmGWSSOptionsDialog::enableAccept()
{
    if (mTaskThread->isValid())
    {
        ui->mCheckMessage->setText(tr("Valid."));
        ui->btbOkCancle->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    }
    else
    {
        ui->mCheckMessage->setText("Invalid.");
        ui->btbOkCancle->setStandardButtons(QDialogButtonBox::Cancel);
    }
}



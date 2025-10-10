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


GwmGWAverageOptionsDialog::GwmGWAverageOptionsDialog(QList<GwmLayerGroupItem*> originItemList, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmGWAverageOptionsDialog),
    mMapLayerList(originItemList)
{
    ui->setupUi(this);

    //图层选择部分
    for (GwmLayerGroupItem* item : mMapLayerList){
        ui->mLayerComboBox->addItem(item->originChild()->layer()->name());
    }
    ui->mLayerComboBox->setCurrentIndex(-1);


    //连接图层选择部分信号
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWAverageOptionsDialog::layerChanged);

    //带宽类型选择部分
    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::onVariableRadioToggled);


    //距离计算部分
    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGWAverageOptionsDialog::onDmatFileOpenClicked);

    //并行参数设置部分
    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
//    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelGPURadio);
#ifdef ENABLE_OpenMP
    int cores = omp_get_num_procs();
    ui->mThreadNum->setValue(cores);
    ui->mThreadNum->setMaximum(cores);    
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::onMultithreadingRadioToggled);
#else
    ui->mCalcParallelMultithreadRadio->setEnabled(false);
#endif
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::onNoneRadioToggled);
    //    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::onGPURadioToggled);
    ui->mCalcParallelGPURadio->hide();

    //更新线程参数信息
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mThetaValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mPValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileNameEdit, &QLineEdit::textChanged, this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mThreadNum, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mSampleGroupSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);
    connect(ui->mQuantileCheckBox, &QAbstractButton::toggle, this, &GwmGWAverageOptionsDialog::updateFieldsAndEnable);

    ui->mBwSizeAdaptiveSize->setMaximum(INT_MAX);
    ui->mBwSizeFixedSize->setMaximum(DBL_MAX);
    ui->mDistTypeCRSRadio->setChecked(true);
    updateFieldsAndEnable();
}

GwmGWAverageOptionsDialog::~GwmGWAverageOptionsDialog()
{
    delete ui;
}

bool GwmGWAverageOptionsDialog::isNumeric(QVariant::Type type)
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

GwmLayerGroupItem *GwmGWAverageOptionsDialog::selectedLayer() const
{
    return mSelectedLayer;
}

void GwmGWAverageOptionsDialog::setSelectedLayer(GwmLayerGroupItem *selectedLayer)
{
    mSelectedLayer = selectedLayer;
}

void GwmGWAverageOptionsDialog::layerChanged(int index)
{
    ui->mIndepVarSelector->layerChanged(mMapLayerList[index]->originChild()->layer());
    if (mSelectedLayer)
    {
        mSelectedLayer = nullptr;
    }
    mSelectedLayer =  mMapLayerList[index];
    ui->mIndepVarSelector->onDepVarChanged("");
}

QString GwmGWAverageOptionsDialog::crsRotateTheta()
{
    return ui->mThetaValue->text();
}

QString GwmGWAverageOptionsDialog::crsRotateP()
{
    return ui->mPValue->text();
}

bool GwmGWAverageOptionsDialog::bandwidthType()
{
    if(ui->mBwTypeFixedRadio->isChecked()){
        return false;
    }
    else if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return true;
    }
    else return false;
}

IParallelalbe::ParallelType GwmGWAverageOptionsDialog::approachType()
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


void GwmGWAverageOptionsDialog::onNoneRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void GwmGWAverageOptionsDialog::onMultithreadingRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void GwmGWAverageOptionsDialog::onGPURadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(2);
    }
}


GwmDistance::DistanceType GwmGWAverageOptionsDialog::distanceSourceType()
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

QVariant GwmGWAverageOptionsDialog::distanceSourceParameters()
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
void GwmGWAverageOptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(0);
}

void GwmGWAverageOptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(1);
}

void GwmGWAverageOptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(2);
    ui->mCalcParallelGroup->setEnabled(!checked);
}

void GwmGWAverageOptionsDialog::onDmatFileOpenClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Dmat File"), tr(""), tr("Dmat File (*.dmat)"));
    ui->mDistMatrixFileNameEdit->setText(filePath);
}

void GwmGWAverageOptionsDialog::onFixedRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(1);
}

void GwmGWAverageOptionsDialog::onVariableRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(0);
}

double GwmGWAverageOptionsDialog::bandwidthSize(){
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


gwm::BandwidthWeight::KernelFunctionType GwmGWAverageOptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return gwm::BandwidthWeight::KernelFunctionType(kernelSelected);
}

QVariant GwmGWAverageOptionsDialog::parallelParameters()
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

void GwmGWAverageOptionsDialog::updateFieldsAndEnable()
{
    this->updateFields();
    this->enableAccept();
}

void GwmGWAverageOptionsDialog::updateFields()
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


    GwmVariableItemModel* selectedIndepVarModel = ui->mIndepVarSelector->selectedIndepVarModel();
    if (selectedIndepVarModel)
    {
        if (selectedIndepVarModel->rowCount() > 0)
        {
            mAlgorithmMeta.variables = selectedIndepVarModel->attributeItemList();
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
        mAlgorithmMeta.distanceType = gwm::Distance::DistanceType::CRSDistance;
        mAlgorithmMeta.distanceCrsGeographic = dataLayer->crs().isGeographic();
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

    mAlgorithmMeta.quantile = ui->mQuantileCheckBox->isChecked();

}

void GwmGWAverageOptionsDialog::enableAccept()
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


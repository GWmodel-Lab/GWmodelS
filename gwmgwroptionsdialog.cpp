#include "gwmgwroptionsdialog.h"
#include "ui_gwmgwroptionsdialog.h"
#include <omp.h>
#include <QComboBox>
#include <QButtonGroup>
#include <QFileDialog>
#include <SpatialWeight/gwmcrsdistance.h>
#include <SpatialWeight/gwmdmatdistance.h>
#include <SpatialWeight/gwmminkwoskidistance.h>


GwmGWROptionsDialog::GwmGWROptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmBasicGWRAlgorithm *thread, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmGWROptionsDialog),
    mMapLayerList(originItemList),
    mDepVarModel(new GwmVariableItemModel),
    mTaskThread(thread)
{
    ui->setupUi(this);
    ui->mBwSizeAdaptiveSize->setMaximum(INT_MAX);
    ui->mBwSizeFixedSize->setMaximum(DBL_MAX);

    for (GwmLayerGroupItem* item : mMapLayerList){
        ui->mLayerComboBox->addItem(item->originChild()->layer()->name());
        ui->cmbRegressionLayerSelect->addItem(item->originChild()->layer()->name());
    }
    ui->mLayerComboBox->setCurrentIndex(-1);
    ui->cmbRegressionLayerSelect->setCurrentIndex(-1);
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWROptionsDialog::layerChanged);
    connect(ui->ckbRegressionPoints, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::on_cbkRegressionPoints_toggled);
    connect(ui->cmbRegressionLayerSelect, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWROptionsDialog::on_cmbRegressionLayerSelect_currentIndexChanged);

    ui->mDepVarComboBox->setCurrentIndex(-1);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWROptionsDialog::onDepVarChanged);

    ui->mModelSelAICThreshold->setMaximum(DBL_MAX);
    connect(ui->mVariableAutoSelectionCheck, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onVariableAutoSelectionToggled);

    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onVariableRadioToggled);

    QButtonGroup* bwSizeTypeBtnGroup = new QButtonGroup(this);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeAutomaticRadio);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeCustomizeRadio);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onAutomaticRadioToggled);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onCustomizeRaidoToggled);

    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGWROptionsDialog::onDmatFileOpenClicked);

    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelGPURadio);
    int cores = omp_get_num_procs();
    ui->mThreadNum->setValue(cores);
    ui->mThreadNum->setMaximum(cores);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onNoneRadioToggled);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onMultithreadingRadioToggled);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onGPURadioToggled);

    ui->mBwTypeAdaptiveRadio->setChecked(true);
    ui->mBwSizeAutomaticRadio->setChecked(true);
    ui->mCalcParallelNoneRadio->setChecked(true);
    ui->mDistTypeCRSRadio->setChecked(true);

    connect(ui->cbxHatmatrix, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::on_cbxHatmatrix_toggled);

    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->ckbRegressionPoints, &QAbstractButton::toggle, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->cmbRegressionLayerSelect, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mVariableAutoSelectionCheck, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mModelSelAICThreshold, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticApprochCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThetaValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mPValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileNameEdit, &QLineEdit::textChanged, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThreadNum, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mSampleGroupSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->cbxHatmatrix, &QAbstractButton::toggle, this, &GwmGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->cbxFTest, &QAbstractButton::toggle, this, &GwmGWROptionsDialog::updateFieldsAndEnable);

    updateFieldsAndEnable();
}

GwmGWROptionsDialog::~GwmGWROptionsDialog()
{
    delete ui;
}

bool GwmGWROptionsDialog::isNumeric(QVariant::Type type)
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

GwmLayerGroupItem *GwmGWROptionsDialog::selectedLayer() const
{
    return mSelectedLayer;
}

void GwmGWROptionsDialog::setSelectedLayer(GwmLayerGroupItem *selectedLayer)
{
    mSelectedLayer = selectedLayer;
}

void GwmGWROptionsDialog::layerChanged(int index)
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
//            GwmLayerAttributeItem* item = new GwmLayerAttributeItem();
//            item->setAttributeName(field.name());
//            item->setAttributeType(field.type());
//            item->setAttributeIndex(i);
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

void GwmGWROptionsDialog::onDepVarChanged(const int index)
{
    ui->mIndepVarSelector->onDepVarChanged(ui->mDepVarComboBox->itemText(index));
}

QString GwmGWROptionsDialog::crsRotateTheta()
{
    return ui->mThetaValue->text();
}

QString GwmGWROptionsDialog::crsRotateP()
{
    return ui->mPValue->text();
}

bool GwmGWROptionsDialog::bandwidthType()
{
    if(ui->mBwTypeFixedRadio->isChecked()){
        return false;
    }
    else if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return true;
    }
    else return true;
}

GwmGWRTaskThread::ParallelMethod GwmGWROptionsDialog::approachType()
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

void GwmGWROptionsDialog::onNoneRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void GwmGWROptionsDialog::onMultithreadingRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void GwmGWROptionsDialog::onGPURadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void GwmGWROptionsDialog::onAutomaticRadioToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(false);
        ui->mBwSizeAdaptiveUnit->setEnabled(false);
        ui->mBwSizeFixedSize->setEnabled(false);
        ui->mBwSizeFixedUnit->setEnabled(false);
    }
}

void GwmGWROptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(0);
}

void GwmGWROptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(1);
}

void GwmGWROptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(2);
    ui->mCalcParallelGroup->setEnabled(!checked);
}

void GwmGWROptionsDialog::onDmatFileOpenClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Dmat File"), tr(""), tr("Dmat File (*.dmat)"));
    ui->mDistMatrixFileNameEdit->setText(filePath);
}

void GwmGWROptionsDialog::onVariableAutoSelectionToggled(bool checked)
{
    ui->mModelSelAICThreshold->setEnabled(checked);
}

void GwmGWROptionsDialog::onCustomizeRaidoToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(true);
        ui->mBwSizeAdaptiveUnit->setEnabled(true);
        ui->mBwSizeFixedSize->setEnabled(true);
        ui->mBwSizeFixedUnit->setEnabled(true);
    }
}

void GwmGWROptionsDialog::onFixedRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(1);
}

void GwmGWROptionsDialog::onVariableRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(0);
}

double GwmGWROptionsDialog::bandwidthSize(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        return (double)ui->mBwSizeAdaptiveSize->value();
    }
    else
    {
        return ui->mBwSizeFixedSize->value();
    }
}

GwmBasicGWRAlgorithm::BandwidthSelectionCriterionType GwmGWROptionsDialog::bandwidthSelectionApproach()
{
    switch (ui->mBwSizeAutomaticApprochCombo->currentIndex())
    {
    case 0:
        return GwmBasicGWRAlgorithm::BandwidthSelectionCriterionType::CV;
    default:
        return GwmBasicGWRAlgorithm::BandwidthSelectionCriterionType::AIC;
    }
}

QString GwmGWROptionsDialog::bandWidthUnit(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        return ui->mBwSizeAdaptiveUnit->currentText();
    }
    else
    {
        return ui->mBwSizeFixedUnit->currentText();
    }
}

GwmBandwidthWeight::KernelFunctionType GwmGWROptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return GwmBandwidthWeight::KernelFunctionType(kernelSelected);
}

GwmGWRTaskThread::DistanceSourceType GwmGWROptionsDialog::distanceSourceType()
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

QVariant GwmGWROptionsDialog::distanceSourceParameters()
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

GwmGWRTaskThread::ParallelMethod GwmGWROptionsDialog::parallelMethod()
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

QVariant GwmGWROptionsDialog::parallelParameters()
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

void GwmGWROptionsDialog::setTaskThread(GwmGWRTaskThread *taskThread)
{

}

void GwmGWROptionsDialog::updateFieldsAndEnable()
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

void GwmGWROptionsDialog::updateFields()
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
    // 回归点设置
    if (ui->ckbRegressionPoints->isChecked())
    {
        int regLayerIndex = ui->cmbRegressionLayerSelect->currentIndex();
        if (regLayerIndex > -1)
        {
            mTaskThread->setRegressionLayer(mMapLayerList[regLayerIndex]->originChild()->layer());
        }
        else
        {
            mTaskThread->setRegressionLayer(nullptr);
        }
    }
    else
    {
        mTaskThread->setRegressionLayer(nullptr);
    }
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
        else if (ui->mVariableAutoSelectionCheck->isChecked())
        {
            GwmVariableItemModel* indepVarModel = ui->mIndepVarSelector->indepVarModel();
            if (indepVarModel)
            {
                mTaskThread->setIndependentVariables(indepVarModel->attributeItemList());
            }
        }
        mTaskThread->setIsAutoselectIndepVars(ui->mVariableAutoSelectionCheck->isChecked());
        mTaskThread->setIndepVarSelectionThreshold(ui->mModelSelAICThreshold->value());
    }
    // 带宽设置
    if (ui->mBwSizeAutomaticRadio->isChecked())
    {
        mTaskThread->setIsAutoselectBandwidth(true);
        mTaskThread->setBandwidthSelectionCriterionType(bandwidthSelectionApproach());
    }
    else if (ui->mBwSizeCustomizeRadio->isChecked())
    {
        mTaskThread->setIsAutoselectBandwidth(false);
    }
    GwmSpatialWeight spatialWeight;
    GwmBandwidthWeight weight(bandwidthSize(), bandwidthType(), bandwidthKernelFunction());
    spatialWeight.setWeight(weight);
    // 距离设置
    if (ui->mDistTypeDmatRadio->isChecked())
    {
        QString filename = ui->mDistMatrixFileNameEdit->text();
        int featureCount = dataLayer->featureCount();
        GwmDMatDistance distance(filename, featureCount);
        spatialWeight.setDistance(distance);
    }
    else if (ui->mDistTypeMinkowskiRadio->isChecked())
    {
        double theta = ui->mThetaValue->value();
        double p = ui->mPValue->value();
        GwmMinkwoskiDistance distance(p, theta);
        spatialWeight.setDistance(distance);
    }
    else
    {
        GwmCRSDistance distance(dataLayer->crs().isGeographic());
        spatialWeight.setDistance(distance);
    }
    mTaskThread->setSpatialWeight(spatialWeight);
    // 并行设置
//    if (distSrcType != GwmGWRTaskThread::DistanceSourceType::DMatFile)
//    {
//        mTaskThread->setParallelMethodType(this->parallelMethod());
//        mTaskThread->setParallelParameter(this->parallelParameters());
//    }
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
    mTaskThread->setHasFTest(ui->cbxFTest->isChecked());
}

void GwmGWROptionsDialog::enableAccept()
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


void GwmGWROptionsDialog::on_cbxHatmatrix_toggled(bool checked)
{
    if (checked)
    {
        ui->cbxFTest->setEnabled(true);
    }
    else
    {
        ui->cbxFTest->setChecked(false);
        ui->cbxFTest->setEnabled(false);
    }
}

void GwmGWROptionsDialog::on_cbkRegressionPoints_toggled(bool checked)
{
    ui->cmbRegressionLayerSelect->setEnabled(checked);
    if (checked)
    {
        ui->mBwSizeCustomizeRadio->setChecked(true);
        ui->mBwSizeAutomaticRadio->setEnabled(false);
        ui->mVariableAutoSelectionCheck->setChecked(false);
        ui->mVariableAutoSelectionCheck->setEnabled(false);
        ui->cbxHatmatrix->setEnabled(false);
        ui->cbxHatmatrix->setChecked(false);
    }
    else
    {
        ui->mBwSizeAutomaticRadio->setEnabled(true);
        ui->mVariableAutoSelectionCheck->setEnabled(true);
    }
    ui->cbxHatmatrix->setEnabled(!checked);
    ui->cbxHatmatrix->setChecked(!checked);
}

void GwmGWROptionsDialog::on_cmbRegressionLayerSelect_currentIndexChanged(int index)
{

}

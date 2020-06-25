#include "gwmggwroptionsdialog.h"
#include "ui_gwmggwroptionsdialog.h"
#include <QComboBox>
#include <QButtonGroup>
#include <QFileDialog>

#include "Model/gwmvariableitemmodel.h"
#include <SpatialWeight/gwmbandwidthweight.h>

GwmGGWROptionsDialog::GwmGGWROptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmGGWRAlgorithm* thread,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmGGWROptionsDialog),
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
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGGWROptionsDialog::layerChanged);
    connect(ui->ckbRegressionPoints, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::on_cbkRegressionPoints_toggled);
    connect(ui->cmbRegressionLayerSelect, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGGWROptionsDialog::on_cmbRegressionLayerSelect_currentIndexChanged);

    ui->mDepVarComboBox->setCurrentIndex(-1);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGGWROptionsDialog::onDepVarChanged);

    ui->mModelSelAICThreshold->setMaximum(DBL_MAX);
    connect(ui->mVariableAutoSelectionCheck, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::onVariableAutoSelectionToggled);

    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::onVariableRadioToggled);

    QButtonGroup* bwSizeTypeBtnGroup = new QButtonGroup(this);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeAutomaticRadio);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeCustomizeRadio);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::onAutomaticRadioToggled);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::onCustomizeRaidoToggled);

    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGGWROptionsDialog::onDmatFileOpenClicked);

    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelGPURadio);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::onNoneRadioToggled);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::onMultithreadingRadioToggled);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::onGPURadioToggled);

    ui->mBwTypeAdaptiveRadio->setChecked(true);
    ui->mBwSizeAutomaticRadio->setChecked(true);
    ui->mCalcParallelNoneRadio->setChecked(true);
    ui->mDistTypeCRSRadio->setChecked(true);

    connect(ui->cbxHatmatrix, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::on_cbxHatmatrix_toggled);

    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->ckbRegressionPoints, &QAbstractButton::toggle, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->cmbRegressionLayerSelect, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mVariableAutoSelectionCheck, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mModelSelAICThreshold, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticApprochCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThetaValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mPValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileNameEdit, &QLineEdit::textChanged, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThreadNum, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mSampleGroupSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->cbxHatmatrix, &QAbstractButton::toggle, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->cbxFTest, &QAbstractButton::toggle, this, &GwmGGWROptionsDialog::updateFieldsAndEnable);

    connect(ui->mDistributionType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mEpsilonSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mEpsilonUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mMaxiterSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGGWROptionsDialog::updateFieldsAndEnable);

    ui->mEpsilonSize->setValue(1.0);
    ui->mMaxiterSpinBox->setValue(20);
    ui->mEpsilonUnit->setCurrentIndex(1);

    ui->mVariableAutoSelectionCheck->hide();
    ui->mModelSelAICThreshold->hide();
    ui->label_10->hide();
    ui->cbxFTest->hide();
}

GwmGGWROptionsDialog::~GwmGGWROptionsDialog()
{
    delete ui;
}

bool GwmGGWROptionsDialog::isNumeric(QVariant::Type type)
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

GwmLayerGroupItem *GwmGGWROptionsDialog::selectedLayer() const
{
    return mSelectedLayer;
}

void GwmGGWROptionsDialog::setSelectedLayer(GwmLayerGroupItem *selectedLayer)
{
    mSelectedLayer = selectedLayer;
}

void GwmGGWROptionsDialog::layerChanged(int index)
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
}

void GwmGGWROptionsDialog::onDepVarChanged(const int index)
{
    ui->mIndepVarSelector->onDepVarChanged(ui->mDepVarComboBox->itemText(index));
}

QString GwmGGWROptionsDialog::crsRotateTheta()
{
    return ui->mThetaValue->text();
}

QString GwmGGWROptionsDialog::crsRotateP()
{
    return ui->mPValue->text();
}

bool GwmGGWROptionsDialog::bandwidthType()
{
    if(ui->mBwTypeFixedRadio->isChecked()){
        return false;
    }
    else if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return true;
    }
    else return true;
}

GwmGWRTaskThread::ParallelMethod GwmGGWROptionsDialog::approachType()
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

void GwmGGWROptionsDialog::onNoneRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void GwmGGWROptionsDialog::onMultithreadingRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void GwmGGWROptionsDialog::onGPURadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void GwmGGWROptionsDialog::onAutomaticRadioToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(false);
        ui->mBwSizeAdaptiveUnit->setEnabled(false);
        ui->mBwSizeFixedSize->setEnabled(false);
        ui->mBwSizeFixedUnit->setEnabled(false);
    }
}

void GwmGGWROptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(0);
}

void GwmGGWROptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(1);
}

void GwmGGWROptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(2);
    ui->mCalcParallelGroup->setEnabled(!checked);
}

void GwmGGWROptionsDialog::onDmatFileOpenClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Dmat File"), tr(""), tr("Dmat File (*.dmat)"));
    ui->mDistMatrixFileNameEdit->setText(filePath);
}

void GwmGGWROptionsDialog::onVariableAutoSelectionToggled(bool checked)
{
    ui->mModelSelAICThreshold->setEnabled(checked);
}

void GwmGGWROptionsDialog::onCustomizeRaidoToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(true);
        ui->mBwSizeAdaptiveUnit->setEnabled(true);
        ui->mBwSizeFixedSize->setEnabled(true);
        ui->mBwSizeFixedUnit->setEnabled(true);
    }
}

void GwmGGWROptionsDialog::onFixedRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(1);
//    mTaskThread->setBandwidthType(GwmGWRTaskThread::BandwidthType::Fixed);
}

void GwmGGWROptionsDialog::onVariableRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(0);
//    mTaskThread->setBandwidthType(GwmGWRTaskThread::BandwidthType::Adaptive);
}

double GwmGGWROptionsDialog::bandwidthSize(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        QList<double> unit = { 1, 10, 100, 1000 };
        return (double)ui->mBwSizeAdaptiveSize->value() * unit[ui->mBwSizeAdaptiveUnit->currentIndex()];
    }
    else
    {
        QList<double> unit = { 1.0, 1000.0, 1609.344 };
        return ui->mBwSizeFixedSize->value() * unit[ui->mBwSizeAdaptiveUnit->currentIndex()];
    }
}

GwmGGWRAlgorithm::BandwidthSelectionCriterionType GwmGGWROptionsDialog::bandwidthSelectionApproach()
{
    switch (ui->mBwSizeAutomaticApprochCombo->currentIndex())
    {
    case 0:
        return GwmGGWRAlgorithm::BandwidthSelectionCriterionType::CV;
    default:
        return GwmGGWRAlgorithm::BandwidthSelectionCriterionType::AIC;
    }
}

QString GwmGGWROptionsDialog::bandWidthUnit(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        return ui->mBwSizeAdaptiveUnit->currentText();
    }
    else
    {
        return ui->mBwSizeFixedUnit->currentText();
    }
}

GwmBandwidthWeight::KernelFunctionType GwmGGWROptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return GwmBandwidthWeight::KernelFunctionType(kernelSelected);
}

GwmGWRTaskThread::DistanceSourceType GwmGGWROptionsDialog::distanceSourceType()
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

QVariant GwmGGWROptionsDialog::distanceSourceParameters()
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

GwmGWRTaskThread::ParallelMethod GwmGGWROptionsDialog::parallelMethod()
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

QVariant GwmGGWROptionsDialog::parallelParameters()
{
    if (ui->mCalcParallelGPURadio->isChecked())
    {
        return ui->mSampleGroupSize->value();
    }
    else if (ui->mCalcParallelMultithreadRadio->isChecked())
    {
        return ui->mThreadNum->text();
    }
    else
    {
        return QVariant();
    }
}

double GwmGGWROptionsDialog::epsilonSize(){
    if(ui->mEpsilonSize->value() <= 0.0){
        return 1.0;
    }
    return ui->mEpsilonSize->value();
}

QString GwmGGWROptionsDialog::epsilonUnit(){
    return ui->mEpsilonUnit->currentText();
}

GwmGGWRAlgorithm::Family GwmGGWROptionsDialog::distributionFunction(){
    int distributionSelected = ui->mDistributionType->currentIndex();
    return GwmGGWRAlgorithm::Family(distributionSelected);
}

int GwmGGWROptionsDialog::maxiter(){
    if(ui->mMaxiterSpinBox->value() <= 0){
        return 20;
    }
    return ui->mMaxiterSpinBox->value();
}


void GwmGGWROptionsDialog::setTaskThread(GwmGGWRAlgorithm *taskThread)
{
    mTaskThread = taskThread;
}

void GwmGGWROptionsDialog::updateFieldsAndEnable()
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

void GwmGGWROptionsDialog::updateFields()
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
    //GGWR 参数设置
    mTaskThread->setTol(this->epsilonSize(),this->epsilonUnit());
    mTaskThread->setFamily(this->distributionFunction());
    mTaskThread->setMaxiter(this->maxiter());
}

void GwmGGWROptionsDialog::enableAccept()
{
    QString message = "Invalid.";
    if (!mTaskThread->isValid())
    {
        ui->mCheckMessage->setText(message);
    }
    else
    {
        ui->mCheckMessage->setText(tr("Valid."));
    }
}


void GwmGGWROptionsDialog::on_cbxHatmatrix_toggled(bool checked)
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

void GwmGGWROptionsDialog::on_cbkRegressionPoints_toggled(bool checked)
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

void GwmGGWROptionsDialog::on_cmbRegressionLayerSelect_currentIndexChanged(int index)
{

}


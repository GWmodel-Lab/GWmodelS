#include "GwmLcrGWRoptionsdialog.h"
#include "ui_GwmLcrGWRoptionsdialog.h"
#include <omp.h>
#include <QComboBox>
#include <QButtonGroup>
#include <QFileDialog>


GwmLcrGWROptionsDialog::GwmLcrGWROptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmLcrGWRTaskThread* thread,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmLcrGWROptionsDialog),
    mMapLayerList(originItemList),
    mDepVarModel(new GwmLayerAttributeItemModel),
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
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmLcrGWROptionsDialog::layerChanged);
    connect(ui->ckbRegressionPoints, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::on_cbkRegressionPoints_toggled);
    connect(ui->cmbRegressionLayerSelect, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmLcrGWROptionsDialog::on_cmbRegressionLayerSelect_currentIndexChanged);

    ui->mDepVarComboBox->setCurrentIndex(-1);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmLcrGWROptionsDialog::onDepVarChanged);

    connect(ui->mLambdaAdjustCheck,&QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::onLambdaAdjustCheckToggled);

    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::onVariableRadioToggled);

    QButtonGroup* bwSizeTypeBtnGroup = new QButtonGroup(this);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeAutomaticRadio);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeCustomizeRadio);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::onAutomaticRadioToggled);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::onCustomizeRaidoToggled);

    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmLcrGWROptionsDialog::onDmatFileOpenClicked);

    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelGPURadio);
    int cores = omp_get_num_procs();
    ui->mThreadNum->setValue(cores);
    ui->mThreadNum->setMaximum(cores);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::onNoneRadioToggled);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::onMultithreadingRadioToggled);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::onGPURadioToggled);

    ui->mBwTypeAdaptiveRadio->setChecked(true);
    ui->mBwSizeAutomaticRadio->setChecked(true);
    ui->mCalcParallelNoneRadio->setChecked(true);
    ui->mDistTypeCRSRadio->setChecked(true);

    connect(ui->cbxHatmatrix, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::on_cbxHatmatrix_toggled);

    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->ckbRegressionPoints, &QAbstractButton::toggle, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->cmbRegressionLayerSelect, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    //connect(ui->mVariableAutoSelectionCheck, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    //connect(ui->mModelSelAICThreshold, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticApprochCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThetaValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mPValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileNameEdit, &QLineEdit::textChanged, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThreadNum, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mSampleGroupSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->cbxHatmatrix, &QAbstractButton::toggle, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    //connect(ui->cbxFTest, &QAbstractButton::toggle, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);

    connect(ui->mLambdaAdjustCheck,&QAbstractButton::toggled, this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mLambdaSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mcnThreshSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmLcrGWROptionsDialog::updateFieldsAndEnable);

    updateFieldsAndEnable();
}

GwmLcrGWROptionsDialog::~GwmLcrGWROptionsDialog()
{
    delete ui;
}

bool GwmLcrGWROptionsDialog::isNumeric(QVariant::Type type)
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

GwmLayerGroupItem *GwmLcrGWROptionsDialog::selectedLayer() const
{
    return mSelectedLayer;
}

void GwmLcrGWROptionsDialog::setSelectedLayer(GwmLayerGroupItem *selectedLayer)
{
    mSelectedLayer = selectedLayer;
}

void GwmLcrGWROptionsDialog::layerChanged(int index)
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
            GwmLayerAttributeItem* item = new GwmLayerAttributeItem();
            item->setAttributeName(field.name());
            item->setAttributeType(field.type());
            item->setAttributeIndex(i);
            mDepVarModel->appendRow(item);
            ui->mDepVarComboBox->addItem(field.name());
        }
    }
}

void GwmLcrGWROptionsDialog::onDepVarChanged(const int index)
{
    ui->mIndepVarSelector->onDepVarChanged(ui->mDepVarComboBox->itemText(index));
}

QString GwmLcrGWROptionsDialog::crsRotateTheta()
{
    return ui->mThetaValue->text();
}

QString GwmLcrGWROptionsDialog::crsRotateP()
{
    return ui->mPValue->text();
}

GwmLcrGWRTaskThread::BandwidthType GwmLcrGWROptionsDialog::bandwidthType()
{
    if(ui->mBwTypeFixedRadio->isChecked()){
        return GwmLcrGWRTaskThread::BandwidthType::Fixed;
    }
    else if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return GwmLcrGWRTaskThread::BandwidthType::Adaptive;
    }
    else return GwmLcrGWRTaskThread::BandwidthType::Fixed;
}

GwmLcrGWRTaskThread::ParallelMethod GwmLcrGWROptionsDialog::approachType()
{
    if(ui->mCalcParallelNoneRadio->isChecked()){
        return GwmLcrGWRTaskThread::ParallelMethod::None;
    }
    else if(ui->mCalcParallelMultithreadRadio->isChecked()){
        return GwmLcrGWRTaskThread::ParallelMethod::Multithread;
    }
    else if(ui->mCalcParallelGPURadio->isChecked()){
        return GwmLcrGWRTaskThread::ParallelMethod::GPU;
    }
    else return GwmLcrGWRTaskThread::ParallelMethod::None;
}

void GwmLcrGWROptionsDialog::onNoneRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void GwmLcrGWROptionsDialog::onMultithreadingRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void GwmLcrGWROptionsDialog::onGPURadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void GwmLcrGWROptionsDialog::onAutomaticRadioToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(false);
        ui->mBwSizeAdaptiveUnit->setEnabled(false);
        ui->mBwSizeFixedSize->setEnabled(false);
        ui->mBwSizeFixedUnit->setEnabled(false);
    }
}

void GwmLcrGWROptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(0);
}

void GwmLcrGWROptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(1);
}

void GwmLcrGWROptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(2);
    ui->mCalcParallelGroup->setEnabled(!checked);
}

void GwmLcrGWROptionsDialog::onDmatFileOpenClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Dmat File"), tr(""), tr("Dmat File (*.dmat)"));
    ui->mDistMatrixFileNameEdit->setText(filePath);
}

void GwmLcrGWROptionsDialog::onVariableAutoSelectionToggled(bool checked)
{
    //ui->mModelSelAICThreshold->setEnabled(checked);
}

void GwmLcrGWROptionsDialog::onLambdaAdjustCheckToggled(bool checked)
{
    ui->mcnThreshSpinBox->setEnabled(checked);
}

void GwmLcrGWROptionsDialog::onCustomizeRaidoToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(true);
        ui->mBwSizeAdaptiveUnit->setEnabled(true);
        ui->mBwSizeFixedSize->setEnabled(true);
        ui->mBwSizeFixedUnit->setEnabled(true);
    }
}

void GwmLcrGWROptionsDialog::onFixedRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(1);
    mTaskThread->setBandwidthType(GwmLcrGWRTaskThread::BandwidthType::Fixed);
}

void GwmLcrGWROptionsDialog::onVariableRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(0);
    mTaskThread->setBandwidthType(GwmLcrGWRTaskThread::BandwidthType::Adaptive);
}

double GwmLcrGWROptionsDialog::bandwidthSize(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        return (double)ui->mBwSizeAdaptiveSize->value();
    }
    else
    {
        return ui->mBwSizeFixedSize->value();
    }
}

GwmLcrGWRTaskThread::BandwidthSelectionApproach GwmLcrGWROptionsDialog::bandwidthSelectionApproach()
{
    switch (ui->mBwSizeAutomaticApprochCombo->currentIndex())
    {
    case 0:
        return GwmLcrGWRTaskThread::CV;
    default:
        return GwmLcrGWRTaskThread::AIC;
    }
}

QString GwmLcrGWROptionsDialog::bandWidthUnit(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        return ui->mBwSizeAdaptiveUnit->currentText();
    }
    else
    {
        return ui->mBwSizeFixedUnit->currentText();
    }
}

GwmLcrGWRTaskThread::KernelFunction GwmLcrGWROptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return GwmLcrGWRTaskThread::KernelFunction(kernelSelected);
}

GwmLcrGWRTaskThread::DistanceSourceType GwmLcrGWROptionsDialog::distanceSourceType()
{
    if (ui->mDistTypeCRSRadio->isChecked())
        return GwmLcrGWRTaskThread::DistanceSourceType::CRS;
    else if (ui->mDistTypeDmatRadio->isChecked())
        return GwmLcrGWRTaskThread::DistanceSourceType::DMatFile;
    else if (ui->mDistTypeMinkowskiRadio->isChecked())
        return GwmLcrGWRTaskThread::DistanceSourceType::Minkowski;
    else
        return GwmLcrGWRTaskThread::DistanceSourceType::CRS;
}

QVariant GwmLcrGWROptionsDialog::distanceSourceParameters()
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

GwmLcrGWRTaskThread::ParallelMethod GwmLcrGWROptionsDialog::parallelMethod()
{
    if (ui->mCalcParallelMultithreadRadio->isChecked())
    {
        return GwmLcrGWRTaskThread::ParallelMethod::Multithread;
    }
    else if (ui->mCalcParallelGPURadio->isChecked())
    {
        return GwmLcrGWRTaskThread::ParallelMethod::GPU;
    }
    else
    {
        return GwmLcrGWRTaskThread::ParallelMethod::None;
    }
}

QVariant GwmLcrGWROptionsDialog::parallelParameters()
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

void GwmLcrGWROptionsDialog::setTaskThread(GwmLcrGWRTaskThread *taskThread)
{

}

void GwmLcrGWROptionsDialog::updateFieldsAndEnable()
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

void GwmLcrGWROptionsDialog::updateFields()
{
    // 图层设置
    if (ui->mLayerComboBox->currentIndex() > -1)
    {
        mTaskThread->setLayer(mSelectedLayer->originChild()->layer());
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
        mTaskThread->setDepVar(mDepVarModel->item(ui->mDepVarComboBox->currentIndex()));
    }
    // 自变量设置
    GwmLayerAttributeItemModel* selectedIndepVarModel = ui->mIndepVarSelector->selectedIndepVarModel();
    if (selectedIndepVarModel)
    {
        if (selectedIndepVarModel->rowCount() > 0)
        {
            mTaskThread->setIndepVars(selectedIndepVarModel->attributeItemList());
        }
//        else if (ui->mVariableAutoSelectionCheck->isChecked())
//        {
//            GwmLayerAttributeItemModel* indepVarModel = ui->mIndepVarSelector->indepVarModel();
//            if (indepVarModel)
//            {
//                mTaskThread->setIndepVars(indepVarModel->attributeItemList());
//            }
//        }
//        mTaskThread->setEnableIndepVarAutosel(ui->mVariableAutoSelectionCheck->isChecked());
//        mTaskThread->setModelSelThreshold(ui->mModelSelAICThreshold->value());
    }
    // 带宽设置
    if (ui->mBwSizeAutomaticRadio->isChecked())
    {
        mTaskThread->setIsBandwidthSizeAutoSel(true);
        mTaskThread->setBandwidthSelectionApproach(bandwidthSelectionApproach());
    }
    else if (ui->mBwSizeCustomizeRadio->isChecked())
    {
        mTaskThread->setIsBandwidthSizeAutoSel(false);
        mTaskThread->setBandwidth(this->bandwidthType(), this->bandwidthSize(), this->bandWidthUnit());
    }
    mTaskThread->setBandwidthKernelFunction(this->bandwidthKernelFunction());
    // 距离设置
    auto distSrcType = this->distanceSourceType();
    mTaskThread->setDistSrcType(distSrcType);
    mTaskThread->setDistSrcParameters(this->distanceSourceParameters());
    // 并行设置
    if (distSrcType != GwmLcrGWRTaskThread::DistanceSourceType::DMatFile)
    {
        mTaskThread->setParallelMethodType(this->parallelMethod());
        mTaskThread->setParallelParameter(this->parallelParameters());
    }
    // 其他设置
    mTaskThread->setHasHatMatrix(ui->cbxHatmatrix->isChecked());
    mTaskThread->mlambdaAdjust = ui->mLambdaAdjustCheck->isChecked();
    mTaskThread->mcnThresh = ui->mcnThreshSpinBox->value();
    mTaskThread->mlambda = ui->mLambdaSpinBox->value();
}

void GwmLcrGWROptionsDialog::enableAccept()
{
    QString message;
    if (mTaskThread->isValid(message))
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


void GwmLcrGWROptionsDialog::on_cbxHatmatrix_toggled(bool checked)
{
//    if (checked)
//    {
//        ui->cbxFTest->setEnabled(true);
//    }
//    else
//    {
//        ui->cbxFTest->setChecked(false);
//        ui->cbxFTest->setEnabled(false);
//    }
}

void GwmLcrGWROptionsDialog::on_cbkRegressionPoints_toggled(bool checked)
{
    ui->cmbRegressionLayerSelect->setEnabled(checked);
    if (checked)
    {
        ui->mBwSizeCustomizeRadio->setChecked(true);
        ui->mBwSizeAutomaticRadio->setEnabled(false);
        //ui->mVariableAutoSelectionCheck->setChecked(false);
        //ui->mVariableAutoSelectionCheck->setEnabled(false);
        ui->cbxHatmatrix->setEnabled(false);
        ui->cbxHatmatrix->setChecked(false);
    }
    else
    {
        ui->mBwSizeAutomaticRadio->setEnabled(true);
        //ui->mVariableAutoSelectionCheck->setEnabled(true);
    }
    ui->cbxHatmatrix->setEnabled(!checked);
    ui->cbxHatmatrix->setChecked(!checked);
}

void GwmLcrGWROptionsDialog::on_cmbRegressionLayerSelect_currentIndexChanged(int index)
{

}

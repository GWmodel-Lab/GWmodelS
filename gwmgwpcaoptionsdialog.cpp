#include "gwmgwpcaoptionsdialog.h"
#include "ui_gwmgwpcaoptionsdialog.h"
#include <omp.h>
#include <QComboBox>
#include <QButtonGroup>
#include <QFileDialog>
#include <SpatialWeight/gwmcrsdistance.h>
#include <SpatialWeight/gwmdmatdistance.h>
#include <SpatialWeight/gwmminkwoskidistance.h>

GwmGWPCAOptionsDialog::GwmGWPCAOptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmGWPCATaskThread *thread, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmGWPCAOptionsDialog),
    mMapLayerList(originItemList),
    mDepVarModel(new GwmVariableItemModel),
    mTaskThread(thread)
{
    ui->setupUi(this);
    ui->mBwSizeAdaptiveSize->setMaximum(INT_MAX);
    ui->mBwSizeFixedSize->setMaximum(DBL_MAX);

    for (GwmLayerGroupItem* item : mMapLayerList){
        ui->mLayerComboBox->addItem(item->originChild()->layer()->name());
        //ui->cmbRegressionLayerSelect->addItem(item->originChild()->layer()->name());
    }
    ui->mLayerComboBox->setCurrentIndex(-1);
    //ui->cmbRegressionLayerSelect->setCurrentIndex(-1);
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWPCAOptionsDialog::layerChanged);
    //connect(ui->ckbRegressionPoints, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::on_cbkRegressionPoints_toggled);

    //ui->mDepVarComboBox->setCurrentIndex(-1);
    //connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWPCAOptionsDialog::onDepVarChanged);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGWPCAOptionsDialog::onSelectedIndenpendentVariablesChanged);
    ui->mKspinBox->setMaximum(1);

    //ui->mModelSelAICThreshold->setMaximum(DBL_MAX);
    //connect(ui->mVariableAutoSelectionCheck, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::onVariableAutoSelectionToggled);

    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::onVariableRadioToggled);

    QButtonGroup* bwSizeTypeBtnGroup = new QButtonGroup(this);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeAutomaticRadio);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeCustomizeRadio);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::onAutomaticRadioToggled);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::onCustomizeRaidoToggled);

    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGWPCAOptionsDialog::onDmatFileOpenClicked);

    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelGPURadio);
    int cores = omp_get_num_procs();
    ui->mThreadNum->setValue(cores);
    ui->mThreadNum->setMaximum(cores);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::onNoneRadioToggled);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::onMultithreadingRadioToggled);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::onGPURadioToggled);

    connect(ui->mKspinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);


    ui->mCalcParallelGPURadio->hide();

    ui->mBwTypeAdaptiveRadio->setChecked(true);
    ui->mBwSizeAutomaticRadio->setChecked(true);
    ui->mCalcParallelNoneRadio->setChecked(true);
    ui->mDistTypeCRSRadio->setChecked(true);

    //connect(ui->cbxHatmatrix, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::on_cbxHatmatrix_toggled);

    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    //connect(ui->ckbRegressionPoints, &QAbstractButton::toggle, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    //connect(ui->cmbRegressionLayerSelect, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    //connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    //connect(ui->mVariableAutoSelectionCheck, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    //connect(ui->mModelSelAICThreshold, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticApprochCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mThetaValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mPValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileNameEdit, &QLineEdit::textChanged, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mThreadNum, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mSampleGroupSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    //connect(ui->cbxHatmatrix, &QAbstractButton::toggle, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    //connect(ui->cbxFTest, &QAbstractButton::toggle, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);
    connect(ui->mScoresCheckBox,&QAbstractButton::toggle, this, &GwmGWPCAOptionsDialog::updateFieldsAndEnable);

    updateFieldsAndEnable();
}

GwmGWPCAOptionsDialog::~GwmGWPCAOptionsDialog()
{
    delete ui;
}

bool GwmGWPCAOptionsDialog::isNumeric(QVariant::Type type)
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

GwmLayerGroupItem *GwmGWPCAOptionsDialog::selectedLayer() const
{
    return mSelectedLayer;
}

void GwmGWPCAOptionsDialog::setSelectedLayer(GwmLayerGroupItem *selectedLayer)
{
    mSelectedLayer = selectedLayer;
}

void GwmGWPCAOptionsDialog::layerChanged(int index)
{
    //
    ui->mIndepVarSelector->layerChanged(mMapLayerList[index]->originChild()->layer());
    //
    ui->mIndepVarSelector->onDepVarChanged("");

    if (mSelectedLayer)
    {
        mSelectedLayer = nullptr;
    }
    mSelectedLayer =  mMapLayerList[index];
    QgsFields fieldList = mSelectedLayer->originChild()->layer()->fields();
    //ui->mDepVarComboBox->clear();
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
            //ui->mDepVarComboBox->addItem(field.name());
        }
        ui->mKspinBox->setMaximum(field.length());
    }
}

void GwmGWPCAOptionsDialog::onDepVarChanged(const int index)
{
    //ui->mIndepVarSelector->onDepVarChanged(ui->mDepVarComboBox->itemText(index));
}

QString GwmGWPCAOptionsDialog::crsRotateTheta()
{
    return ui->mThetaValue->text();
}

QString GwmGWPCAOptionsDialog::crsRotateP()
{
    return ui->mPValue->text();
}

bool GwmGWPCAOptionsDialog::bandwidthType()
{
    if(ui->mBwTypeFixedRadio->isChecked()){
        return false;
    }
    else if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return true;
    }
    else return true;
}

void GwmGWPCAOptionsDialog::onNoneRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void GwmGWPCAOptionsDialog::onMultithreadingRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void GwmGWPCAOptionsDialog::onGPURadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void GwmGWPCAOptionsDialog::onAutomaticRadioToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(false);
        ui->mBwSizeAdaptiveUnit->setEnabled(false);
        ui->mBwSizeFixedSize->setEnabled(false);
        ui->mBwSizeFixedUnit->setEnabled(false);
    }
}

void GwmGWPCAOptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(0);
}

void GwmGWPCAOptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(1);
}

void GwmGWPCAOptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(2);
    ui->mCalcParallelGroup->setEnabled(!checked);
}

void GwmGWPCAOptionsDialog::onDmatFileOpenClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Dmat File"), tr(""), tr("Dmat File (*.dmat)"));
    ui->mDistMatrixFileNameEdit->setText(filePath);
}

void GwmGWPCAOptionsDialog::onVariableAutoSelectionToggled(bool checked)
{
    //ui->mModelSelAICThreshold->setEnabled(checked);
}

void GwmGWPCAOptionsDialog::onSelectedIndenpendentVariablesChanged()
{
    ui->mKspinBox->setMaximum(ui->mIndepVarSelector->selectedIndepVarModel()->rowCount());
}

void GwmGWPCAOptionsDialog::onCustomizeRaidoToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(true);
        ui->mBwSizeAdaptiveUnit->setEnabled(true);
        ui->mBwSizeFixedSize->setEnabled(true);
        ui->mBwSizeFixedUnit->setEnabled(true);
    }
}

void GwmGWPCAOptionsDialog::onFixedRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(1);
}

void GwmGWPCAOptionsDialog::onVariableRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(0);
}

double GwmGWPCAOptionsDialog::bandwidthSize(){
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

//GwmBasicGWRAlgorithm::BandwidthSelectionCriterionType GwmGWPCAOptionsDialog::bandwidthSelectionApproach()
//{
//    switch (ui->mBwSizeAutomaticApprochCombo->currentIndex())
//    {
//    case 0:
//        return GwmBasicGWRAlgorithm::BandwidthSelectionCriterionType::CV;
//    default:
//        return GwmBasicGWRAlgorithm::BandwidthSelectionCriterionType::AIC;
//    }
//}

QString GwmGWPCAOptionsDialog::bandWidthUnit(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        return ui->mBwSizeAdaptiveUnit->currentText();
    }
    else
    {
        return ui->mBwSizeFixedUnit->currentText();
    }
}

GwmBandwidthWeight::KernelFunctionType GwmGWPCAOptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return GwmBandwidthWeight::KernelFunctionType(kernelSelected);
}

QVariant GwmGWPCAOptionsDialog::distanceSourceParameters()
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

QVariant GwmGWPCAOptionsDialog::parallelParameters()
{
    if (ui->mCalcParallelGPURadio->isChecked())
    {
        QMap<QString, QVariant> parameters;
        parameters["groupSize"] = ui->mSampleGroupSize->value();
        parameters["gpuID"] = ui->mGPUSelection->currentIndex();
        return parameters;
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

void GwmGWPCAOptionsDialog::updateFieldsAndEnable()
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

void GwmGWPCAOptionsDialog::updateFields()
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
//    if (ui->ckbRegressionPoints->isChecked())
//    {
//        int regLayerIndex = ui->cmbRegressionLayerSelect->currentIndex();
//        if (regLayerIndex > -1)
//        {
//            mTaskThread->setRegressionLayer(mMapLayerList[regLayerIndex]->originChild()->layer());
//        }
//        else
//        {
//            mTaskThread->setRegressionLayer(nullptr);
//        }
//    }
//    else
//    {
//        mTaskThread->setRegressionLayer(nullptr);
//    }
    // 因变量设置
//    if (ui->mDepVarComboBox->currentIndex() > -1)
//    {
//        mTaskThread->setDependentVariable(mDepVarModel->item(ui->mDepVarComboBox->currentIndex()));
//    }
    // 自变量设置
    GwmVariableItemModel* selectedIndepVarModel = ui->mIndepVarSelector->selectedIndepVarModel();
    if (selectedIndepVarModel)
    {
        if (selectedIndepVarModel->rowCount() > 0)
        {
            mTaskThread->setVariables(selectedIndepVarModel->attributeItemList());
        }
//        else if (ui->mVariableAutoSelectionCheck->isChecked())
//        {
//            GwmVariableItemModel* indepVarModel = ui->mIndepVarSelector->indepVarModel();
//            if (indepVarModel)
//            {
//                mTaskThread->setIndependentVariables(indepVarModel->attributeItemList());
//            }
//        }
//        mTaskThread->setIsAutoselectIndepVars(ui->mVariableAutoSelectionCheck->isChecked());
//        mTaskThread->setIndepVarSelectionThreshold(ui->mModelSelAICThreshold->value());
    }
    // 带宽设置
    if (ui->mBwSizeAutomaticRadio->isChecked())
    {
        mTaskThread->setIsAutoselectBandwidth(true);
        //mTaskThread->setBandwidthSelectionCriterionType(bandwidthSelectionApproach());
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
    //mTaskThread->setHasHatMatrix(ui->cbxHatmatrix->isChecked());
    //mTaskThread->setHasFTest(ui->cbxFTest->isChecked());
    mTaskThread->setK(ui->mKspinBox->value());
    mTaskThread->setScoresCal(ui->mScoresCheckBox->isChecked());
}

void GwmGWPCAOptionsDialog::enableAccept()
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


void GwmGWPCAOptionsDialog::on_cbxHatmatrix_toggled(bool checked)
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

void GwmGWPCAOptionsDialog::on_cbkRegressionPoints_toggled(bool checked)
{
//    ui->cmbRegressionLayerSelect->setEnabled(checked);
//    if (checked)
//    {
//        ui->mBwSizeCustomizeRadio->setChecked(true);
//        ui->mBwSizeAutomaticRadio->setEnabled(false);
//        ui->mVariableAutoSelectionCheck->setChecked(false);
//        ui->mVariableAutoSelectionCheck->setEnabled(false);
//        ui->cbxHatmatrix->setEnabled(false);
//        ui->cbxHatmatrix->setChecked(false);
//    }
//    else
//    {
//        ui->mBwSizeAutomaticRadio->setEnabled(true);
//        ui->mVariableAutoSelectionCheck->setEnabled(true);
//    }
//    ui->cbxHatmatrix->setEnabled(!checked);
//    ui->cbxHatmatrix->setChecked(!checked);
}

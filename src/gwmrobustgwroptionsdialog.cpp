#include "gwmrobustgwroptionsdialog.h"
#include "ui_gwmrobustgwroptionsdialog.h"
#include <QComboBox>
#include <QButtonGroup>
#include <QFileDialog>

#include <SpatialWeight/gwmcrsdistance.h>
#include <SpatialWeight/gwmdmatdistance.h>
#include <SpatialWeight/gwmminkwoskidistance.h>

#ifdef ENABLE_OpenMP
#include <omp.h>
#endif

#ifdef ENABLE_CUDA
#include <GWmodelCUDA/ICUDAInspector.h>
#endif

GwmRobustGWROptionsDialog::GwmRobustGWROptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmRobustGWRAlgorithm* thread,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmRobustGWROptionsDialog),
    mMapLayerList(originItemList),
    mDepVarModel(new GwmVariableItemModel),
    mTaskThread(thread)
{
    ui->setupUi(this);
    ui->buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
    ui->mBwSizeAdaptiveSize->setMaximum(INT_MAX);
    ui->mBwSizeFixedSize->setMaximum(DBL_MAX);

    for (GwmLayerGroupItem* item : mMapLayerList){
        ui->mLayerComboBox->addItem(item->originChild()->layer()->name());
    }
    ui->mLayerComboBox->setCurrentIndex(-1);
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmRobustGWROptionsDialog::layerChanged);

    ui->mDepVarComboBox->setCurrentIndex(-1);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmRobustGWROptionsDialog::onDepVarChanged);

    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::onVariableRadioToggled);

    QButtonGroup* bwSizeTypeBtnGroup = new QButtonGroup(this);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeCustomizeRadio);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::onCustomizeRaidoToggled);

    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmRobustGWROptionsDialog::onDmatFileOpenClicked);

    ui->stackedWidget->setCurrentIndex(0);
    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelGPURadio);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::onNoneRadioToggled);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::onMultithreadingRadioToggled);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::onGPURadioToggled);
#ifdef ENABLE_OpenMP
    int cores = omp_get_num_procs();
    ui->mThreadNum->setValue(cores);
    ui->mThreadNum->setMaximum(cores);
#else
    ui->mCalcParallelMultithreadRadio->setEnabled(false);
#endif

#ifdef ENABLE_CUDA
    // 获取显卡信息
    ICUDAInspector* inspector = CUDAInspector_Create();
    int gpuCount = inspector->GetDeviceCount();
    if (gpuCount > 0)
    {
        for (int i = 0; i < gpuCount; ++i)
        {
            char name[255] = { "" };
            int nameLength = inspector->GetDeviceName(i);
            for (int n = 0; n < nameLength; ++n)
            {
                name[n] = inspector->GetNameChar(n);
            }
            name[nameLength] = '\0';
            QString gpuItem(name);
            ui->mGPUSelection->addItem(gpuItem);
        }
        CUDAInspector_Delete(inspector);
    }
    else
    {
        ui->mCalcParallelGPURadio->setEnabled(false);
    }
#else
    ui->mCalcParallelGPURadio->setEnabled(false);
#endif

    ui->mBwTypeAdaptiveRadio->setChecked(true);
    ui->mCalcParallelNoneRadio->setChecked(true);
    ui->mDistTypeCRSRadio->setChecked(true);

    connect(ui->cbxHatmatrix, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::on_cbxHatmatrix_toggled);

    //鲁棒GWR
    //connect(ui->cbxFilter,&QAbstractButton::toggled,this,&GwmRobustGWROptionsDialog::on_cbxFilter_toggled);

    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThetaValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mPValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileNameEdit, &QLineEdit::textChanged, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThreadNum, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mSampleGroupSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->cbxHatmatrix, &QAbstractButton::toggle, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->cbxFTest, &QAbstractButton::toggle, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    //
    //connect(ui->cbxFilter,&QAbstractButton::toggle, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mFilteredRadio,&QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mNotFilteredRadio,&QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mFilteredRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::onFilteredRadioToggled);
    connect(ui->mNotFilteredRadio, &QAbstractButton::toggled, this, &GwmRobustGWROptionsDialog::onNotFilteredRadioToggled);
}

GwmRobustGWROptionsDialog::~GwmRobustGWROptionsDialog()
{
    delete ui;
}

bool GwmRobustGWROptionsDialog::isNumeric(QVariant::Type type)
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

GwmLayerGroupItem *GwmRobustGWROptionsDialog::selectedLayer() const
{
    return mSelectedLayer;
}

void GwmRobustGWROptionsDialog::setSelectedLayer(GwmLayerGroupItem *selectedLayer)
{
    mSelectedLayer = selectedLayer;
}

void GwmRobustGWROptionsDialog::layerChanged(int index)
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

void GwmRobustGWROptionsDialog::onDepVarChanged(const int index)
{
    ui->mIndepVarSelector->onDepVarChanged(ui->mDepVarComboBox->itemText(index));
}

QString GwmRobustGWROptionsDialog::crsRotateTheta()
{
    return ui->mThetaValue->text();
}

QString GwmRobustGWROptionsDialog::crsRotateP()
{
    return ui->mPValue->text();
}

bool GwmRobustGWROptionsDialog::bandwidthType()
{
    if(ui->mBwTypeFixedRadio->isChecked()){
        return false;
    }
    else if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return true;
    }
    else return true;
}

//GwmRobustGWRTaskThread::ParallelMethod GwmRobustGWROptionsDialog::approachType()
//{
//    if(ui->mCalcParallelNoneRadio->isChecked()){
//        return GwmRobustGWRTaskThread::ParallelMethod::None;
//    }
//    else if(ui->mCalcParallelMultithreadRadio->isChecked()){
//        return GwmRobustGWRTaskThread::ParallelMethod::Multithread;
//    }
//    else if(ui->mCalcParallelGPURadio->isChecked()){
//        return GwmRobustGWRTaskThread::ParallelMethod::GPU;
//    }
//    else return GwmRobustGWRTaskThread::ParallelMethod::None;
//}

void GwmRobustGWROptionsDialog::onNoneRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void GwmRobustGWROptionsDialog::onMultithreadingRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void GwmRobustGWROptionsDialog::onGPURadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void GwmRobustGWROptionsDialog::onAutomaticRadioToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(false);
        ui->mBwSizeAdaptiveUnit->setEnabled(false);
        ui->mBwSizeFixedSize->setEnabled(false);
        ui->mBwSizeFixedUnit->setEnabled(false);
    }
}

void GwmRobustGWROptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(0);
}

void GwmRobustGWROptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(1);
}

void GwmRobustGWROptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(2);
    ui->mCalcParallelGroup->setEnabled(!checked);
}

void GwmRobustGWROptionsDialog::onDmatFileOpenClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Dmat File"), tr(""), tr("Dmat File (*.dmat)"));
    ui->mDistMatrixFileNameEdit->setText(filePath);
}

void GwmRobustGWROptionsDialog::onVariableAutoSelectionToggled(bool checked)
{

}

void GwmRobustGWROptionsDialog::onCustomizeRaidoToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(true);
        ui->mBwSizeAdaptiveUnit->setEnabled(true);
        ui->mBwSizeFixedSize->setEnabled(true);
        ui->mBwSizeFixedUnit->setEnabled(true);
    }
}

void GwmRobustGWROptionsDialog::onFixedRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(1);
    //mTaskThread->setBandwidthType(GwmRobustGWRTaskThread::BandwidthType::Fixed);
}

void GwmRobustGWROptionsDialog::onVariableRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(0);
    //mTaskThread->setBandwidthType(GwmRobustGWRTaskThread::BandwidthType::Adaptive);
}

double GwmRobustGWROptionsDialog::bandwidthSize(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        return (double)ui->mBwSizeAdaptiveSize->value();
    }
    else
    {
        return ui->mBwSizeFixedSize->value();
    }
}

QString GwmRobustGWROptionsDialog::bandWidthUnit(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        return ui->mBwSizeAdaptiveUnit->currentText();
    }
    else
    {
        return ui->mBwSizeFixedUnit->currentText();
    }
}

GwmBandwidthWeight::KernelFunctionType GwmRobustGWROptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return GwmBandwidthWeight::KernelFunctionType(kernelSelected);
}

QVariant GwmRobustGWROptionsDialog::distanceSourceParameters()
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

//GwmRobustGWRTaskThread::ParallelMethod GwmRobustGWROptionsDialog::parallelMethod()
//{
//    if (ui->mCalcParallelMultithreadRadio->isChecked())
//    {
//        return GwmRobustGWRTaskThread::ParallelMethod::Multithread;
//    }
//    else if (ui->mCalcParallelGPURadio->isChecked())
//    {
//        return GwmRobustGWRTaskThread::ParallelMethod::GPU;
//    }
//    else
//    {
//        return GwmRobustGWRTaskThread::ParallelMethod::None;
//    }
//}

QVariant GwmRobustGWROptionsDialog::parallelParameters()
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

void GwmRobustGWROptionsDialog::setTaskThread(GwmRobustGWRAlgorithm *taskThread)
{

}

void GwmRobustGWROptionsDialog::updateFieldsAndEnable()
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

void GwmRobustGWROptionsDialog::updateFields()
{
    QgsVectorLayer* dataLayer;
    // 图层设置
    if (ui->mLayerComboBox->currentIndex() > -1)
    {
        dataLayer = mSelectedLayer->originChild()->layer();
        mTaskThread->setDataLayer(dataLayer);
    }else{
        return;
    }
    // 因变量设置
    if (ui->mDepVarComboBox->currentIndex() > -1)
    {
        mTaskThread->setDependentVariable(mDepVarModel ->item(ui->mDepVarComboBox->currentIndex()));
    }
    // 自变量设置
    GwmVariableItemModel* selectedIndepVarModel = ui->mIndepVarSelector->selectedIndepVarModel();
    if (selectedIndepVarModel)
    {
        if (selectedIndepVarModel->rowCount() > 0)
        {
//            mTaskThread->setIndepVars(selectedIndepVarModel->attributeItemList());
            mTaskThread->setIndependentVariables(selectedIndepVarModel->attributeItemList());
        }
//        else
//        {
//            GwmVariableItemModel* indepVarModel = ui->mIndepVarSelector->indepVarModel();
//            if (indepVarModel)
//            {
//                mTaskThread->setIndepVars(indepVarModel->attributeItemList());
//            }
//        }
    }
    // 带宽设置
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
        mTaskThread->setGroupSize(ui->mSampleGroupSize->value());
        mTaskThread->setGPUId(ui->mGPUSelection->currentIndex());
    }
    else
    {
        mTaskThread->setParallelType(IParallelalbe::SerialOnly);
    }
    // 其他设置
    mTaskThread->setHasHatMatrix(ui->cbxHatmatrix->isChecked());
    mTaskThread->setHasFTest(ui->cbxFTest->isChecked());
    //
    //mTaskThread->setFiltered(ui->cbxFilter->isChecked());
}

void GwmRobustGWROptionsDialog::enableAccept()
{
    QString message;
    if (mTaskThread->isValid())
    {
        ui->mCheckMessage->setText(tr("Valid."));
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    }
    else
    {
        ui->mCheckMessage->setText(message);
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
    }
}


void GwmRobustGWROptionsDialog::on_cbxHatmatrix_toggled(bool checked)
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

void GwmRobustGWROptionsDialog::on_cbkRegressionPoints_toggled(bool checked)
{
    ui->cbxHatmatrix->setEnabled(!checked);
    ui->cbxHatmatrix->setChecked(!checked);
}

void GwmRobustGWROptionsDialog::on_cmbRegressionLayerSelect_currentIndexChanged(int index)
{

}

//
//void GwmRobustGWROptionsDialog::on_cbxFilter_toggled(bool checked)
//{
//    if (checked)
//    {
//        ui->cbxFilter->setEnabled(true);
//    }
//    else
//    {
//        ui->cbxFilter->setChecked(false);
//        //ui->cbxFilter->setEnabled(false);
//    }
//}

void GwmRobustGWROptionsDialog::onFilteredRadioToggled(bool checked)
{
    mTaskThread->setFiltered(true);
}

void GwmRobustGWROptionsDialog::onNotFilteredRadioToggled(bool checked)
{
    mTaskThread->setFiltered(false);
}

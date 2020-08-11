#include "gwmgtwroptionsdialog.h"
#include "ui_gwmgtwroptionsdialog.h"
#include <omp.h>
#include <QComboBox>
#include <QButtonGroup>
#include <QFileDialog>

#include <SpatialWeight/gwmcrsdistance.h>
#include <SpatialWeight/gwmdmatdistance.h>
#include <SpatialWeight/gwmminkwoskidistance.h>

#include <GWmodelCUDA/ICUDAInspector.h>


GwmGTWROptionsDialog::GwmGTWROptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmGTWRAlgorithm *thread, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmGTWROptionsDialog),
    mMapLayerList(originItemList),
    mTaskThread(thread),
    mDepVarModel(new GwmVariableItemModel),
    mTimeVarModel(new GwmVariableItemModel)
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
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTWROptionsDialog::layerChanged);
    connect(ui->cmbRegressionLayerSelect, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTWROptionsDialog::updatePredict);
    connect(ui->ckbRegressionPoints, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::on_cbkRegressionPoints_toggled);

    ui->mDepVarComboBox->setCurrentIndex(-1);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTWROptionsDialog::onDepVarChanged);

    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::onVariableRadioToggled);

    QButtonGroup* bwSizeTypeBtnGroup = new QButtonGroup(this);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeAutomaticRadio);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeCustomizeRadio);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::onAutomaticRadioToggled);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::onCustomizeRaidoToggled);

    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGTWROptionsDialog::onDmatFileOpenClicked);

    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelGPURadio);
    int cores = omp_get_num_procs();
    ui->mThreadNum->setValue(cores);
    ui->mThreadNum->setMaximum(cores);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::onNoneRadioToggled);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::onMultithreadingRadioToggled);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::onGPURadioToggled);

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

    ui->mBwTypeAdaptiveRadio->setChecked(true);
    ui->mBwSizeAutomaticRadio->setChecked(true);
    ui->mCalcParallelNoneRadio->setChecked(true);
    ui->mDistTypeCRSRadio->setChecked(true);

    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->ckbRegressionPoints, &QAbstractButton::toggle, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->cmbRegressionLayerSelect, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAutomaticApprochCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThetaValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mPValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileNameEdit, &QLineEdit::textChanged, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThreadNum, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mSampleGroupSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGTWROptionsDialog::updateFieldsAndEnable);
    connect(ui->cbxHatmatrix, &QAbstractButton::toggle, this, &GwmGTWROptionsDialog::updateFieldsAndEnable);

    updateFieldsAndEnable();
}

GwmGTWROptionsDialog::~GwmGTWROptionsDialog()
{
    delete ui;
}

bool GwmGTWROptionsDialog::isNumeric(QVariant::Type type)
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

GwmLayerGroupItem *GwmGTWROptionsDialog::selectedLayer() const
{
    return mSelectedLayer;
}

void GwmGTWROptionsDialog::setSelectedLayer(GwmLayerGroupItem *selectedLayer)
{
    mSelectedLayer = selectedLayer;
}

bool GwmGTWROptionsDialog::hasRegressionLayer() const
{
    return ui->ckbRegressionPoints->checkState() == Qt::Checked;
}

GwmLayerGroupItem *GwmGTWROptionsDialog::selectedRegressionLayer() const
{
    if (ui->ckbRegressionPoints->isChecked())
    {
        int regLayerIndex = ui->cmbRegressionLayerSelect->currentIndex();
        if (regLayerIndex > -1)
        {
            return mMapLayerList[regLayerIndex];
        }
    }
    return nullptr;
}

void GwmGTWROptionsDialog::layerChanged(int index)
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
    mTimeVarModel->clear();
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
            mTimeVarModel->append(item);
            ui->mDepVarComboBox->addItem(field.name());
            ui->mTimeVarCombo->addItem(field.name());
        }
    }
}

void GwmGTWROptionsDialog::updatePredict()
{
    bool flag = false;
    if (ui->ckbRegressionPoints->isChecked() && ui->mLayerComboBox->currentIndex() > -1 && ui->cmbRegressionLayerSelect->currentIndex() > -1)
    {
        int iDataLayer = ui->mLayerComboBox->currentIndex();
        int iRegLayer = ui->cmbRegressionLayerSelect->currentIndex();
        if (iDataLayer == iRegLayer) flag = true;
        else
        {
            QgsVectorLayer* dataLayer = mMapLayerList[iDataLayer]->originChild()->layer();
            QgsFields dataFields = dataLayer->fields();
            QgsVectorLayer* regLayer = mMapLayerList[iRegLayer]->originChild()->layer();
            QgsFields regFields = regLayer->fields();
            bool all = true;
            for (auto df : dataFields)
            {
                bool have = false;
                for (auto rf : regFields)
                {
                    if (df.name() == rf.name() && df.type() == rf.type())
                        have = true;
                }
                all = all && have;
            }
            if (all) flag = true;
//            GwmVariableItemModel* selectedIndepVarModel = ui->mIndepVarSelector->selectedIndepVarModel();
//            if (selectedIndepVarModel && selectedIndepVarModel->rowCount() > 0)
//            {
//                auto dataFields = selectedIndepVarModel->attributeItemList();

//            }
        }
    }
    ui->ckbPredict->setEnabled(flag);
}

void GwmGTWROptionsDialog::onDepVarChanged(const int index)
{
    ui->mIndepVarSelector->onDepVarChanged(ui->mDepVarComboBox->itemText(index));
}

QString GwmGTWROptionsDialog::crsRotateTheta()
{
    return ui->mThetaValue->text();
}

QString GwmGTWROptionsDialog::crsRotateP()
{
    return ui->mPValue->text();
}

bool GwmGTWROptionsDialog::bandwidthType()
{
    if(ui->mBwTypeFixedRadio->isChecked()){
        return false;
    }
    else if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return true;
    }
    else return true;
}

GwmGWRTaskThread::ParallelMethod GwmGTWROptionsDialog::approachType()
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

void GwmGTWROptionsDialog::onNoneRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void GwmGTWROptionsDialog::onMultithreadingRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void GwmGTWROptionsDialog::onGPURadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void GwmGTWROptionsDialog::onAutomaticRadioToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(false);
        ui->mBwSizeAdaptiveUnit->setEnabled(false);
        ui->mBwSizeFixedSize->setEnabled(false);
        ui->mBwSizeFixedUnit->setEnabled(false);
    }
}

void GwmGTWROptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(0);
}

void GwmGTWROptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(1);
}

void GwmGTWROptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(2);
    ui->mCalcParallelGroup->setEnabled(!checked);
}

void GwmGTWROptionsDialog::onDmatFileOpenClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Dmat File"), tr(""), tr("Dmat File (*.dmat)"));
    ui->mDistMatrixFileNameEdit->setText(filePath);
}

void GwmGTWROptionsDialog::onCustomizeRaidoToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(true);
        ui->mBwSizeAdaptiveUnit->setEnabled(true);
        ui->mBwSizeFixedSize->setEnabled(true);
        ui->mBwSizeFixedUnit->setEnabled(true);
    }
}

void GwmGTWROptionsDialog::onFixedRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(1);
}

void GwmGTWROptionsDialog::onVariableRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(0);
}

double GwmGTWROptionsDialog::bandwidthSize(){
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

GwmGTWRAlgorithm::BandwidthSelectionCriterionType GwmGTWROptionsDialog::bandwidthSelectionApproach()
{
    switch (ui->mBwSizeAutomaticApprochCombo->currentIndex())
    {
    case 0:
        return GwmGTWRAlgorithm::BandwidthSelectionCriterionType::CV;
    default:
        return GwmGTWRAlgorithm::BandwidthSelectionCriterionType::AIC;
    }
}

QString GwmGTWROptionsDialog::bandWidthUnit(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        return ui->mBwSizeAdaptiveUnit->currentText();
    }
    else
    {
        return ui->mBwSizeFixedUnit->currentText();
    }
}

GwmBandwidthWeight::KernelFunctionType GwmGTWROptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return GwmBandwidthWeight::KernelFunctionType(kernelSelected);
}

GwmGWRTaskThread::DistanceSourceType GwmGTWROptionsDialog::distanceSourceType()
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

QVariant GwmGTWROptionsDialog::distanceSourceParameters()
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

GwmGWRTaskThread::ParallelMethod GwmGTWROptionsDialog::parallelMethod()
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

QVariant GwmGTWROptionsDialog::parallelParameters()
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

void GwmGTWROptionsDialog::setTaskThread(GwmGWRTaskThread *taskThread)
{

}

void GwmGTWROptionsDialog::updateFieldsAndEnable()
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

void GwmGTWROptionsDialog::updateFields()
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
        mTaskThread->setHasPredict(ui->ckbPredict->isChecked());
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
        mTaskThread->setBandwidthSelectionCriterionType(bandwidthSelectionApproach());
    }
    else if (ui->mBwSizeCustomizeRadio->isChecked())
    {
        mTaskThread->setIsAutoselectBandwidth(false);
    }
    GwmSpatialTemporalWeight spatialWeight;
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
    // 时间设置
    if (ui->mTimeVarCombo->currentIndex() > -1)
    {
        mTaskThread->setTimeVar(mTimeVarModel->item(ui->mTimeVarCombo->currentIndex()));
    }
    spatialWeight.setLambda((1.0 * ui->sldTimeLambda->value()) / 100.0);
    mTaskThread->setSTWeight(spatialWeight);
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
//        mTaskThread->setGroupSize(ui->mSampleGroupSize->value());
//        mTaskThread->setGPUId(ui->mGPUSelection->currentIndex());
//    }
//    else
//    {
//        mTaskThread->setParallelType(IParallelalbe::SerialOnly);
//    }
    // 其他设置
    mTaskThread->setHasHatMatrix(ui->cbxHatmatrix->isChecked());
//    mTaskThread->setHasFTest(ui->cbxFTest->isChecked());
}

void GwmGTWROptionsDialog::enableAccept()
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

void GwmGTWROptionsDialog::on_cbkRegressionPoints_toggled(bool checked)
{
    ui->cmbRegressionLayerSelect->setEnabled(checked);
    if (checked)
    {
        ui->mBwSizeCustomizeRadio->setChecked(true);
        ui->mBwSizeAutomaticRadio->setEnabled(false);
        ui->cbxHatmatrix->setEnabled(false);
        ui->cbxHatmatrix->setChecked(false);
    }
    else
    {
        ui->mBwSizeAutomaticRadio->setEnabled(true);
    }
    ui->cbxHatmatrix->setEnabled(!checked);
    ui->cbxHatmatrix->setChecked(!checked);
}

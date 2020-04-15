#include "gwmgwroptionsdialog.h"
#include "ui_gwmgwroptionsdialog.h"
#include <QComboBox>
#include <QButtonGroup>

GwmGWROptionsDialog::GwmGWROptionsDialog(QList<QgsMapLayer*> vectorLayerList,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmGWROptionsDialog),
    mMapLayerList(vectorLayerList),
    mDepVarModel(new GwmLayerAttributeItemModel)
{
    ui->setupUi(this);

    for(QgsMapLayer* layer:mMapLayerList){
        ui->mLayerComboBox->addItem(layer->name());
    }
    ui->mLayerComboBox->setCurrentIndex(-1);
    ui->mDepVarComboBox->setCurrentIndex(-1);

    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);

    QButtonGroup* bwSizeTypeBtnGroup = new QButtonGroup(this);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeAutomaticRadio);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeCustomizeRadio);

    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);

    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelGPURadio);

    connect(ui->mLayerComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(layerChanged(int)));
    connect(ui->mDepVarComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDepVarChanged(int)));

    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onVariableRadioToggled);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onAutomaticRadioToggled);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onCustomizeRaidoToggled);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onNoneRadioToggled);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onMultithreadingRadioToggled);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onGPURadioToggled);

    ui->mBwTypeAdaptiveRadio->setChecked(true);
    ui->mBwSizeAutomaticRadio->setChecked(true);
    ui->mCalcParallelNoneRadio->setChecked(true);
}

GwmGWROptionsDialog::~GwmGWROptionsDialog()
{
    delete ui;
}

void GwmGWROptionsDialog::layerChanged(int index)
{
    ui->mIndepVarSelector->layerChanged((QgsVectorLayer*)mMapLayerList[index]);
    if (mLayer)
    {
        mLayer = nullptr;
    }
    mLayer =  (QgsVectorLayer*)mMapLayerList[index];
    auto fieldList = mLayer->fields();
    ui->mDepVarComboBox->clear();
    mDepVarModel->clear();
    for (int i = 0; i < fieldList.size(); i++)
    {
        QgsField field = fieldList[i];
        ui->mDepVarComboBox->addItem(field.name());
        GwmLayerAttributeItem* item = new GwmLayerAttributeItem();
        item->setAttributeName(field.name());
        item->setAttributeType(field.type());
        item->setAttributeIndex(i);
        mDepVarModel->appendRow(item);
    }
}

void GwmGWROptionsDialog::onDepVarChanged(const int index)
{
    ui->mIndepVarSelector->onDepVarChanged(ui->mDepVarComboBox->itemText(index));
    updateFieldsAndEnable();
}

QString GwmGWROptionsDialog::crsRotateTheta()
{
    return ui->mThetaValue->text();
}

QString GwmGWROptionsDialog::crsRotateP()
{
    return ui->mPValue->text();
}

GwmGWRTaskThread::BandwidthType GwmGWROptionsDialog::bandwidthType()
{
    if(ui->mBwTypeFixedRadio->isChecked()){
        return GwmGWRTaskThread::BandwidthType::Fixed;
    }
    else if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return GwmGWRTaskThread::BandwidthType::Adaptive;
    }
    else return GwmGWRTaskThread::BandwidthType::Fixed;
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

void GwmGWROptionsDialog::onDistTypeDmatToggled(bool checked)
{
    ui->mCalcParallelGroup->setEnabled(!checked);
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

GwmGWRTaskThread::KernelFunction GwmGWROptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return GwmGWRTaskThread::KernelFunction(kernelSelected);
}

GwmGWRTaskThread::DistanceSourceType GwmGWROptionsDialog::distanceSourceType()
{
    if (ui->mDistTypeCRSRadio)
        return GwmGWRTaskThread::DistanceSourceType::CRS;
    else if (ui->mDistTypeDmatRadio)
        return GwmGWRTaskThread::DistanceSourceType::DMatFile;
    else if (ui->mDistTypeMinkowskiRadio)
        return GwmGWRTaskThread::DistanceSourceType::Minkowski;
    else
        return GwmGWRTaskThread::DistanceSourceType::CRS;
}

QVariant GwmGWROptionsDialog::distanceSourceParameters()
{
    if (ui->mDistTypeDmatRadio)
    {
        return ui->mDistMatrixFileNameEdit->text();
    }
    else if (ui->mDistTypeMinkowskiRadio)
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
        return ui->mThreadNum->text();
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
}

void GwmGWROptionsDialog::updateFields()
{
    // 图层设置
    if (ui->mLayerComboBox->currentIndex() > -1)
    {
        mTaskThread->setLayer(static_cast<QgsVectorLayer*>(mMapLayerList[ui->mLayerComboBox->currentIndex()]));
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
        else
        {
            GwmLayerAttributeItemModel* indepVarModel = ui->mIndepVarSelector->indepVarModel();
            if (indepVarModel)
            {
                mTaskThread->setIndepVars(indepVarModel->attributeItemList());
            }
        }
        mTaskThread->setEnableIndepVarAutosel(ui->mVariableAutoSelectionCheck->isChecked());
    }
    // 带宽设置
    mTaskThread->setBandwidth(this->bandwidthType(), this->bandwidthSize(), this->bandWidthUnit());
    mTaskThread->setBandwidthKernelFunction(this->bandwidthKernelFunction());
    // 距离设置
    auto distSrcType = this->distanceSourceType();
    mTaskThread->setDistSrcType(distSrcType);
    mTaskThread->setDistSrcParameters(this->distanceSourceParameters());
    // 并行设置
    if (distSrcType != GwmGWRTaskThread::DistanceSourceType::DMatFile)
    {
        mTaskThread->setParallelMethodType(this->parallelMethod());
        mTaskThread->setParallelParameter(this->parallelParameters());
    }
}

void GwmGWROptionsDialog::enableAccept()
{
    QString message;
    if (!mTaskThread->isValid(message))
    {
        ui->mCheckMessage->setText(message);
    }
    else
    {
        ui->mCheckMessage->setText(tr("Valid."));
    }
}


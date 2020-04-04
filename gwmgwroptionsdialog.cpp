#include "gwmgwroptionsdialog.h"
#include "ui_gwmgwroptionsdialog.h"
#include <QComboBox>
#include <QButtonGroup>

GwmGWROptionsDialog::GwmGWROptionsDialog(QList<QgsMapLayer*> vectorLayerList,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmGWROptionsDialog),
    mapLayerList(vectorLayerList)
{
    ui->setupUi(this);
    for(QgsMapLayer* layer:mapLayerList){
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

    QButtonGroup* calcParallelTypeBtnBroup = new QButtonGroup(this);
    calcParallelTypeBtnBroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnBroup->addButton(ui->mCalcParallelMultithreadRadio);
    calcParallelTypeBtnBroup->addButton(ui->mCalcParallelGPURadio);

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
    ui->mIndepVarSelector->layerChanged((QgsVectorLayer*)mapLayerList[index]);
    if (mLayer)
    {
        mLayer = nullptr;
    }
    mLayer =  (QgsVectorLayer*)mapLayerList[index];
    QList<int> attributeList = mLayer->attributeList();
    ui->mDepVarComboBox->clear();
    qDebug() << "layerChanged";
    for (int index : attributeList)
    {
        QString attributeName = static_cast<QString>(mLayer->attributeDisplayName(index));
        ui->mDepVarComboBox->addItem(attributeName);
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

QVariant GwmGWROptionsDialog::bandwidthSize(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        return ui->mBwSizeAdaptiveSize->value();
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

QString GwmGWROptionsDialog::parallelGPUBatchSize() {
    return ui->mSampleGroupSize->text();
}

QString GwmGWROptionsDialog::parallelMultiThreadNum() {
    return ui->mThreadNum->text();
}

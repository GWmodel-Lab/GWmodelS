#include "gwmgwroptionsdialog.h"
#include "ui_gwmgwroptionsdialog.h"
#include <qcombobox.h>

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
    connect(ui->mLayerComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(layerChanged(int)));
    connect(ui->mAddAttributeBtn,&QPushButton::clicked,this,&GwmGWROptionsDialog::onAddAttributeBtn);
    connect(ui->mDelAttributeBtn,&QPushButton::clicked,this,&GwmGWROptionsDialog::onDelAttributeBtn);

    connect(ui->mFixedRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onFixedRadioToggled);
    connect(ui->mVariableRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onVariableRadioToggled);
    connect(ui->mAutomaticRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onAutomaticRadioToggled);
    connect(ui->mCustomizeRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onCustomizeRaidoToggled);
    connect(ui->mNoneRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onNoneRadioToggled);
    connect(ui->mMultithreadingRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onMultithreadingRadioToggled);
    connect(ui->mGPURadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onGPURadioToggled);
}

GwmGWROptionsDialog::~GwmGWROptionsDialog()
{
    delete ui;
}

void GwmGWROptionsDialog::layerChanged(int index)
{
    if(mLayer){
        mLayer = nullptr;
    }
    mLayer =  (QgsVectorLayer*)mapLayerList[index];
    QList<int> QgsAttributeList = mLayer->attributeList();
    if(mAttributeModel){
        delete mAttributeModel;
        mAttributeModel = nullptr;
    }
    if(mSelectedAttributeModel){
        delete mSelectedAttributeModel;
        mSelectedAttributeModel = nullptr;
    }
    qDebug() << "layerChanged";
    mAttributeModel = new QStandardItemModel(this);
    for(int index:QgsAttributeList)
    {
      QString string = static_cast<QString>(mLayer->attributeDisplayName(index));
      QStandardItem *item = new QStandardItem(string);
      mAttributeModel->appendRow(item);
    }
    ui->mAttributeView->setModel(mAttributeModel);
}

void GwmGWROptionsDialog::onAddAttributeBtn()
{
    if(!mSelectedAttributeModel){
        mSelectedAttributeModel = new QStandardItemModel(this);
//        qDebug() << "mSelectedAttributeModel";
    }
    QModelIndexList selected = ui->mAttributeView->selectionModel()->selectedIndexes();
    for(QModelIndex index:selected){
        if(index.isValid()){
            QStandardItem *item = mAttributeModel->itemFromIndex(index)->clone();
            mSelectedAttributeModel->appendRow(item);
        }
    }
    for(QModelIndex index:selected)
    {
        if(index.isValid()){
            mAttributeModel->removeRows(index.row(),1);
        }
    }
    ui->mSelectedAttributeView->setModel(mSelectedAttributeModel);
//    if()
}

void GwmGWROptionsDialog::onDelAttributeBtn()
{
    QModelIndexList selected = ui->mSelectedAttributeView->selectionModel()->selectedIndexes();
    for(QModelIndex index:selected){
        if(index.isValid()){
            QStandardItem *item = mSelectedAttributeModel->itemFromIndex(index)->clone();
            mAttributeModel->appendRow(item);
        }
    }
    for(QModelIndex index:selected)
    {
        if(index.isValid()){
            mSelectedAttributeModel->removeRows(index.row(),1);
        }
    }
}

QString GwmGWROptionsDialog::thetaValue()
{
    return ui->mThetaValue->text();
}

QString GwmGWROptionsDialog::pValue()
{
    return ui->mPValue->text();
}

GwmGWROptionsDialog::GwmBandWidthType GwmGWROptionsDialog::bandwidthType()
{
    if(ui->mFixedRadio->isChecked()){
        return GwmGWROptionsDialog::GwmBandWidthType::Fixed;
    }
    else if(ui->mVariableRadio->isChecked()){
        return GwmGWROptionsDialog::GwmBandWidthType::Variable;
    }
    return GwmGWROptionsDialog::GwmBandWidthType::Fixed;
}

GwmGWROptionsDialog::GwmApproachType GwmGWROptionsDialog::approachType()
{
    if(ui->mNoneRadio->isChecked()){
        return GwmGWROptionsDialog::GwmApproachType::None;
    }
    else if(ui->mMultithreadingRadio->isChecked()){
        return GwmGWROptionsDialog::GwmApproachType::Multithreading;
    }
    else if(ui->mGPURadio->isChecked()){
        return GwmGWROptionsDialog::GwmApproachType::GPU;
    }
    return GwmGWROptionsDialog::GwmApproachType::None;
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
    if(checked){
        ui->mBandWidthSize->setEnabled(false);
        ui->mBandWidthUnit->setEnabled(false);
    }
}

void GwmGWROptionsDialog::onCustomizeRaidoToggled(bool checked)
{
    if(checked){
        ui->mBandWidthSize->setEnabled(true);
        ui->mBandWidthUnit->setEnabled(true);
    }
}

void GwmGWROptionsDialog::onFixedRadioToggled(bool checked){
    ui->mBandWidthUnit->clear();
    ui->mBandWidthUnit->addItem("metre");
    ui->mBandWidthUnit->addItem("kilometre");
    ui->mBandWidthUnit->addItem("mile");
}

void GwmGWROptionsDialog::onVariableRadioToggled(bool checked){
    ui->mBandWidthUnit->clear();
    ui->mBandWidthUnit->addItem("one");
    ui->mBandWidthUnit->addItem("ten");
    ui->mBandWidthUnit->addItem("hundred");
}

QString GwmGWROptionsDialog::bandwidthSize(){
    return ui->mBandWidthSize->text();
}

QString GwmGWROptionsDialog::bandWidthUnit(){
    return ui->mBandWidthUnit->currentText();
}
QString GwmGWROptionsDialog::sampleGroupSize() {
    return ui->mSampleGroupSize->text();
}
QString GwmGWROptionsDialog::threadNum() {
    return ui->mThreadNum->text();
}

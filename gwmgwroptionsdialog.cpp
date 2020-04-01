#include "gwmgwroptionsdialog.h"
#include "ui_gwmgwroptionsdialog.h"
#include <QComboBox>
#include <QButtonGroup>

GwmGWROptionsDialog::GwmGWROptionsDialog(QList<QgsMapLayer*> vectorLayerList,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmGWROptionsDialog),
    mapLayerList(vectorLayerList),
    mIndepVarModel(new QStandardItemModel),
    mSelectedIndepVarModel(new QStandardItemModel)
{
    ui->setupUi(this);
    for(QgsMapLayer* layer:mapLayerList){
        ui->mLayerComboBox->addItem(layer->name());
    }
    ui->mLayerComboBox->setCurrentIndex(-1);
    ui->mDepVarComboBox->setCurrentIndex(-1);

    ui->mIndepVarView->setModel(mIndepVarModel);
    ui->mSelectedIndepVarView->setModel(mSelectedIndepVarModel);

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
    connect(ui->mAddIndepVarBtn,&QPushButton::clicked,this,&GwmGWROptionsDialog::onAddIndepVarBtn);
    connect(ui->mDelIndepVarBtn,&QPushButton::clicked,this,&GwmGWROptionsDialog::onDelIndepVarBtn);

    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onVariableRadioToggled);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onAutomaticRadioToggled);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onCustomizeRaidoToggled);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onNoneRadioToggled);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onMultithreadingRadioToggled);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGWROptionsDialog::onGPURadioToggled);
}

GwmGWROptionsDialog::~GwmGWROptionsDialog()
{
    delete ui;
}

void GwmGWROptionsDialog::layerChanged(int index)
{
    if (mLayer)
    {
        mLayer = nullptr;
    }
    mLayer =  (QgsVectorLayer*)mapLayerList[index];
    QList<int> QgsAttributeList = mLayer->attributeList();
    if (mIndepVarModel)
    {
        mIndepVarModel->clear();
    }
    if (mSelectedIndepVarModel)
    {
        mSelectedIndepVarModel->clear();
    }
    qDebug() << "layerChanged";
    ui->mDepVarComboBox->clear();
    for (int index:QgsAttributeList)
    {
        QString attributeName = static_cast<QString>(mLayer->attributeDisplayName(index));
        ui->mDepVarComboBox->addItem(attributeName);
    }
    ui->mIndepVarView->setModel(mIndepVarModel);
}

void GwmGWROptionsDialog::onDepVarChanged(const int index)
{
    QString depVarName = ui->mDepVarComboBox->itemText(index);
    if (mIndepVarModel)
    {
        mIndepVarModel->clear();
    }
    QList<int> QgsAttributeList = mLayer->attributeList();
    for(int index:QgsAttributeList)
    {
        QString attributeName = static_cast<QString>(mLayer->attributeDisplayName(index));
        if (attributeName != depVarName)
        {
            QStandardItem *item = new QStandardItem(attributeName);
            mIndepVarModel->appendRow(item);
        }
    }
    if (mSelectedIndepVarModel)
    {
        QList<QStandardItem*> removeItems = mSelectedIndepVarModel->findItems(depVarName);
        for (QStandardItem* item : removeItems)
        {
            mSelectedIndepVarModel->removeRow(mSelectedIndepVarModel->indexFromItem(item).row());
        }
    }
}

void GwmGWROptionsDialog::onAddIndepVarBtn()
{
    if(!mSelectedIndepVarModel){
        mSelectedIndepVarModel = new QStandardItemModel(this);
//        qDebug() << "mSelectedAttributeModel";
    }
    QModelIndexList selected = ui->mIndepVarView->selectionModel()->selectedIndexes();
    for(QModelIndex index:selected){
        if(index.isValid()){
            QStandardItem *item = mIndepVarModel->itemFromIndex(index)->clone();
            mSelectedIndepVarModel->appendRow(item);
        }
    }
    for(QModelIndex index:selected)
    {
        if(index.isValid()){
            mIndepVarModel->removeRows(index.row(),1);
        }
    }
    ui->mSelectedIndepVarView->setModel(mSelectedIndepVarModel);
//    if()
}

void GwmGWROptionsDialog::onDelIndepVarBtn()
{
    QModelIndexList selected = ui->mSelectedIndepVarView->selectionModel()->selectedIndexes();
    for(QModelIndex index:selected){
        if(index.isValid()){
            QStandardItem *item = mSelectedIndepVarModel->itemFromIndex(index)->clone();
            mIndepVarModel->appendRow(item);
        }
    }
    for(QModelIndex index:selected)
    {
        if(index.isValid()){
            mSelectedIndepVarModel->removeRows(index.row(),1);
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
    if(ui->mBwTypeFixedRadio->isChecked()){
        return GwmGWROptionsDialog::GwmBandWidthType::Fixed;
    }
    else if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return GwmGWROptionsDialog::GwmBandWidthType::Variable;
    }
    return GwmGWROptionsDialog::GwmBandWidthType::Fixed;
}

GwmGWROptionsDialog::GwmApproachType GwmGWROptionsDialog::approachType()
{
    if(ui->mCalcParallelNoneRadio->isChecked()){
        return GwmGWROptionsDialog::GwmApproachType::None;
    }
    else if(ui->mCalcParallelMultithreadRadio->isChecked()){
        return GwmGWROptionsDialog::GwmApproachType::Multithreading;
    }
    else if(ui->mCalcParallelGPURadio->isChecked()){
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
        ui->mBwSizeValue->setEnabled(false);
        ui->mBwSizeUnit->setEnabled(false);
    }
}

void GwmGWROptionsDialog::onCustomizeRaidoToggled(bool checked)
{
    if(checked){
        ui->mBwSizeValue->setEnabled(true);
        ui->mBwSizeUnit->setEnabled(true);
    }
}

void GwmGWROptionsDialog::onFixedRadioToggled(bool checked){
    ui->mBwSizeUnit->clear();
    ui->mBwSizeUnit->addItem("metre");
    ui->mBwSizeUnit->addItem("kilometre");
    ui->mBwSizeUnit->addItem("mile");
}

void GwmGWROptionsDialog::onVariableRadioToggled(bool checked){
    ui->mBwSizeUnit->clear();
    ui->mBwSizeUnit->addItem("one");
    ui->mBwSizeUnit->addItem("ten");
    ui->mBwSizeUnit->addItem("hundred");
}

QString GwmGWROptionsDialog::bandwidthSize(){
    return ui->mBwSizeValue->text();
}

QString GwmGWROptionsDialog::bandWidthUnit(){
    return ui->mBwSizeUnit->currentText();
}
QString GwmGWROptionsDialog::sampleGroupSize() {
    return ui->mSampleGroupSize->text();
}
QString GwmGWROptionsDialog::threadNum() {
    return ui->mThreadNum->text();
}

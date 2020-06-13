#include "gwmscalablegwroptionsdialog.h"
#include "ui_gwmscalablegwroptionsdialog.h"
#include <omp.h>
#include <QComboBox>
#include <QButtonGroup>
#include <QFileDialog>


GwmScalableGWROptionsDialog::GwmScalableGWROptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmScalableGWRTaskThread* thread,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmScalableGWROptionsDialog),
    mMapLayerList(originItemList),
    mDepVarModel(new GwmLayerAttributeItemModel),
    mTaskThread(thread)
{
    ui->setupUi(this);
    ui->mBwSizeAdaptiveSize->setMaximum(INT_MAX);
    ui->mBwSizeFixedSize->setMaximum(DBL_MAX);

    for (GwmLayerGroupItem* item : mMapLayerList){
        ui->mLayerComboBox->addItem(item->originChild()->layer()->name());
    }
    ui->mLayerComboBox->setCurrentIndex(-1);
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmScalableGWROptionsDialog::layerChanged);

    ui->mDepVarComboBox->setCurrentIndex(-1);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmScalableGWROptionsDialog::onDepVarChanged);

    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmScalableGWROptionsDialog::onVariableRadioToggled);

    QButtonGroup* bwSizeTypeBtnGroup = new QButtonGroup(this);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeCustomizeRadio);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmScalableGWROptionsDialog::onCustomizeRaidoToggled);

    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmScalableGWROptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmScalableGWROptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmScalableGWROptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmScalableGWROptionsDialog::onDmatFileOpenClicked);

    ui->mBwTypeAdaptiveRadio->setChecked(true);
    ui->mDistTypeCRSRadio->setChecked(true);

    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThetaValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mPValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileNameEdit, &QLineEdit::textChanged, this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);
    connect(ui->mPolynomialSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmScalableGWROptionsDialog::updateFieldsAndEnable);

    updateFieldsAndEnable();
}

GwmScalableGWROptionsDialog::~GwmScalableGWROptionsDialog()
{
    delete ui;
}

bool GwmScalableGWROptionsDialog::isNumeric(QVariant::Type type)
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

GwmLayerGroupItem *GwmScalableGWROptionsDialog::selectedLayer() const
{
    return mSelectedLayer;
}

void GwmScalableGWROptionsDialog::setSelectedLayer(GwmLayerGroupItem *selectedLayer)
{
    mSelectedLayer = selectedLayer;
}

void GwmScalableGWROptionsDialog::layerChanged(int index)
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

void GwmScalableGWROptionsDialog::onDepVarChanged(const int index)
{
    ui->mIndepVarSelector->onDepVarChanged(ui->mDepVarComboBox->itemText(index));
}

QString GwmScalableGWROptionsDialog::crsRotateTheta()
{
    return ui->mThetaValue->text();
}

QString GwmScalableGWROptionsDialog::crsRotateP()
{
    return ui->mPValue->text();
}

GwmGWRTaskThread::BandwidthType GwmScalableGWROptionsDialog::bandwidthType()
{
    if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return GwmGWRTaskThread::BandwidthType::Adaptive;
    }
    else return GwmGWRTaskThread::BandwidthType::Fixed;
}

void GwmScalableGWROptionsDialog::onAutomaticRadioToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(false);
        ui->mBwSizeAdaptiveUnit->setEnabled(false);
        ui->mBwSizeFixedSize->setEnabled(false);
        ui->mBwSizeFixedUnit->setEnabled(false);
    }
}

void GwmScalableGWROptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(0);
}

void GwmScalableGWROptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(1);
}

void GwmScalableGWROptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(2);
}

void GwmScalableGWROptionsDialog::onDmatFileOpenClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Dmat File"), tr(""), tr("Dmat File (*.dmat)"));
    ui->mDistMatrixFileNameEdit->setText(filePath);
}

void GwmScalableGWROptionsDialog::onCustomizeRaidoToggled(bool checked)
{
    if (checked){
        ui->mBwSizeAdaptiveSize->setEnabled(true);
        ui->mBwSizeAdaptiveUnit->setEnabled(true);
        ui->mBwSizeFixedSize->setEnabled(true);
        ui->mBwSizeFixedUnit->setEnabled(true);
    }
}

void GwmScalableGWROptionsDialog::onFixedRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(1);
    mTaskThread->setBandwidthType(GwmGWRTaskThread::BandwidthType::Fixed);
}

void GwmScalableGWROptionsDialog::onVariableRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(0);
    mTaskThread->setBandwidthType(GwmGWRTaskThread::BandwidthType::Adaptive);
}

double GwmScalableGWROptionsDialog::bandwidthSize(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        return (double)ui->mBwSizeAdaptiveSize->value();
    }
    else
    {
        return ui->mBwSizeFixedSize->value();
    }
}

QString GwmScalableGWROptionsDialog::bandWidthUnit(){
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        return ui->mBwSizeAdaptiveUnit->currentText();
    }
    else
    {
        return ui->mBwSizeFixedUnit->currentText();
    }
}

GwmGWRTaskThread::KernelFunction GwmScalableGWROptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return GwmGWRTaskThread::KernelFunction(kernelSelected);
}

GwmGWRTaskThread::DistanceSourceType GwmScalableGWROptionsDialog::distanceSourceType()
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

QVariant GwmScalableGWROptionsDialog::distanceSourceParameters()
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

void GwmScalableGWROptionsDialog::setTaskThread(GwmGWRTaskThread *taskThread)
{

}

void GwmScalableGWROptionsDialog::updateFieldsAndEnable()
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

void GwmScalableGWROptionsDialog::updateFields()
{
    // 图层设置
    if (ui->mLayerComboBox->currentIndex() > -1)
    {
        mTaskThread->setLayer(mSelectedLayer->originChild()->layer());
    }
    // 因变量设置
    if (ui->mDepVarComboBox->currentIndex() > -1)
    {
        mTaskThread->setDepVar(mDepVarModel->item(ui->mDepVarComboBox->currentIndex()));
    }
    // 自变量设置
    GwmVariableItemModel* selectedIndepVarModel = ui->mIndepVarSelector->selectedIndepVarModel();
    if (selectedIndepVarModel)
    {
        if (selectedIndepVarModel->rowCount() > 0)
        {
//            mTaskThread->setIndepVars(selectedIndepVarModel->attributeItemList());
        }
    }
    // 带宽设置
    if (ui->mBwSizeCustomizeRadio->isChecked())
    {
        mTaskThread->setIsBandwidthSizeAutoSel(false);
        mTaskThread->setBandwidth(this->bandwidthType(), this->bandwidthSize(), this->bandWidthUnit());
    }
    mTaskThread->setBandwidthKernelFunction(this->bandwidthKernelFunction());
    // 距离设置
    auto distSrcType = this->distanceSourceType();
    mTaskThread->setDistSrcType(distSrcType);
    mTaskThread->setDistSrcParameters(this->distanceSourceParameters());
    // 参数设置
    mTaskThread->setPolynomial(ui->mPolynomialSpin->value());
}

void GwmScalableGWROptionsDialog::enableAccept()
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

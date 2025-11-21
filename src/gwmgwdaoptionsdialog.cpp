#include "gwmgwdaoptionsdialog.h"
#include "ui_gwmgwdaoptionsdialog.h"

#include <QButtonGroup>

GwmGWDAOptionsDialog::GwmGWDAOptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmGWDATaskThread * thread, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GwmGWDAOptionsDialog)
    , mMapLayerList(originItemList)
    , mDepVarModel(new GwmVariableItemModel)
    , mTaskThread(thread)
{
    ui->setupUi(this);

    // 初始化图层列表
    for (GwmLayerGroupItem* item : mMapLayerList){
        ui->mLayerComboBox_2->addItem(item->originChild()->layer()->name());
    }
    ui->mLayerComboBox_2->setCurrentIndex(-1);

    // 连接图层选择信号
    connect(ui->mLayerComboBox_2, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &GwmGWDAOptionsDialog::layerChanged);


    // 连接分组变量选择信号
    ui->mDepVarComboBox->setCurrentIndex(-1);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &GwmGWDAOptionsDialog::onDepVarChanged);

    // bandwidth type button group
    QButtonGroup* bandwidthTypeSettingBtnGroup = new QButtonGroup(this);
    bandwidthTypeSettingBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bandwidthTypeSettingBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGWDAOptionsDialog::onBwTypeAdaptiveToggled);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGWDAOptionsDialog::onBwTypeFixedToggled);

    // 添加以下代码：设置 Distance Metric 按钮组和信号连接
    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);

    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGWDAOptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGWDAOptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGWDAOptionsDialog::onDistTypeDmatToggled);

    connect(ui->cbxWQDA, &QAbstractButton::toggled, this, &GwmGWDAOptionsDialog::isWqda);
    connect(ui->cbxCovMatrix, &QAbstractButton::toggled, this, &GwmGWDAOptionsDialog::hasCov);
    connect(ui->cbxLocalMean, &QAbstractButton::toggled, this, &GwmGWDAOptionsDialog::hasMean);
    connect(ui->cbxLocalPrior, &QAbstractButton::toggled, this, &GwmGWDAOptionsDialog::hasPrior);

    // 设置默认选中 CRS（如果需要）
    ui->mDistTypeCRSRadio->setChecked(true);
}

GwmGWDAOptionsDialog::~GwmGWDAOptionsDialog()
{
    delete ui;
}

GwmLayerGroupItem *GwmGWDAOptionsDialog::selectedLayer() const
{
    return mSelectedLayer;
}

void GwmGWDAOptionsDialog::setSelectedLayer(GwmLayerGroupItem *selectedLayer)
{
    mSelectedLayer = selectedLayer;
}

void GwmGWDAOptionsDialog::onBwTypeAdaptiveToggled(bool checked)
{
    if(checked)
        ui->mBwSizeSettingStack->setCurrentIndex(0);
}

void GwmGWDAOptionsDialog::onBwTypeFixedToggled(bool checked)
{
    if(checked)
        ui->mBwSizeSettingStack->setCurrentIndex(1);
}

void GwmGWDAOptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(0);
}

void GwmGWDAOptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(1);
}

void GwmGWDAOptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(2);
    // 如果 DMat 距离不支持并行计算，可以禁用并行选项（参考 GWR 的实现）
    // ui->mCalcParallelGroup->setEnabled(!checked);
}

bool GwmGWDAOptionsDialog::isNumeric(QVariant::Type type)
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

void GwmGWDAOptionsDialog::layerChanged(int index)
{
    if (index < 0 || index >= mMapLayerList.size())
        return;

    // 更新自变量选择器
    ui->mIndepVarSelector->layerChanged(mMapLayerList[index]->originChild()->layer());

    // 更新选中的图层
    if (mSelectedLayer)
    {
        mSelectedLayer = nullptr;
    }
    mSelectedLayer = mMapLayerList[index];

    // 获取图层字段
    QgsFields fieldList = mSelectedLayer->originChild()->layer()->fields();

    // 清空并填充分组变量（因变量）ComboBox
    // 注意：GWDA 的分组变量是数值型（如 0, 1, 2），所以只显示数值型字段
    ui->mDepVarComboBox->clear();
    //QList<GwmVariable>* mDepVarModel;
    for (int i = 0; i < fieldList.size(); i++)
    {
        QgsField field = fieldList[i];
        if (isNumeric(field.type()))  // 只显示数值型字段
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

    // 如果已经有选中的分组变量，更新自变量选择器以排除它
    if (ui->mDepVarComboBox->currentIndex() >= 0)
    {
        onDepVarChanged(ui->mDepVarComboBox->currentIndex());
    }
}

void GwmGWDAOptionsDialog::onDepVarChanged(const int index)
{
    if (index < 0)
        return;

    // 获取选中的分组变量名称
    QString depVarName = ui->mDepVarComboBox->itemText(index);

    // 更新自变量选择器，排除已选择的因变量
    // 这会重新加载候选变量列表，排除分组变量
    ui->mIndepVarSelector->onDepVarChanged(depVarName);
}

void GwmGWDAOptionsDialog::updateFields()
{
    qDebug()<<"updateFields start";
    QgsVectorLayer* dataLayer;

    // 1. 图层设置
    if (ui->mLayerComboBox_2->currentIndex() > -1)
    {
        dataLayer = mSelectedLayer->originChild()->layer();
        //mTaskThread->setDataLayer(dataLayer);
        if (mTaskThread != nullptr)  // ← 添加空指针检查
        {
            mTaskThread->setDataLayer(dataLayer);
        }
    }
    else
    {
        return;
    }
    qDebug()<<"1 completed";

    // 2. 分组变量（因变量）设置
    if (ui->mDepVarComboBox->currentIndex() > -1)
    {
        mTaskThread->setGroupVariable(mDepVarModel->item(ui->mDepVarComboBox->currentIndex()));
    }

    // 3. 自变量设置
    GwmVariableItemModel* selectedIndepVarModel = ui->mIndepVarSelector->selectedIndepVarModel();
    if (selectedIndepVarModel && selectedIndepVarModel->rowCount() > 0)
    {
        mTaskThread->setIndependentVariables(selectedIndepVarModel->attributeItemList());
    }

    // 4. 空间权重设置
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
        QMap<QString, QVariant> params = distanceSourceParameters().toMap();
        double theta = params["theta"].toDouble();
        double p = params["p"].toDouble();
        GwmMinkwoskiDistance distance(featureCount, p, theta);
        spatialWeight.setDistance(distance);
    }
    else  // CRS
    {
        GwmCRSDistance distance(featureCount, dataLayer->crs().isGeographic());
        spatialWeight.setDistance(distance);
    }
    mTaskThread->setSpatialWeight(spatialWeight);

    // 5. GWDA 特定参数设置
    mTaskThread->setIsWqda(isWqda());
    mTaskThread->setHascov(hasCov());
    mTaskThread->setHasmean(hasMean());
    mTaskThread->setHasprior(hasPrior());

    // 6. 并行设置
    if (ui->mCalcParallelNoneRadio->isChecked())
    {
        mTaskThread->setParallelType(IParallelalbe::SerialOnly);
    }
    else if (ui->mCalcParallelMultithreadRadio->isChecked())
    {
        mTaskThread->setParallelType(IParallelalbe::OpenMP);
        mTaskThread->setOmpThreadNum(ui->mThreadNum->value());
    }
    else
    {
        mTaskThread->setParallelType(IParallelalbe::SerialOnly);
    }
    qDebug()<<"updateFields complete";
}

void GwmGWDAOptionsDialog::updateFieldsAndEnable()
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

void GwmGWDAOptionsDialog::enableAccept()
{
    QString message;
    if (mTaskThread->isValid())
    {
        ui->mCheckMessage->setText(tr("Valid."));
        ui->btbOKCancel->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    }
    else
    {
        ui->mCheckMessage->setText(message);
        ui->btbOKCancel->setStandardButtons(QDialogButtonBox::Cancel);
    }
}

// 辅助方法
double GwmGWDAOptionsDialog::bandwidthSize()
{
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        QList<double> unit = { 1, 10, 100, 1000 };
        return (double)ui->mBwSizeAdaptiveSize->value() * unit[ui->mBwSizeAdaptiveUnit->currentIndex()];
    }
    else
    {
        QList<double> unit = { 1.0, 1000.0, 1609.344 };
        return ui->mBwSizeFixedSize->value() * unit[ui->mBwSizeFixedUnit->currentIndex()];
    }
}

bool GwmGWDAOptionsDialog::bandwidthType()
{
    return ui->mBwTypeAdaptiveRadio->isChecked();
}

GwmBandwidthWeight::KernelFunctionType GwmGWDAOptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return GwmBandwidthWeight::KernelFunctionType(kernelSelected);
}

QVariant GwmGWDAOptionsDialog::distanceSourceParameters()
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
    else
    {
        return QVariant();
    }
}

bool GwmGWDAOptionsDialog::isWqda()
{
    return ui->cbxWQDA->isChecked();  // 从 UI 控件获取
}

bool GwmGWDAOptionsDialog::hasCov()
{
    return ui->cbxCovMatrix->isChecked();  // 从 UI 控件获取
}

bool GwmGWDAOptionsDialog::hasMean()
{
    return ui->cbxLocalMean->isChecked();  // 从 UI 控件获取
}

bool GwmGWDAOptionsDialog::hasPrior()
{
    return ui->cbxLocalPrior->isChecked();  // 从 UI 控件获取
}

#include "gwmgtdroptionsdialog.h"
#include "ui_gwmgtdroptionsdialog.h"
#ifdef ENABLE_OPENMP
#include <omp.h>
#endif
#include <QComboBox>
#include <QButtonGroup>
#include <QFileDialog>
#include <QDialogButtonBox>

#include <SpatialWeight/gwmcrsdistance.h>
#include <SpatialWeight/gwmdmatdistance.h>
#include <SpatialWeight/gwmminkwoskidistance.h>


GwmGTDROptionsDialog::GwmGTDROptionsDialog(QList<GwmLayerGroupItem*> originItemList, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GwmGTDROptionsDialog),
    mMapLayerList(originItemList),
    mDepVarModel(new GwmVariableItemModel),
    mPreviousTimeStampVarName(QString())
{
    ui->setupUi(this);

    //图层选择部分
    for (GwmLayerGroupItem* item : mMapLayerList){
        ui->mLayerComboBox->addItem(item->originChild()->layer()->name());
    }
    ui->mLayerComboBox->setCurrentIndex(-1);


    //连接图层选择部分信号
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::layerChanged);

    //因变量选择部分
    ui->mDepVarComboBox->setCurrentIndex(-1);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::onDepVarChanged);

    //自变量选择部分
    mParameterSpecifiedOptionsModel = new GwmGTDRParameterSpecifiedOptionsModel(this);
    mParameterSpecifiedOptionsSelectionModel = new QItemSelectionModel(mParameterSpecifiedOptionsModel, this);
    ui->lsvParameterSpecifiedParameterList->setModel(mParameterSpecifiedOptionsModel);
    ui->lsvParameterSpecifiedParameterList->setSelectionModel(mParameterSpecifiedOptionsSelectionModel);
    // 连接信号
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGTDROptionsDialog::onSelectedIndenpendentVariablesChanged);
    connect(ui->mIndepVarSelector_2, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGTDROptionsDialog::onSelectedWeightingVariablesChanged);
    connect(mParameterSpecifiedOptionsSelectionModel, &QItemSelectionModel::currentChanged, this, &GwmGTDROptionsDialog::onSpecifiedParameterCurrentChanged);

    // 初始化时间戳选择器
    ui->mTimeStampCombo->addItem(QStringLiteral("(None)"));  // 添加"无"选项
    ui->mTimeStampCombo->setCurrentIndex(0);  // 默认选择"无"
    // 连接时间戳选择器的信号
    connect(ui->mTimeStampCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), 
            this, &GwmGTDROptionsDialog::onTimeStampChanged);

    //带宽类型选择部分
    QButtonGroup* bwTypeBtnGroup = new QButtonGroup(this);
    bwTypeBtnGroup->addButton(ui->mBwTypeAdaptiveRadio);
    bwTypeBtnGroup->addButton(ui->mBwTypeFixedRadio);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onFixedRadioToggled);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onVariableRadioToggled);
    
    QButtonGroup* bwSizeTypeBtnGroup = new QButtonGroup(this);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeAutomaticRadio);
    bwSizeTypeBtnGroup->addButton(ui->mBwSizeCustomizeRadio);
    connect(ui->mBwSizeAutomaticRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onBwSizeAutomaticToggled);
    connect(ui->mBwSizeCustomizeRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onBwSizeCustomizeToggled);


    //距离计算部分
    QButtonGroup* distanceSettingBtnGroup = new QButtonGroup(this);
    distanceSettingBtnGroup->addButton(ui->mDistTypeCRSRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeDmatRadio);
    distanceSettingBtnGroup->addButton(ui->mDistTypeMinkowskiRadio);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onDistTypeCRSToggled);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onDistTypeMinkowskiToggled);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onDistTypeDmatToggled);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGTDROptionsDialog::onDmatFileOpenClicked);

    //并行参数设置部分
    QButtonGroup* calcParallelTypeBtnGroup = new QButtonGroup(this);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelNoneRadio);
    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelMultithreadRadio);
//    calcParallelTypeBtnGroup->addButton(ui->mCalcParallelGPURadio);
#ifdef ENABLE_OPENMP
    int cores = omp_get_num_procs();
    ui->mThreadNum->setValue(cores);
    ui->mThreadNum->setMaximum(cores);    
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onMultithreadingRadioToggled);
#else
    ui->mCalcParallelMultithreadRadio->setEnabled(false);
#endif
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onNoneRadioToggled);
    //    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::onGPURadioToggled);
    ui->mCalcParallelGPURadio->hide();

    //更新线程参数信息
    connect(ui->mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDepVarComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mIndepVarSelector, &GwmIndepVarSelectorWidget::selectedIndepVarChangedSignal, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeFixedRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwTypeAdaptiveRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    // connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    // connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    // connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    // connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    // connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mBwSizeFixedSize, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGTDROptionsDialog::onBwSizeFixedSizeChanged);
    connect(ui->mBwSizeFixedUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::onBwSizeFixedSizeChanged);
    connect(ui->mBwSizeAdaptiveSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGTDROptionsDialog::onBwSizeAdaptiveSizeChanged);
    connect(ui->mBwSizeAdaptiveUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::onBwSizeAdaptiveSizeChanged);
    connect(ui->mBwKernelFunctionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &GwmGTDROptionsDialog::onBwKernelFunctionChanged);
    connect(ui->mDistTypeCRSRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeMinkowskiRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThetaValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mPValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistTypeDmatRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileNameEdit, &QLineEdit::textChanged, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mDistMatrixFileOpenBtn, &QAbstractButton::clicked, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelNoneRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelMultithreadRadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mThreadNum, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mCalcParallelGPURadio, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mSampleGroupSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &GwmGTDROptionsDialog::updateFieldsAndEnable);
    connect(ui->mHatmatrixCheckBox, &QAbstractButton::toggled, this, &GwmGTDROptionsDialog::updateFieldsAndEnable);

    ui->mBwSizeAdaptiveSize->setMaximum(INT_MAX);
    ui->mBwSizeFixedSize->setMaximum(DBL_MAX);
    ui->mDistTypeCRSRadio->setChecked(true);
    ui->mBwTypeAdaptiveRadio->setChecked(true);
    ui->mBwSizeAutomaticRadio->setChecked(true);
    ui->mBwSizeSettingStack->setEnabled(ui->mBwSizeCustomizeRadio->isChecked());
    updateFieldsAndEnable();
}

GwmGTDROptionsDialog::~GwmGTDROptionsDialog()
{
    delete ui;
}

bool GwmGTDROptionsDialog::isNumeric(QVariant::Type type)
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

GwmLayerGroupItem *GwmGTDROptionsDialog::selectedLayer() const
{
    return mSelectedLayer;
}

void GwmGTDROptionsDialog::setSelectedLayer(GwmLayerGroupItem *selectedLayer)
{
    mSelectedLayer = selectedLayer;
}

void GwmGTDROptionsDialog::layerChanged(int index)
{
    ui->mIndepVarSelector->layerChanged(mMapLayerList[index]->originChild()->layer());
    ui->mIndepVarSelector_2->layerChanged(mMapLayerList[index]->originChild()->layer());
    if (mSelectedLayer)
    {
        mSelectedLayer = nullptr;
    }
    mSelectedLayer =  mMapLayerList[index];
    QgsFields fieldList = mSelectedLayer->originChild()->layer()->fields();
    ui->mDepVarComboBox->clear();
    ui->mIndepVarSelector->onDepVarChanged("");
    ui->mIndepVarSelector_2->onDepVarChanged("");
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

        // 为权重变量选择器添加空间坐标选项
        GwmVariableItemModel* weightingVarModel = ui->mIndepVarSelector_2->indepVarModel();
        QgsVectorLayer* layer = mSelectedLayer->originChild()->layer();
        if (weightingVarModel && layer->wkbType() != QgsWkbTypes::NoGeometry)
        {
            // 检查是否已经添加了坐标选项（避免重复添加）
            for (int i = weightingVarModel->rowCount() - 1; i >= 0; i--)
            {
                GwmVariable var = weightingVarModel->item(i);
                if (var.name == QStringLiteral("__X_COORD__") || var.name == QStringLiteral("__Y_COORD__"))
                {
                    weightingVarModel->remove(i);
                }
            }

            // 添加坐标选项（使用特殊命名约定）
            GwmVariable yCoordVar;
            yCoordVar.name = QStringLiteral("__Y_COORD__");  // 特殊命名约定
            yCoordVar.type = QVariant::Double;
            yCoordVar.index = -2;  // 使用 -2 标识为 Y 坐标
            yCoordVar.isNumeric = true;
            weightingVarModel->insert(0, yCoordVar);
            //weightingVarModel->append(yCoordVar);

            GwmVariable xCoordVar;
            xCoordVar.name = QStringLiteral("__X_COORD__");  // 特殊命名约定
            xCoordVar.type = QVariant::Double;
            xCoordVar.index = -1;  // 使用 -1 标识为 X 坐标
            xCoordVar.isNumeric = true;
            weightingVarModel->insert(0, xCoordVar);
            //weightingVarModel->append(xCoordVar);



        }

    // GwmVariableItemModel* selectedWeightingVarModel = ui->mIndepVarSelector_2->selectedIndepVarModel();
    // if (selectedWeightingVarModel && selectedWeightingVarModel->rowCount() > 0)
    // {
    //     mParameterSpecifiedOptionsModel->syncWithAttributes(selectedWeightingVarModel);
    //     if (mParameterSpecifiedOptionsModel->rowCount() > 0)
    //     {
    //         QModelIndex firstIndex = mParameterSpecifiedOptionsModel->index(0, 0);
    //         mParameterSpecifiedOptionsSelectionModel->setCurrentIndex(firstIndex, QItemSelectionModel::SelectCurrent);
    //     }
    // }

    // 更新时间戳选择器
    ui->mTimeStampCombo->clear();
    ui->mTimeStampCombo->addItem(QStringLiteral("(None)"));  // 添加"无"选项
    
    if (mSelectedLayer)
    {
        QgsVectorLayer* layer = mSelectedLayer->originChild()->layer();
        QgsFields fieldList = layer->fields();
        
        // 添加所有数值型变量到时间戳选择器
        for (int i = 0; i < fieldList.size(); i++)
        {
            QgsField field = fieldList[i];
            if (isNumeric(field.type()))
            {
                ui->mTimeStampCombo->addItem(field.name());
            }
        }
    }
    
    ui->mTimeStampCombo->setCurrentIndex(0);  // 默认选择"无"
    
    // 同步参数列表（基于权重变量）
    GwmVariableItemModel* selectedWeightingVarModel = ui->mIndepVarSelector_2->selectedIndepVarModel();
    if (selectedWeightingVarModel && selectedWeightingVarModel->rowCount() > 0)
    {
        mParameterSpecifiedOptionsModel->syncWithAttributes(selectedWeightingVarModel);
        if (mParameterSpecifiedOptionsModel->rowCount() > 0)
        {
            QModelIndex firstIndex = mParameterSpecifiedOptionsModel->index(0, 0);
            mParameterSpecifiedOptionsSelectionModel->setCurrentIndex(firstIndex, QItemSelectionModel::SelectCurrent);
        }
    }

}

void GwmGTDROptionsDialog::onDepVarChanged(const int index)
{
    ui->mIndepVarSelector->onDepVarChanged(ui->mDepVarComboBox->itemText(index));
    ui->mIndepVarSelector_2->onDepVarChanged(ui->mDepVarComboBox->itemText(index));

    // 更新时间戳选择器，排除因变量
    QString currentDepVarName;
    if (index >= 0)
    {
        currentDepVarName = ui->mDepVarComboBox->itemText(index);
    }
    
    // 保存当前选择的时间戳（如果存在）
    int currentTimeStampIndex = ui->mTimeStampCombo->currentIndex();
    QString currentTimeStampName;
    if (currentTimeStampIndex > 0)
    {
        currentTimeStampName = ui->mTimeStampCombo->itemText(currentTimeStampIndex);
    }
    
    // 重新填充时间戳选择器
    ui->mTimeStampCombo->clear();
    ui->mTimeStampCombo->addItem(QStringLiteral("(None)"));
    
    if (mSelectedLayer)
    {
        QgsVectorLayer* layer = mSelectedLayer->originChild()->layer();
        QgsFields fieldList = layer->fields();
        
        for (int i = 0; i < fieldList.size(); i++)
        {
            QgsField field = fieldList[i];
            if (isNumeric(field.type()) && field.name() != currentDepVarName)
            {
                ui->mTimeStampCombo->addItem(field.name());
            }
        }
    }
    
    // 恢复之前的选择（如果仍然存在）
    if (!currentTimeStampName.isEmpty())
    {
        int newIndex = ui->mTimeStampCombo->findText(currentTimeStampName);
        if (newIndex >= 0)
        {
            ui->mTimeStampCombo->setCurrentIndex(newIndex);
        }
        else
        {
            // 如果之前选择的时间戳变成了因变量，重置为"无"
            ui->mTimeStampCombo->setCurrentIndex(0);
            mPreviousTimeStampVarName.clear();
        }
    }
    else
    {
        ui->mTimeStampCombo->setCurrentIndex(0);
    }

    // 为权重变量选择器重新添加空间坐标选项（因为 onDepVarChanged 会清空列表）
    QgsVectorLayer* layer = mSelectedLayer ? mSelectedLayer->originChild()->layer() : nullptr;
    if (layer && layer->wkbType() != QgsWkbTypes::NoGeometry)
    {
        GwmVariableItemModel* weightingVarModel = ui->mIndepVarSelector_2->indepVarModel();
        if (weightingVarModel)
        {
            // 检查是否已经添加了坐标选项（避免重复添加）
            bool hasXCoord = false;
            bool hasYCoord = false;
            for (int i = 0; i < weightingVarModel->rowCount(); i++)
            {
                GwmVariable var = weightingVarModel->item(i);
                if (var.name == QStringLiteral("__X_COORD__"))
                    hasXCoord = true;
                if (var.name == QStringLiteral("__Y_COORD__"))
                    hasYCoord = true;
            }

            // 添加 X 坐标选项
            if (!hasXCoord)
            {
                GwmVariable xCoordVar;
                xCoordVar.name = QStringLiteral("__X_COORD__");
                xCoordVar.type = QVariant::Double;
                xCoordVar.index = -1;
                xCoordVar.isNumeric = true;
                weightingVarModel->append(xCoordVar);
            }

            // 添加 Y 坐标选项
            if (!hasYCoord)
            {
                GwmVariable yCoordVar;
                yCoordVar.name = QStringLiteral("__Y_COORD__");
                yCoordVar.type = QVariant::Double;
                yCoordVar.index = -2;
                yCoordVar.isNumeric = true;
                weightingVarModel->append(yCoordVar);
            }
        }
    }

    // 处理时间戳变量：如果当前选择了时间戳，需要从权重变量列表中移除
    int timeStampIndex = ui->mTimeStampCombo->currentIndex();
    QString currentTimeStampVarName;
    if (timeStampIndex > 0)
    {
        currentTimeStampVarName = ui->mTimeStampCombo->itemText(timeStampIndex);
    }
    
    // 如果之前有选择时间戳变量，将其添加回列表（如果需要）
    if (!mPreviousTimeStampVarName.isEmpty() && mPreviousTimeStampVarName != currentTimeStampVarName)
    {
        GwmVariableItemModel* weightingVarModel = ui->mIndepVarSelector_2->indepVarModel();
        if (weightingVarModel)
        {
            // 检查是否已经存在
            bool alreadyExists = false;
            for (int i = 0; i < weightingVarModel->rowCount(); i++)
            {
                GwmVariable var = weightingVarModel->item(i);
                if (var.name == QStringLiteral("__X_COORD__") || var.name == QStringLiteral("__Y_COORD__"))
                    continue;
                if (var.name == mPreviousTimeStampVarName)
                {
                    alreadyExists = true;
                    break;
                }
            }
            
            // 如果不存在，添加回去
            if (!alreadyExists && mSelectedLayer)
            {
                QgsVectorLayer* layer = mSelectedLayer->originChild()->layer();
                QgsFields fieldList = layer->fields();
                for (int i = 0; i < fieldList.size(); i++)
                {
                    if (fieldList[i].name() == mPreviousTimeStampVarName && isNumeric(fieldList[i].type()))
                    {
                        GwmVariable var;
                        var.name = fieldList[i].name();
                        var.type = fieldList[i].type();
                        var.index = i;
                        var.isNumeric = fieldList[i].isNumeric();
                        weightingVarModel->append(var);
                        break;
                    }
                }
            }
        }
    }
    
    // 如果当前选择了时间戳，从权重变量列表中移除
    if (!currentTimeStampVarName.isEmpty())
    {
        GwmVariableItemModel* weightingVarModel = ui->mIndepVarSelector_2->indepVarModel();
        if (weightingVarModel)
        {
            for (int i = weightingVarModel->rowCount() - 1; i >= 0; i--)
            {
                GwmVariable var = weightingVarModel->item(i);
                if (var.name == QStringLiteral("__X_COORD__") || var.name == QStringLiteral("__Y_COORD__"))
                    continue;
                if (var.name == currentTimeStampVarName)
                {
                    weightingVarModel->remove(i);
                    break;
                }
            }
        }
    }
    
    mPreviousTimeStampVarName = currentTimeStampVarName;

    // 使用新的同步方法
    syncParameterListWithTimeStamp();
}

QString GwmGTDROptionsDialog::crsRotateTheta()
{
    return ui->mThetaValue->text();
}

QString GwmGTDROptionsDialog::crsRotateP()
{
    return ui->mPValue->text();
}

bool GwmGTDROptionsDialog::bandwidthType()
{
    if(ui->mBwTypeFixedRadio->isChecked()){
        return false;
    }
    else if(ui->mBwTypeAdaptiveRadio->isChecked()){
        return true;
    }
    else return false;
}

IParallelalbe::ParallelType GwmGTDROptionsDialog::approachType()
{
    if(ui->mCalcParallelNoneRadio->isChecked()){
        return IParallelalbe::ParallelType::SerialOnly;
    }
    else if(ui->mCalcParallelMultithreadRadio->isChecked()){
        return IParallelalbe::ParallelType::OpenMP;
    }
    else if(ui->mCalcParallelGPURadio->isChecked()){
        return IParallelalbe::ParallelType::CUDA;
    }
    else return IParallelalbe::ParallelType::SerialOnly;
}


void GwmGTDROptionsDialog::onNoneRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void GwmGTDROptionsDialog::onMultithreadingRadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void GwmGTDROptionsDialog::onGPURadioToggled(bool checked)
{
    if(checked){
        ui->stackedWidget->setCurrentIndex(2);
    }
}


GwmDistance::DistanceType GwmGTDROptionsDialog::distanceSourceType()
{
    if (ui->mDistTypeCRSRadio->isChecked())
        return GwmDistance::DistanceType::OneDimDistance;
    else if (ui->mDistTypeDmatRadio->isChecked())
        return GwmDistance::DistanceType::DMatDistance;
    else if (ui->mDistTypeMinkowskiRadio->isChecked())
        return GwmDistance::DistanceType::MinkwoskiDistance;
    else
        return GwmDistance::DistanceType::OneDimDistance;
}

QVariant GwmGTDROptionsDialog::distanceSourceParameters()
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
void GwmGTDROptionsDialog::onDistTypeCRSToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(0);
}

void GwmGTDROptionsDialog::onDistTypeMinkowskiToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(1);
}

void GwmGTDROptionsDialog::onDistTypeDmatToggled(bool checked)
{
    if (checked)
        ui->mDistParamSettingStack->setCurrentIndex(2);
    ui->mCalcParallelGroup->setEnabled(!checked);
}

void GwmGTDROptionsDialog::onDmatFileOpenClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Dmat File"), tr(""), tr("Dmat File (*.dmat)"));
    ui->mDistMatrixFileNameEdit->setText(filePath);
}

void GwmGTDROptionsDialog::onFixedRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(1);
}

void GwmGTDROptionsDialog::onBwSizeAutomaticToggled(bool checked)
{
    if (checked)
        ui->mBwSizeSettingStack->setEnabled(false);
}

void GwmGTDROptionsDialog::onBwSizeCustomizeToggled(bool checked)
{
    ui->mBwSizeSettingStack->setEnabled(checked);
}

void GwmGTDROptionsDialog::onVariableRadioToggled(bool checked)
{
    ui->mBwSizeSettingStack->setCurrentIndex(0);
}

void GwmGTDROptionsDialog::onSelectedIndenpendentVariablesChanged()
{
    // 同步独立变量到列表（类似 MultiscaleGWR）
    // mParameterSpecifiedOptionsModel->syncWithAttributes(ui->mIndepVarSelector->selectedIndepVarModel());

    // 如果有项目，选中第一个
    if (mParameterSpecifiedOptionsModel->rowCount() > 0)
    {
        QModelIndex firstIndex = mParameterSpecifiedOptionsModel->index(0, 0);
        mParameterSpecifiedOptionsSelectionModel->setCurrentIndex(firstIndex, QItemSelectionModel::SelectCurrent);
    }
}

void GwmGTDROptionsDialog::onSelectedWeightingVariablesChanged()
{   
    // 使用新的同步方法，包括时间戳
    syncParameterListWithTimeStamp();
}

void GwmGTDROptionsDialog::onSpecifiedParameterCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    // 可以在这里更新右侧编辑控件（如果需要）
    // GTDR 可能不需要，因为可以直接在列表中编辑
    // Q_UNUSED(current);
    // Q_UNUSED(previous);

    // 启用/禁用编辑控件（只有当选中有效项时才启用）
    bool isValid = current.isValid();
    ui->mBwSizeSettingStack->setEnabled(isValid && ui->mBwSizeCustomizeRadio->isChecked());
    ui->mBwKernelFunctionCombo->setEnabled(isValid);

    if (!isValid)
        return;

    // 获取当前选中变量对应的参数选项
    GwmGTDRParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(current);
    if (!option)
        return;

    // 更新右侧编辑控件，显示当前选中变量的参数
    // 注意：带宽值的单位转换
    if (ui->mBwTypeAdaptiveRadio->isChecked())
    {
        // Adaptive 模式：直接显示数值（不需要单位转换，因为 adaptive 是数量）
        ui->mBwSizeAdaptiveSize->setValue(int(option->initialBandwidthSize));
    }
    else if (ui->mBwTypeFixedRadio->isChecked())
    {
        // Fixed 模式：需要根据单位转换
        // 假设默认单位是米，需要根据实际单位转换
        double value = option->initialBandwidthSize;
        QList<double> units = { 1.0, 1000.0, 1609.344 };  // 米、千米、英里
        // 尝试找到合适的单位和值
        int unitIndex = 0;
        if (value >= 1000.0 && value < 1000000.0)
        {
            unitIndex = 1;  // 千米
            value = value / 1000.0;
        }
        else if (value >= 1609.344)
        {
            unitIndex = 2;  // 英里
            value = value / 1609.344;
        }
        ui->mBwSizeFixedSize->setValue(value);
        ui->mBwSizeFixedUnit->setCurrentIndex(unitIndex);
    }

    // 更新核函数类型
    ui->mBwKernelFunctionCombo->setCurrentIndex(static_cast<int>(option->kernel));
}

void GwmGTDROptionsDialog::onBwSizeAdaptiveSizeChanged(int size)
{
    // 获取当前选中的变量
    QModelIndex currentIndex = mParameterSpecifiedOptionsSelectionModel->currentIndex();
    if (!currentIndex.isValid())
        return;

    GwmGTDRParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(currentIndex);
    if (!option)
        return;

    // 更新该变量的初始带宽值
    // Adaptive 模式：需要考虑单位
    QList<double> units = { 1, 10, 100, 1000 };
    double bandwidthValue = size * units[ui->mBwSizeAdaptiveUnit->currentIndex()];
    option->initialBandwidthSize = bandwidthValue;
}

void GwmGTDROptionsDialog::onBwSizeFixedSizeChanged(double size)
{
    // 获取当前选中的变量
    QModelIndex currentIndex = mParameterSpecifiedOptionsSelectionModel->currentIndex();
    if (!currentIndex.isValid())
        return;

    GwmGTDRParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(currentIndex);
    if (!option)
        return;

    // 更新该变量的初始带宽值
    // Fixed 模式：需要考虑单位
    QList<double> units = { 1.0, 1000.0, 1609.344 };  // 米、千米、英里
    double bandwidthValue = size * units[ui->mBwSizeFixedUnit->currentIndex()];
    option->initialBandwidthSize = bandwidthValue;
}

void GwmGTDROptionsDialog::onBwKernelFunctionChanged(int index)
{
    // 获取当前选中的变量
    QModelIndex currentIndex = mParameterSpecifiedOptionsSelectionModel->currentIndex();
    if (!currentIndex.isValid())
        return;

    GwmGTDRParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(currentIndex);
    if (!option)
        return;

    // 更新该变量的核函数类型
    option->kernel = static_cast<gwm::BandwidthWeight::KernelFunctionType>(index);
}

void GwmGTDROptionsDialog::onTimeStampChanged(int index)
{
    // 获取权重变量列表模型（所有可用的变量）
    GwmVariableItemModel* weightingVarModel = ui->mIndepVarSelector_2->indepVarModel();
    if (!weightingVarModel)
        return;
    
    // 获取当前选中的时间戳变量名（如果选择了"无"，则为空）
    QString currentTimeStampVarName;
    if (index > 0)  // index 0 是 "(None)"
    {
        currentTimeStampVarName = ui->mTimeStampCombo->itemText(index);
    }

    // 如果之前有选择时间戳变量，将其添加回权重变量列表
    if (!mPreviousTimeStampVarName.isEmpty() && mPreviousTimeStampVarName != currentTimeStampVarName)
    {
        // 检查该变量是否已经在列表中（避免重复）
        bool alreadyExists = false;
        for (int i = 0; i < weightingVarModel->rowCount(); i++)
        {
            GwmVariable var = weightingVarModel->item(i);
            // 跳过特殊命名的坐标变量
            if (var.name == QStringLiteral("__X_COORD__") || var.name == QStringLiteral("__Y_COORD__"))
                continue;
            if (var.name == mPreviousTimeStampVarName)
            {
                alreadyExists = true;
                break;
            }
        }
        
        // 如果不存在，从图层字段中查找并添加
        if (!alreadyExists && mSelectedLayer)
        {
            QgsVectorLayer* layer = mSelectedLayer->originChild()->layer();
            QgsFields fieldList = layer->fields();
            for (int i = 0; i < fieldList.size(); i++)
            {
                if (fieldList[i].name() == mPreviousTimeStampVarName && isNumeric(fieldList[i].type()))
                {
                    GwmVariable var;
                    var.name = fieldList[i].name();
                    var.type = fieldList[i].type();
                    var.index = i;
                    var.isNumeric = fieldList[i].isNumeric();
                    weightingVarModel->append(var);
                    break;
                }
            }
        }
    }

    // 如果当前选择了某个变量作为时间戳，从权重变量列表中移除
    if (!currentTimeStampVarName.isEmpty())
    {
        // 从后往前遍历，移除时间戳变量
        for (int i = weightingVarModel->rowCount() - 1; i >= 0; i--)
        {
            GwmVariable var = weightingVarModel->item(i);
            // 跳过特殊命名的坐标变量
            if (var.name == QStringLiteral("__X_COORD__") || var.name == QStringLiteral("__Y_COORD__"))
                continue;
                
            // 如果当前变量是时间戳变量，从列表中移除
            if (var.name == currentTimeStampVarName)
            {
                weightingVarModel->remove(i);
                break;  // 找到并移除后退出循环
            }
        }
    }

    // 更新之前的时间戳变量名
    mPreviousTimeStampVarName = currentTimeStampVarName;

    // 触发权重变量选择器的更新（这会导致参数列表同步）
    // 但我们需要手动触发参数列表的同步，因为时间戳不在权重变量列表中
    syncParameterListWithTimeStamp();
}

// 新增辅助方法：同步参数列表（包括时间戳）
void GwmGTDROptionsDialog::syncParameterListWithTimeStamp()
{
    // 先同步权重变量
    mParameterSpecifiedOptionsModel->syncWithAttributes(ui->mIndepVarSelector_2->selectedIndepVarModel());
    
    // 如果选择了时间戳，添加"TIMESTAMP"项到参数列表
    int timeStampIndex = ui->mTimeStampCombo->currentIndex();
    if (timeStampIndex > 0)  // 如果选择了某个变量作为时间戳
    {
        // 检查是否已经存在"TIMESTAMP"项
        bool hasTimeStamp = false;
        for (int i = 0; i < mParameterSpecifiedOptionsModel->rowCount(); i++)
        {
            QModelIndex idx = mParameterSpecifiedOptionsModel->index(i, 0);
            GwmGTDRParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(idx);
            if (option && option->attributeName == QStringLiteral("TIMESTAMP"))
            {
                hasTimeStamp = true;
                break;
            }
        }
        
        // 如果不存在，添加"TIMESTAMP"项
        if (!hasTimeStamp)
        {
            // 由于模型没有公开的添加方法，我们需要创建一个临时模型来包含TIMESTAMP
            // 创建一个临时模型，包含所有权重变量和时间戳
            GwmVariableItemModel* tempModel = new GwmVariableItemModel(this);
            GwmVariableItemModel* weightingModel = ui->mIndepVarSelector_2->selectedIndepVarModel();
            if (weightingModel)
            {
                for (int i = 0; i < weightingModel->rowCount(); i++)
                {
                    tempModel->append(weightingModel->item(i));
                }
            }
            
            // 添加TIMESTAMP虚拟变量
            GwmVariable timeStampVar;
            timeStampVar.name = QStringLiteral("TIMESTAMP");
            timeStampVar.type = QVariant::Double;  // 时间戳通常是数值型
            timeStampVar.index = -999;  // 使用特殊索引标识时间戳
            timeStampVar.isNumeric = true;
            tempModel->append(timeStampVar);
            
            // 使用临时模型同步参数列表
            mParameterSpecifiedOptionsModel->syncWithAttributes(tempModel);
            
            // 清理临时模型
            delete tempModel;
        }
    }
    
    // 如果有项目，选中第一个
    if (mParameterSpecifiedOptionsModel->rowCount() > 0)
    {
        QModelIndex firstIndex = mParameterSpecifiedOptionsModel->index(0, 0);
        mParameterSpecifiedOptionsSelectionModel->setCurrentIndex(firstIndex, QItemSelectionModel::SelectCurrent);
    }
    
    // 触发更新验证
    updateFieldsAndEnable();
}

double GwmGTDROptionsDialog::bandwidthSize(){
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


gwm::BandwidthWeight::KernelFunctionType GwmGTDROptionsDialog::bandwidthKernelFunction()
{
    int kernelSelected = ui->mBwKernelFunctionCombo->currentIndex();
    return gwm::BandwidthWeight::KernelFunctionType(kernelSelected);
}

QVariant GwmGTDROptionsDialog::parallelParameters()
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

void GwmGTDROptionsDialog::updateFieldsAndEnable()
{
    this->updateFields();
    this->enableAccept();
}

void GwmGTDROptionsDialog::updateFields()
{
    QgsVectorLayer* dataLayer;
    // 图层设置
    if (ui->mLayerComboBox->currentIndex() > -1)
    {
        dataLayer = mSelectedLayer->originChild()->layer();
        mAlgorithmMeta.layer = dataLayer;
    }
    else
    {
        return;
    }
    

    if (ui->mDepVarComboBox->currentIndex() > -1)
    {
        mAlgorithmMeta.dependentVariable = mDepVarModel->item(ui->mDepVarComboBox->currentIndex());
    }

    GwmVariableItemModel* selectedIndepVarModel = ui->mIndepVarSelector->selectedIndepVarModel();
    if (selectedIndepVarModel)
    {
        if (selectedIndepVarModel->rowCount() > 0)
        {
            mAlgorithmMeta.independentVariables = selectedIndepVarModel->attributeItemList();
        }
    }

    GwmVariableItemModel* selectedWeightingVarModel = ui->mIndepVarSelector_2->selectedIndepVarModel();
    if (selectedWeightingVarModel)
    {
        if (selectedWeightingVarModel->rowCount() > 0)
        {
            mAlgorithmMeta.weightingVariables = selectedWeightingVarModel->attributeItemList();
        }
        else
        {
            // 如果没有选择权重变量，清空列表
            mAlgorithmMeta.weightingVariables.clear();
        }
    }
    else
    {
        mAlgorithmMeta.weightingVariables.clear();
    }

    // 设置时间戳变量
    int timeStampIndex = ui->mTimeStampCombo->currentIndex();
    if (timeStampIndex > 0)  // 如果选择了某个变量作为时间戳
    {
        QString timeStampVarName = ui->mTimeStampCombo->itemText(timeStampIndex);
        // 从图层字段中查找对应的变量
        QgsVectorLayer* layer = mSelectedLayer->originChild()->layer();
        QgsFields fieldList = layer->fields();
        for (int i = 0; i < fieldList.size(); i++)
        {
            if (fieldList[i].name() == timeStampVarName)
            {
                GwmVariable timeStampVar;
                timeStampVar.name = fieldList[i].name();
                timeStampVar.type = fieldList[i].type();
                timeStampVar.index = i;
                timeStampVar.isNumeric = fieldList[i].isNumeric();
                mAlgorithmMeta.timeStampVariable = timeStampVar;  // 需要在 GwmAlgorithmMetaGTDR 中添加此字段
                break;
            }
        }
    }
    else
    {
        // 如果没有选择时间戳，设置为空变量
        mAlgorithmMeta.timeStampVariable = GwmVariable();  // 默认构造的空变量
        mAlgorithmMeta.timeStampVariable.index = -1;  // 显式设置为 -1
        mAlgorithmMeta.timeStampVariable.name = QString();  // 显式清空 name
    }

    mAlgorithmMeta.weightType = gwm::Weight::BandwidthWeight;
    mAlgorithmMeta.weightBandwidthSize = bandwidthSize();
    mAlgorithmMeta.weightBandwidthAdaptive = bandwidthType();
    mAlgorithmMeta.weightBandwidthKernel = bandwidthKernelFunction();

    // 距离设置
    if (ui->mDistTypeDmatRadio->isChecked())
    {
        mAlgorithmMeta.distanceType = gwm::Distance::DistanceType::DMatDistance;
        QString filename = ui->mDistMatrixFileNameEdit->text();
        mAlgorithmMeta.distanceDmatFilename = filename.toStdString();
    }
    else if (ui->mDistTypeMinkowskiRadio->isChecked())
    {
        mAlgorithmMeta.distanceType = gwm::Distance::DistanceType::MinkwoskiDistance;
        mAlgorithmMeta.distanceMinkowskiTheta = ui->mThetaValue->value();
        mAlgorithmMeta.distanceMinkowskiPower = ui->mPValue->value();
    }
    else
    {
        mAlgorithmMeta.distanceType = gwm::Distance::DistanceType::OneDimDistance;
        // mAlgorithmMeta.distanceCrsGeographic = dataLayer->crs().isGeographic();
    }

    // 并行设置
    if (ui->mCalcParallelNoneRadio->isChecked())
    {
        mAlgorithmMeta.parallelType = gwm::ParallelType::SerialOnly;
    }
    else if (ui->mCalcParallelMultithreadRadio->isChecked())
    {
        mAlgorithmMeta.parallelType = gwm::ParallelType::OpenMP;
        mAlgorithmMeta.parallelOmpThreads = ui->mThreadNum->value();
    }
    else if (ui->mCalcParallelGPURadio->isChecked() && !ui->mDistTypeDmatRadio->isChecked())
    {
        mAlgorithmMeta.parallelType = gwm::ParallelType::SerialOnly;
    }
    else
    {
        mAlgorithmMeta.parallelType = gwm::ParallelType::SerialOnly;
    }

    // Bandwidth Autoselection Settings
    mAlgorithmMeta.bandwidthAuto = ui->mBwSizeAutomaticRadio->isChecked();
    if (mAlgorithmMeta.bandwidthAuto)
    {
        mAlgorithmMeta.bandwidthCriterionType =
            ui->mBwSizeAutomaticApprochCombo->currentIndex() == 0
                ? gwm::GTDR::BandwidthCriterionType::AIC
                : gwm::GTDR::BandwidthCriterionType::CV;
        mAlgorithmMeta.weightBandwidthSize = bandwidthSize(); // 作为初始值使用，可保留
    }
    else
    {
        mAlgorithmMeta.weightBandwidthSize = bandwidthSize();
    }

    // 读取每个维度的初始带宽值和核函数类型
    mAlgorithmMeta.weightBandwidthSizes.clear();
    mAlgorithmMeta.weightBandwidthKernels.clear();

    for (int i = 0; i < mParameterSpecifiedOptionsModel->rowCount(); ++i)
    {
        GwmGTDRParameterSpecifiedOption* option = mParameterSpecifiedOptionsModel->item(i);
        if (option)
        {
            mAlgorithmMeta.weightBandwidthSizes.append(option->initialBandwidthSize);
            mAlgorithmMeta.weightBandwidthKernels.append(option->kernel);
        }
    }

    // 向后兼容：如果列表为空，使用单个默认值
    if (mAlgorithmMeta.weightBandwidthSizes.isEmpty())
    {
        // 使用当前 UI 中的值作为默认值
        mAlgorithmMeta.weightBandwidthSize = bandwidthSize();
        mAlgorithmMeta.weightBandwidthKernel = bandwidthKernelFunction();
    }
    else
    {
        // 如果列表不为空，也更新单个值（用于向后兼容或作为默认值）
        // 可以选择使用第一个值，或者保持当前 UI 中的值
        mAlgorithmMeta.weightBandwidthSize = mAlgorithmMeta.weightBandwidthSizes.first();
        mAlgorithmMeta.weightBandwidthKernel = mAlgorithmMeta.weightBandwidthKernels.first();
    }

    mAlgorithmMeta.hatmatrix = ui->mHatmatrixCheckBox->isChecked();

}

void GwmGTDROptionsDialog::enableAccept()
{
    QString error;
    if (mAlgorithmMeta.validate(error))
    {
        ui->mCheckMessage->setText(tr("Valid."));
        ui->btbOkCancle->setStandardButtons(QDialogButtonBox::Ok);
        ui->btbOkCancle->addButton(QDialogButtonBox::StandardButton::Cancel);
    }
    else
    {
        ui->mCheckMessage->setText(QString("Invalid: %1").arg(error));
        ui->btbOkCancle->setStandardButtons(QDialogButtonBox::Cancel);
    }
}




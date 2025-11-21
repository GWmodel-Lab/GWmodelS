#ifndef GWMGWDAOPTIONSDIALOG_H
#define GWMGWDAOPTIONSDIALOG_H

#include <QDialog>
#include <TaskThread/gwmgwdataskthread.h>
#include "Model/gwmlayergroupitem.h"

namespace Ui {
class GwmGWDAOptionsDialog;
}

class GwmGWDAOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GwmGWDAOptionsDialog(QList<GwmLayerGroupItem*> originItemList, GwmGWDATaskThread * thread,QWidget *parent = nullptr);
    ~GwmGWDAOptionsDialog();

    GwmLayerGroupItem *selectedLayer() const;
    void setSelectedLayer(GwmLayerGroupItem *selectedLayer);

private:
    Ui::GwmGWDAOptionsDialog *ui;
    QList<GwmLayerGroupItem*> mMapLayerList;
    GwmLayerGroupItem* mSelectedLayer = nullptr;
    GwmVariableItemModel* mDepVarModel;
    bool isNumeric(QVariant::Type type);    // 添加这个辅助函数

private slots:
    void onDistTypeCRSToggled(bool checked);
    void onDistTypeMinkowskiToggled(bool checked);
    void onDistTypeDmatToggled(bool checked);
    void onBwTypeAdaptiveToggled(bool checked);
    void onBwTypeFixedToggled(bool checked);

public slots:
    void layerChanged(const int index);
    void onDepVarChanged(const int index);

public:
    void setTaskThread(GwmGWDATaskThread *taskThread);
    void updateFieldsAndEnable();
    void updateFields();
    void enableAccept();

    // 获取参数的方法
    double bandwidthSize();
    bool bandwidthType();  // true = Adaptive, false = Fixed
    GwmBandwidthWeight::KernelFunctionType bandwidthKernelFunction();
    QVariant distanceSourceParameters();
    QVariant parallelParameters();
    bool isWqda();  // 从 UI 获取（如果有复选框）
    bool hasCov();   // 从 UI 获取（如果有复选框）
    bool hasMean();  // 从 UI 获取（如果有复选框）
    bool hasPrior(); // 从 UI 获取（如果有复选框）

private:
    GwmGWDATaskThread* mTaskThread = nullptr;
};

#endif // GWMGWDAOPTIONSDIALOG_H

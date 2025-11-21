#ifndef GWMPROPERTYGWDATAB_H
#define GWMPROPERTYGWDATAB_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QTableWidget>

#include "Model/gwmlayergwdaitem.h"

namespace Ui {
class GwmPropertyGWDATab;
}

// class GwmLayerGWDAItem;  // 前向声明

class GwmPropertyGWDATab : public QWidget  // 改为 QWidget
{
    Q_OBJECT

public:
    explicit GwmPropertyGWDATab(QWidget *parent = nullptr, GwmLayerGWDAItem* item = nullptr);  // 添加 item 参数
    ~GwmPropertyGWDATab();

public:
    void updateUI();  // 添加 updateUI 方法

private:
    Ui::GwmPropertyGWDATab *ui;
    GwmLayerGWDAItem* mLayerItem;  // 添加成员变量
};

#endif // GWMPROPERTYGWDATAB_H

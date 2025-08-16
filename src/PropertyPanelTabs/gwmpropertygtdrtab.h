#ifndef GWMPROPERTYGTDRTAB_H
#define GWMPROPERTYGTDRTAB_H

#include <QWidget>

#include "Model/gwmlayergtdritem.h"
#include <QTableWidgetItem>
#include <QTableWidget>


namespace Ui {
class GwmPropertyGTDRTab;
}

class GwmPropertyGTDRTab : public QWidget
{
    Q_OBJECT

public:
    static QMap<GwmBandwidthWeight::KernelFunctionType, QString> kernelFunctionNameDict;
    static QMap<bool, QString> bandwidthTypeNameDict;

public:
    explicit GwmPropertyGTDRTab(QWidget *parent = nullptr, GwmLayerGTDRItem *item = nullptr);
    ~GwmPropertyGTDRTab();

public:
    void updateUI();

private:
    Ui::GwmPropertyGTDRTab *ui;
    GwmLayerGTDRItem* mLayerItem;

    QList<QTableWidget *> mTableWidgetList;



};

#endif // GWMPROPERTYGTDRTAB_H

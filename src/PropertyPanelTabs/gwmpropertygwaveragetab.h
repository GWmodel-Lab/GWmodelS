#ifndef GWMPROPERTYGWAVERAGETAB_H
#define GWMPROPERTYGWAVERAGETAB_H

#include <QWidget>

#include "Model/gwmlayergwaverageitem.h"
#include <QTableWidgetItem>
#include <QTableWidget>


namespace Ui {
class GwmPropertyGWAverageTab;
}

class GwmPropertyGWAverageTab : public QWidget
{
    Q_OBJECT

public:
    static QMap<GwmBandwidthWeight::KernelFunctionType, QString> kernelFunctionNameDict;
    static QMap<bool, QString> bandwidthTypeNameDict;

public:
    explicit GwmPropertyGWAverageTab(QWidget *parent = nullptr, GwmLayerGWAverageItem *item = nullptr);
    ~GwmPropertyGWAverageTab();

public:
    void updateUI();

private:
    Ui::GwmPropertyGWAverageTab *ui;
    GwmLayerGWAverageItem* mLayerItem;

    QList<QTableWidget *> mTableWidgetList;



};

#endif // GWMPROPERTYGWAVERAGETAB_H

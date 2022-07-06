#ifndef GWMFEATURESTATISTICSTAB_H
#define GWMFEATURESTATISTICSTAB_H

#include <QWidget>
#include <QStandardItemModel>
#include "Model/gwmlayeroriginitem.h"

namespace Ui {
class GwmPropertyStatisticsTab;
}

class GwmPropertyStatisticsTab : public QWidget
{
    Q_OBJECT

public:
    explicit GwmPropertyStatisticsTab(QWidget *parent = nullptr, GwmLayerOriginItem* layerItem = new GwmLayerOriginItem);
    ~GwmPropertyStatisticsTab();
    GwmLayerOriginItem* mLayerItem;
    void createModel();


private:
    Ui::GwmPropertyStatisticsTab *ui;
//    GwmLayerOriginItem* mLayerItem;
    QStandardItemModel* mModel;
};

#endif // GWMFEATURESTATISTICSTAB_H

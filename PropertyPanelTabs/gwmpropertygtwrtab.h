#ifndef GWMPROPERTYGTWRTAB_H
#define GWMPROPERTYGTWRTAB_H

#include <QWidget>
#include <QTableWidgetItem>

#include "gwmplot.h"
#include "Model/gwmlayergtwritem.h"

namespace Ui {
class GwmPropertyGTWRTab;
}

class GwmPropertyGTWRTab : public QWidget
{
    Q_OBJECT

public:
    explicit GwmPropertyGTWRTab(QWidget *parent = nullptr, GwmLayerGTWRItem* item = nullptr);
    ~GwmPropertyGTWRTab();

private:
    Ui::GwmPropertyGTWRTab *ui;
    GwmLayerGTWRItem* mLayerItem;

    GwmPlot* mBandwidthSelPlot;

public:
    void updateUI();
};


#endif // GWMPROPERTYGTWRTAB_H
